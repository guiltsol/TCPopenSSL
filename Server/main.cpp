//#define WIN32_LEAN_AND_MEAN
#include "server.h"
#include <windows.h>
#include <locale>
#include <ws2tcpip.h>
void showAvailableIPs() {
    char host[256];
    char ipstr[INET6_ADDRSTRLEN];
    struct addrinfo hints, * res, * p;

    gethostname(host, sizeof(host));
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    std::cout << "Получение доступных IP-адресов для " << host << "...\n";

    if (getaddrinfo(host, NULL, &hints, &res) != 0) {
        std::cerr << "Ошибка при вызове getaddrinfo(): " << WSAGetLastError() << std::endl;
        // Вместо выхода из программы, просто возвращаемся из функции.
        return;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        void* addr;
        std::string ipver;

        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else { // IPv6
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        std::cout << ipver << ": " << ipstr << std::endl;
    }

    freeaddrinfo(res); // Освободите связанный список
}
int main()
{
    setlocale(LC_ALL, "Rus");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    WSADATA wsData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsData);
    if (result != 0) {
        std::cerr << "Ошибка при инициализации Winsock: " << result << std::endl;
        return 1;
    }
    showAvailableIPs();
    //TCPserver::Server tempServer(8288, "127.0.0.1");
    //tempServer.init(); // Этот вызов отобразит доступные IP-адреса
    std::string ipAddress;
   // int port;
    std::cout << "Введите IP-адрес сервера: ";
    std::cin >> ipAddress;
    //std::cout << "Введите порт сервера: ";
    //std::cin >> port;
    TCPserver::Server server(8288, ipAddress);
    server.start();
    WSACleanup();
    return 0;
}