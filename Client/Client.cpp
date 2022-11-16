#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>

using namespace std;

#pragma comment(lib,"ws2_32.lib")


int main()
{
    string host = "127.0.0.1";
    unsigned short int port = 6983;
    SOCKET s = INVALID_SOCKET;
    try {
        WORD wVersionRequested = MAKEWORD(1, 1);
        WSADATA wsaData;
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
            throw runtime_error("WSAStartup");

        // Создание сокета
        s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) throw runtime_error("socket creation");

        // Вызов
        sockaddr_in name;
        memset(&name, 0, sizeof(name));
        name.sin_addr.s_addr = inet_addr(host.c_str());
        name.sin_family = AF_INET;
        name.sin_port = htons(port);
        if (SOCKET_ERROR == connect(s, (SOCKADDR*)&name, sizeof(name)))
            throw runtime_error("connect");

        // Получение ответа
        char buf[256];
        if (recv(s,buf,256,0) < 0) throw runtime_error("recv hello");

        cout << buf;
        if (buf != "SRVFILE"s) throw runtime_error("wrong server");



    }
    catch (exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    closesocket(s);
    WSACleanup();
}

