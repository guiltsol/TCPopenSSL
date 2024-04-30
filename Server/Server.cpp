#include "server.h"
#pragma warning(disable:4996)




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
		std::cout << "Couldn't init WSA!";
		exit(EXIT_FAILURE);
	}
	
	serversocket = socket(AF_INET, SOCK_STREAM, NULL);
	bind(serversocket, (SOCKADDR*)&addr, addrlength);
	listen(serversocket, SOMAXCONN);
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

	std::cout << "\nNew Message:\n"
		<< "Index: " << Index << "\n"
		<< "Message Size:" << msgSize << "\n"
		<< "Text:\n[ " << msg << " ]\n";

	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (i == Index) continue;
		sendMessageByIndex(i, msg, msgSize, P_ChatMessage);
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

