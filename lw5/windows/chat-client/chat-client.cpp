#include "stdafx.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using namespace std;

struct MessageQueue
{
    queue<string> values;
    mutex mtx;
};

class ServerHandler
{
public:
    ServerHandler(MessageQueue & messages)
        : serverSocket(INVALID_SOCKET)
        , buffLen(DEFAULT_BUFLEN)
        , address(NULL)
        , messages(messages)
    {}

    ~ServerHandler()
    {
        WSACleanup();

        if (address)
        {
            freeaddrinfo(address);
        }
    }

    void ListenAndServe()
    {
        InitWinSock();
        InitAddressInfo();
        ResolveServerHost();
        InitServerSock();
        Connect();

        Serve();
    }

private:
    void Serve()
    {
        while (1)
        {
            Recive();
            Send();
        }
    }

    void Recive()
    {
        buffLen = recv(serverSocket, buffer, DEFAULT_BUFLEN, 0);
        if (buffLen > 0)
        {
            cout << string(buffer, buffLen) << endl;
        }
    }

    void Send()
    {
        lock_guard<mutex> lockGuard(messages.mtx);
        if (!messages.values.empty())
        {
            auto msg = messages.values.front();

            auto msgSize = msg.size() * sizeof(char);

            auto bytesSend = send(serverSocket, msg.data(), msgSize, 0);
            if (bytesSend != msgSize)
            {
                throw runtime_error("Server Disconnected!");
            }

            messages.values.pop();
        }
    }

    void InitWinSock()
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData))
        {
            throw runtime_error("Err: init winsock!");
        }
    }

    void InitAddressInfo()
    {
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }

    void ResolveServerHost()
    {
        if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &address))
        {
            throw runtime_error("Err: resolve host!");
        }
    }

    void InitServerSock()
    {
        serverSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (serverSocket == INVALID_SOCKET)
        {
            throw runtime_error("Err: init server sock!");
        }
    }

    void Connect()
    {
        auto res = connect(serverSocket, address->ai_addr, (int)address->ai_addrlen);
        if (res == SOCKET_ERROR)
        {
            throw runtime_error("Err: connect server sock!");
        }

        u_long iMode = 1;
        ioctlsocket(serverSocket, FIONBIO, &iMode);
    }

private:
    WSADATA wsaData;
    SOCKET serverSocket;

    struct addrinfo hints;
    struct addrinfo * address;

    char buffer[DEFAULT_BUFLEN];
    int buffLen;

    MessageQueue & messages;
};

string ReadMessage(istream & in)
{
    string str;
    getline(in, str);

    if (!in)
    {
        throw domain_error("Error on reading message");
    }

    return str;
}

void ProcessConsoleMessages(string const& name, MessageQueue & messages)
{
    while (1)
    {
        auto msg = ReadMessage(cin);

        lock_guard<mutex> guard(messages.mtx);
        messages.values.push("[" + name + "]: " + msg);
    }
}

void Serve(MessageQueue * messages)
{
    ServerHandler handler(*messages);
    handler.ListenAndServe();
}

string InitNameFromArgv(int argc, char* argv[])
{
    string name = "Anonymus";
    if (argc == 2)
    {
        name = argv[1];
    }

    return name;
}

int main(int argc, char *argv[])
{
    try
    {
        auto name = InitNameFromArgv(argc, argv);

        MessageQueue messages;
        thread server(Serve, &messages);
        ProcessConsoleMessages(name, messages);
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
        return 1;
    }

    return 0;
}
