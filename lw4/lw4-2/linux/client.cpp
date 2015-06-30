#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>

#define ERROR -1
#define BUF_SIZE 1024
#define SOCKET_FILE "./chat-socket.sock"

using namespace std;

int NewSocket(int domain)
{
    int handle = socket(domain, SOCK_STREAM, 0);
    if (handle == ERROR)
    {
        cerr << "socket error" << endl;
        exit(-1);
    }

    return handle;
}

void Connect(int handle)
{
    string socketPath(SOCKET_FILE);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(handle, (struct sockaddr*)&addr, sizeof(addr)) == ERROR)
    {
        cerr << "connect error" << endl;
        exit(-1);
    }
}

void WaitForActivity(int max_sd, fd_set * readfds)
{
    int activity = select( max_sd + 1 , readfds , NULL , NULL , NULL);
    if ((activity < 0) && (errno != EINTR))
    {
        printf("select error");
    }
}

void TryReceiveMessage(int sock, fd_set * readfds, char buffer[])
{
    if (FD_ISSET(sock, readfds))
    {
        int valread = read(sock, buffer, BUF_SIZE);
        if (valread)
        {
            cout << string(buffer, valread);
        }
    }
}

void TrySendMessage(int sock, fd_set * readfds, char buffer[], string const& name)
{
    if (FD_ISSET(STDIN_FILENO, readfds))
    {
        int valread = read(STDIN_FILENO, buffer, BUF_SIZE);
        if (valread)
        {
            auto msg = string(buffer, valread);
            msg = "[" + name + "]: " + msg;
            send(sock, msg.c_str(), msg.size() * sizeof(char), 0);
        }
    }
}

int InitReadFDS(int sock, fd_set * readfds)
{
    FD_ZERO(readfds);
    FD_SET(sock, readfds);
    FD_SET(STDIN_FILENO, readfds);
    return sock > STDIN_FILENO ? sock : STDIN_FILENO;
}

void ProcessMessages(int handle, string const& name)
{
    static char buffer[BUF_SIZE];
    int rc;

    while (1)
    {
        fd_set readfds;
        int max_sd = InitReadFDS(handle, &readfds);

        WaitForActivity(max_sd, &readfds);

        TryReceiveMessage(handle, &readfds, buffer);
        TrySendMessage(handle, &readfds, buffer, name);
    }
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

int main(int argc, char* argv[])
{
    auto name = InitNameFromArgv(argc, argv);
    int handle = NewSocket(AF_UNIX);

    Connect(handle);
    ProcessMessages(handle, name);

    return 0;
}