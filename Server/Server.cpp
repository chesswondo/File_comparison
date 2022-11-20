#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <winsock2.h>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;


#pragma comment(lib,"ws2_32.lib")

void whois(SOCKET cs);
void checkfile(SOCKET cs, const char * buf_serv);


int Send(SOCKET s, const char* t, int l)
{
    ofstream out("server.log", ios::app);
    char buf[512];
    time_t T = time(0);
    tm* Tm = localtime(&T);
    strftime(buf, 512, "%F %T", Tm);
    out << buf << "  >>  " << t << endl;
    return send(s, t, l, 0);
}

int Recv(SOCKET s, char* t, int l)
{
    int rc = recv(s, t, l, 0);
    ofstream out("server.log", ios::app);
    char buf[512];
    time_t T = time(0);
    tm* Tm = localtime(&T);
    strftime(buf, 512, "%F %T", Tm);
    out << buf << "  <<  " << t << endl;
    return rc;
}


bool quasi(const string& a, const string& b)
{
    istringstream f(a), s(b);
    for (string w1, w2;;)
    {
        f >> w1;
        s >> w2;
        if (!f || !s)
        {
            if (f || s) return false;
            return true;
        }
        if (w1.size() != w2.size()) return false;
        for (int i = 0; i < w1.size(); ++i)
            if (toupper(w1[i]) != toupper(w2[i])) return false;
    }
    return true;
}

int main()
{
    unsigned short int port = 1044;

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
            
            //отправка привета, проверка, что все работает
            if (Send(cs, "SRVFILE", 8) != 8) throw runtime_error("send SRVFILE");

            for (;;)
            {
                char buf_serv[256];
                int rc = Recv(cs, buf_serv, 256);
                if (rc < 0) throw runtime_error("recv file name");
                if (rc == 0) break;
                if (buf_serv == "WHO"s) whois(cs);
                else checkfile(cs, buf_serv);
            }
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

void whois(SOCKET cs)
{
    char answer[] = "(c) Anton Krasikov. V.19 - file comparing.\n";
    Send(cs, answer, strlen(answer) + 1);
}

void checkfile(SOCKET cs, const char* buf_serv)
{
    try {

        //подсчет строк в файле, отправка клиенту
        int rows_server = 0;
        ifstream file(buf_serv);
        if (!file.is_open())
        {
            if (Send(cs, "File not found", 15) != 15) throw runtime_error("send file not found");
        }
        else
        {
            string str;
            while (getline(file, str))
            {
                rows_server++;
            }
            if (Send(cs, "OK", 3) != 3) throw runtime_error("send OK");
            file.close();
        }

        string rows_server_ch = to_string(rows_server);
        if (Send(cs, rows_server_ch.c_str(), rows_server_ch.size() + 1) != rows_server_ch.size() + 1) throw runtime_error("send row counts");

        //самая важная часть - получение строк от клиента и отправка ему результата
        ifstream file_server(buf_serv);
        if (!file_server.is_open())
        {
            throw runtime_error("Server file not found");
        }

        string str_server;
        while (getline(file_server, str_server))
        {
            char curr_str[256];
            if (Recv(cs, curr_str, 256) < 0) throw runtime_error("recv row from client");
            string str_client = curr_str;

            if (quasi(str_client, str_server))
            {
                if (Send(cs, "Quasiequal", 11) != 11) throw runtime_error("send quasiequal");
            }

            else
            {
                if (Send(cs, "Not quasiequal", 15) != 15) throw runtime_error("send not quasiequal");
            }

        }
        file.close();
    }
    catch (exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
}
