#include <iostream>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
#include "WinSock2.h"
#include "Client.h"
#pragma warning(disable:4996)

std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}

void TCPserver::Client::setUsername(std::string username)
{
    this->username = username;
}

TCPserver::Client::Client(int port)
{
    this->port = port;
    std::cout << "Enter server IP address: ";
    std::cin >> this->ipaddress;
    std::cin.ignore(); // Игнорируйте оставшийся символ новой строки

    std::cout << "Enter your username: ";
    getline(std::cin, this->username); // Запросите имя пользователя

    addrlength = sizeof(addr);
}

TCPserver::Client::~Client()
{
}

void TCPserver::Client::start()
{
    init();
    connectToServer();
    while (true)
    {
        sendPacket();
    }
}

void TCPserver::Client::init()
{
    if (WSAStartup(MAKEWORD(2, 1), &wsa) == SOCKET_ERROR)
    {
        std::cout << "Couldn't init WSA!";
        exit(EXIT_FAILURE);
    }
    addr.sin_addr.s_addr = inet_addr(ipaddress.c_str());
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
}

void TCPserver::Client::connectToServer()
{
    Connection = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(Connection, (SOCKADDR*)&addr, addrlength) != 0)
    {
        std::cout << "Error: failed connect to server! \n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Connected to Server: Success. \n";
    HANDLE hThread = CreateThread(NULL, 0, ClientHandler, this, 0, NULL);

}

bool TCPserver::Client::ProcessPacket(Packet packetType)
{
    switch (packetType)
    {
    case P_ChatMessage:
        if (!processChatMessagePacket()) return false;
        break;
    default:
        return false;
        break;
    }
    return true;
}

bool TCPserver::Client::processChatMessagePacket()
{
    int msgSize;
    recv(Connection, (char*)&msgSize, sizeof(int), NULL);
    char* msg = new char[msgSize + 1];
    msg[msgSize] = '\0';
    recv(Connection, msg, msgSize, NULL);
    std::string timeStr = currentDateTime();
   // std::cout << "New message: ";
    std::cout << timeStr << " - Message sent: " << msg << std::endl;

    delete[] msg;
    return true;
}


DWORD __stdcall TCPserver::Client::ClientHandler(LPVOID lpParam)
{
    Client* client = static_cast<Client*>(lpParam);
    while (true)
    {
        client->recieveMessage();
    }
    return 0;
}

void TCPserver::Client::recieveMessage()
{
    Packet packetType;
    recv(Connection, (char*)&packetType, sizeof(Packet), NULL);
    if (!ProcessPacket(packetType)) std::cout << "Failed to receive message" << std::endl;
}

bool TCPserver::Client::sendChatPacket()
{
    Packet packetType = P_ChatMessage;

    //std::cout << "Enter a chat message: ";
    std::string message;
    std::getline(std::cin, message);

    std::string fullMessage = username + ": " + message; // Используйте сохраненное имя пользователя
    
    int messageSize = fullMessage.size();
    std::string timeStr = currentDateTime();

    send(Connection, (char*)&packetType, sizeof(Packet), NULL);
    send(Connection, (char*)&messageSize, sizeof(int), NULL);
    send(Connection, fullMessage.c_str(), messageSize, NULL);
    std::cout << timeStr << " - Message sent: " << fullMessage << std::endl;
    return true;
}

void TCPserver::Client::sendPacket()
{
    /*
    std::cout << "\nSelect packet type: " << std::endl;
    std::cout << "1: " << "Chat Message" << std::endl;
    
    int packetTypeIndex; 
    std::cin >> packetTypeIndex;
    std::cin.ignore();
    packetTypeIndex--;

    switch (packetTypeIndex)
    {
    case P_ChatMessage:
        sendChatPacket();
        break;
    default:
        std::cout << "Unknown packet type" << std::endl;
        break;
    }
    */
    sendChatPacket();
}
