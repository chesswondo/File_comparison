
#include <iostream>
#include <string>
#include <winsock2.h>

using namespace std;

#pragma comment(lib,"ws2_32.lib")


int main()
{
    unsigned short int port = 6983;

    SOCKET ms = INVALID_SOCKET, cs = INVALID_SOCKET;
    try {
        WORD wVersionRequested = MAKEWORD(1, 1);
        WSADATA wsaData;
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
            throw runtime_error("WSAStartup");

        // Создание сокета
        ms = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ms == INVALID_SOCKET) throw runtime_error("socket creation");

        // Приклепление к порту
        sockaddr_in name;
        memset(&name, 0, sizeof(name));
        name.sin_addr.s_addr = INADDR_ANY;
        name.sin_family = AF_INET;
        name.sin_port = htons(port);
        if (SOCKET_ERROR == bind(ms, (SOCKADDR*)&name, sizeof(name)))
            throw runtime_error("bind");

        for (;;)
        {
            if (listen(ms, 1) != 0) throw runtime_error("listen");

            // Ожидание соединения
            cs = accept(ms, NULL, NULL);
            if (cs == INVALID_SOCKET) throw runtime_error("accept");

            // Начинается работа
            if (send(cs, "SRVFILE", 8, 0) != 8) throw runtime_error("send SRVFILE");

            // Текст протокола

            closesocket(cs);
        }


    }
    catch (exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    closesocket(ms);
    closesocket(cs);
    WSACleanup();
}

