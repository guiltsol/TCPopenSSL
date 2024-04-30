#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <unordered_map>
#include <string>
#include <cassert>

#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CONNECTIONS 5

namespace TCPserver
{
	enum Packet
	{
		P_ChatMessage
	};
	class Server
	{
	private:
		WSADATA wsa;
		SOCKET serversocket;
		SOCKET connections[MAX_CONNECTIONS];
		int counter = 0;
		std::string ipaddress;
		int port;
		std::string message;
		SOCKADDR_IN addr;
		int addrlength;
		int reclength;

		struct ClientData {
			Server* server;
			int connectionIndex;
		};
		ClientData clientData;


	private:
		void init();
		static DWORD WINAPI ClientHandler(LPVOID lpParam);
		void getCconnect();
		bool processPacket(int Index, Packet packetType);
		bool processChatMessagePacket(int Index);
		void sendMessageByIndex(int Index, char* msg, int msgSize, Packet packetType);
	public:
		Server(int, std::string);
		~Server();
	public:
		void start();
		//void stop();
	};


}

