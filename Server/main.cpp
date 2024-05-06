
#include "server.h"

int main()
{
    setlocale(LC_ALL, "Rus");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    TCPserver::Server server(8288, "127.0.0.1");
    server.start();

    return 0;
}

