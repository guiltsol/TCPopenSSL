#include "Client.h"

int main()
{
	int port = 8288;
	TCPserver::Client client(port);
	client.start();
	return 0;
}