#include "stdafx.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using namespace std;

class Application
{
public:
    Application()
        : masterSocket(INVALID_SOCKET)
        , address(NULL)
        , buffLen(DEFAULT_BUFLEN)
    {}

    ~Application()
    {
        WSACleanup();

        if (address)
        {
            freeaddrinfo(address);
        }

        for_each(clients.begin(), clients.end(), [](SOCKET & client){
            shutdown(client, SD_SEND);
            closesocket(client);
        });
    }

    void ListenAndServe()
    {
        InitWinSock();
        InitAddressInfo();
        ResolveServerHost();
        InitMasterSock();
        Bind();
        Listen();

        Serve();
    }

private:
    void InitWinSock()
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData))
        {
            Err("Err: init winsock!");
        }
    }

    void InitAddressInfo()
    {
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;
    }

    void ResolveServerHost()
    {
        if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &address))
        {
            Err("Err: resolve host!");
        }
    }

    void InitMasterSock()
    {
        masterSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (masterSocket == INVALID_SOCKET)
        {
            Err("Err: init master sock!");
        }

        u_long iMode = 1;
        ioctlsocket(masterSocket, FIONBIO, &iMode);
    }

    void Bind()
    {
        auto res = bind(masterSocket, address->ai_addr, (int)address->ai_addrlen);
        if (res == SOCKET_ERROR)
        {
            Err("Err: bind fail!");
        }
    }

    void Listen()
    {
        auto res = listen(masterSocket, SOMAXCONN);
        if (res == SOCKET_ERROR)
        {
            Err("Err: listen fail!");
        }
    }

    void Serve()
    {
        while (1)
        {
            Accept();
            ProcessClientsMessage();
        }
    }

    void ProcessClientsMessage()
    {
        for_each(clients.begin(), clients.end(), [this](SOCKET client){
            if (this->Recive(client))
            {
                this->SendForClientsFrom(client);
            }
        });
    }

    void Err(string const& msg)
    {
        throw runtime_error(msg + " Code: " + to_string(WSAGetLastError()));
    }

    void Accept()
    {
        auto client = accept(masterSocket, (struct sockaddr *)NULL, NULL);
        if (client != INVALID_SOCKET)
        {
            clients.push_back(client);
        }
    }

    void SendForClientsFrom(SOCKET sender)
    {
        for_each(clients.begin(), clients.end(), [sender, this](SOCKET & client){
            if (client != sender)
            {
                Send(client);
            }
        });
    }

    void Send(SOCKET client)
    {
        auto bytesSend = send(client, buffer, buffLen, 0);
        if (bytesSend != buffLen)
        {
            cerr << "Client disconnected!" << endl;
            DeleteClient(client);
        }
    }

    bool Recive(SOCKET client)
    {
        buffLen = recv(client, buffer, DEFAULT_BUFLEN, 0);
        return buffLen > 0;
    }

    void DeleteClient(SOCKET client)
    {
        auto it = find_if(clients.begin(), clients.end(), [client](SOCKET const& sock){
            return sock == client;
        });

        if (it != clients.end())
        {
            clients.erase(it);
        }
    }

private:
    WSADATA wsaData;
    SOCKET masterSocket;
    vector<SOCKET> clients;
    struct addrinfo hints;
    struct addrinfo * address;

    char buffer[DEFAULT_BUFLEN];
    int buffLen;
};

int main(int argc, char* argv[])
{
    try
    {
        Application app;
        app.ListenAndServe();
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
    }

    return 0;
}

