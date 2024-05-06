#include <iostream>
#include <ctime>
#include <ws2tcpip.h>
#include "server.h"
#pragma warning(disable:4996)


std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%X", &tstruct);

	return buf;
}

TCPserver::Server::Server(int port, std::string ipaddress)
{
	this->port = port;
	this->ipaddress = ipaddress;
	addrlength = sizeof(addr);
	for (int i = 0; i < MAX_CONNECTIONS; i++) connections[i] = NULL;
}

TCPserver::Server::~Server()
{
}

void TCPserver::Server::start()
{

	init();
	while(true)
	{
		getCconnect();
	}
}

void TCPserver::Server::init()
{
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ipaddress.c_str());

    if (WSAStartup(MAKEWORD(2, 1), &wsa) == SOCKET_ERROR)
    {
        std::cout << "Не удалось инициализировать WSA!";
        exit(EXIT_FAILURE);
    }

    serversocket = socket(AF_INET, SOCK_STREAM, NULL);
    bind(serversocket, (SOCKADDR*)&addr, addrlength);
    listen(serversocket, SOMAXCONN);

    // Отобразите все доступные IP-адреса
    char host[256];
    gethostname(host, sizeof(host));

    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // AF_INET или AF_INET6, чтобы зафиксировать версию
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, NULL, &hints, &res) != 0) {
        perror("getaddrinfo() не удалось");
        exit(EXIT_FAILURE);
    }

    std::cout << "Доступные IP-адреса для " << host << ":\n";
    char ipstr[INET6_ADDRSTRLEN];
    for(p = res; p != NULL; p = p->ai_next) {
        void *addr;
        std::string ipver;

        // получить указатель на сам адрес,
        // разные поля в IPv4 и IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // преобразовать IP в строку и напечатать его:
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        std::cout << ipver << ": " << ipstr << std::endl;
    }

    freeaddrinfo(res); // Освободите связанный список
}

void TCPserver::Server::getCconnect()
{
	while (counter >= MAX_CONNECTIONS) Sleep(5000);
	SOCKET newConnection;
	newConnection = accept(serversocket, (SOCKADDR*)&addr, &addrlength);
	if (newConnection == 0)
	{
		std::cout << "Error to connect";
	}
	else
	{
		for (int i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (connections[i] == NULL or connections[i] == INVALID_SOCKET)
			{
				connections[i] = newConnection;
				std::cout << "Client connected! \n";
				clientData = { this, i };
				HANDLE hThread = CreateThread(NULL, 0, ClientHandler, &clientData, 0, NULL);
				
				break;
			}
		}
		
		counter++;
	}
}

bool TCPserver::Server::processPacket(int Index, Packet packetType)
{
	switch (packetType)
	{
	case P_ChatMessage:
		if (!processChatMessagePacket(Index)) return false;
		break;
	default:
		return false;
		break;
	}

	return true;
}

bool TCPserver::Server::processChatMessagePacket(int Index)
{
	int msgSize;
	int bytesReceived = recv(connections[Index], (char*)&msgSize, sizeof(int), NULL);
	char* msg = new char[msgSize + 1];
	msg[msgSize] = '\0';
	recv(connections[Index], msg, msgSize, NULL);

	// Extract client name and message from the received data
	std::string fullMessage(msg);
	std::string clientName = fullMessage.substr(0, fullMessage.find(": "));

	// Calculate the size of the actual message text without the client's name
	std::string messageText = fullMessage.substr(fullMessage.find(": ") + 2);
	int messageTextSize = messageText.size();

	std::string timeStr = currentDateTime(); // Get the current time as a string
	std::cout << timeStr << " - New Message:\n" // Display the time alongside the message
		<< "Index: " << Index << " - " << clientName << "\n" // Display the client name after index
		<< "Message Size:" << messageTextSize << "\n" // Display the size of the message text without the client's name
		<< "Text:\n[ " << messageText << " ]\n"; // Display the message text without the client name

	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (i == Index) continue;
		// Send the full message including the client's name to other clients
		sendMessageByIndex(i, const_cast<char*>(fullMessage.c_str()), fullMessage.size(), P_ChatMessage);
	}
	delete[] msg;
	return true;
}

void TCPserver::Server::sendMessageByIndex(int Index, char* msg, int msgSize, Packet packetType)
{
	if (connections[Index] == NULL or connections[Index] == INVALID_SOCKET) return;
	send(connections[Index], (char*)&packetType, sizeof(Packet), NULL);
	send(connections[Index], (char*)&msgSize, sizeof(int), NULL);
	send(connections[Index], msg, msgSize, NULL);

}

DWORD __stdcall TCPserver::Server::ClientHandler(LPVOID lpParam)
{
	{
		ClientData* clientData = static_cast<ClientData*>(lpParam);
		Server* server = clientData->server;
		int connectionIndex = clientData->connectionIndex;
		std::cout << "Handling client with index: " << connectionIndex << std::endl;

		Packet packetType;
		while (true)
		{
			int bytesReceived = recv(server->connections[connectionIndex], (char*)&packetType, sizeof(Packet), NULL);
			if (bytesReceived <= 0)
			{
				closesocket(server->connections[connectionIndex]);
				server->connections[connectionIndex] = INVALID_SOCKET;
				server->counter--;
				std::cout << "\nClient with index " << connectionIndex << " disconected. \n";
				return 1;
			}
			if (!server->processPacket(connectionIndex, packetType)) return 1;
		}
		
		return 0;

	}

}

