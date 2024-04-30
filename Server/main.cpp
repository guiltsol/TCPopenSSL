
#include "server.h"

int main()
{

    TCPserver::Server server(8288, "192.168.0.103");
    server.start();

    return 0;
}

