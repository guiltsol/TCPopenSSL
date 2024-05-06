#include "Client.h"

int main()
{
	setlocale(LC_ALL, "Rus");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	int port = 8288;
	TCPserver::Client client(port);
	client.start();
	return 0;
}