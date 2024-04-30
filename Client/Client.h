#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <unordered_map>
#include <string>
#include <cassert>
#pragma comment(lib, "ws2_32.lib")



namespace TCPserver
{
	enum Packet
	{
		P_ChatMessage
	};
	class Client
	{
	private:
		WSADATA wsa;
		SOCKET Connection;
		std::string ipaddress;
		int port;
		//char buffer[SIZE];
		std::string message;
		SOCKADDR_IN addr;
		int addrlength;
		int reclength;
		std::string username;
		bool ProcessPacket(Packet packetType);
		bool processChatMessagePacket();

		bool sendChatPacket();

		static DWORD WINAPI ClientHandler(LPVOID lpParam);
	public:
		Client(int port);
		~Client();
	public:
		void start();
		//void stop(); 
		void init();
		void connectToServer();
		void recieveMessage();
		void sendPacket();
		void setUsername(std::string username);
	};
}
