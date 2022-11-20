#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <string>

using namespace std;

#pragma comment(lib,"ws2_32.lib")

bool whois(SOCKET s);
bool checkFile(SOCKET s, const string& in);

int Send(SOCKET s, const char* t, int l)
{
    ofstream out("client.log", ios::app);
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
    ofstream out("client.log", ios::app);
    char buf[512];
    time_t T = time(0);
    tm* Tm = localtime(&T);
    strftime(buf, 512, "%F %T", Tm);
    out << buf << "  <<  " << t << endl;
    return rc;
}


int main()
{
    string host = "127.0.0.1";
    unsigned short int port = 1044;
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

        // Получение привета, проверка, что все работает
        char buf[256];
        if (Recv(s,buf,256) < 0) throw runtime_error("recv hello");

        cout << buf << endl;
        if (buf != "SRVFILE"s) throw runtime_error("wrong server");

        for (;;)
        {
            cout << "Available commands: Quit, Who or filename\n > ";
            string in;
            getline(cin, in);
            if (in == "Who") { if (whois(s) == false) break; }
            else if (in == "Quit") break;
            else if (checkFile(s, in) == false) break;
        }
    }
    catch (exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    closesocket(s);
    WSACleanup();
}


bool whois(SOCKET s)
{
    try {
        char who[] = "WHO", buf[512];
        if (Send(s, who, strlen(who) + 1) != strlen(who) + 1) throw runtime_error("send who");
        int rc = Recv(s, buf, 512);
        if (rc < 0) throw runtime_error("recv who");
        if (rc == 0) return false;

        cout << "Server says: " << buf << endl;
        return true;
    } 
    catch (exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    return false;
}

bool checkFile(SOCKET s, const string& filename)
{
    try
    {
        //проверка есть ли файл у самого клиента
        int rows_client = 0;
        ifstream file(filename);
        if (!file.is_open())
        {
            cout << "Error: client file not found\n\n";
            return true;
        }

        //отправка имени файла серверу, подсчет строк
        else
        {
            string str;
            while(getline(file, str))
            {
                rows_client++;
            }
            if (Send(s, filename.c_str(), filename.size() + 1) != filename.size() + 1) throw runtime_error("send file name");
            file.close();
        }

        //обработка если файла нет на сервере
        char is_found[15];
        if (Recv(s, is_found, 15) < 0)  throw runtime_error("recv about file");
        if (is_found == "File not found"s) throw runtime_error("Server file not found");

        //получение количества строк на файле сервера, сравнение количеств
        char rows_server_ch[256];
        int rows_server = 0;
        if (Recv(s, rows_server_ch, 256) < 0) throw runtime_error("recv about rows");
        for (int i = 0; i < strlen(rows_server_ch); i++)
        {
            rows_server += (rows_server_ch[i] - '0') * pow(10, strlen(rows_server_ch) - i - 1);
        }

        if (rows_client != rows_server)
        {
            cout << "row client: " << rows_client << endl;
            cout << "row server: " << rows_server << endl;
            throw runtime_error("Row counts don't match");
        }

        cout << "OK" << endl;

        //самая важная часть - посылание строк серверу и получение результата
        ifstream file_client(filename);
        if (!file_client.is_open())
        {
            throw runtime_error("Client file not found");
        }

        string str_client;
        int i = 1;
        while (getline(file_client, str_client))
        {
            if (Send(s, str_client.c_str(), str_client.size() + 1) != str_client.size() + 1) throw runtime_error("send row");
            char curr_str[256];
            if (Recv(s, curr_str, 256) < 0) throw runtime_error("recv is rows quasiequal");

            cout << "Rows " << i++ << " are ";
            if (strlen(curr_str) == 10) cout << "quasiequal" << endl;
            else cout << "not quasiequal" << endl;
        }
        file.close();
        return true;
    }
    catch (exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    return false;
}
