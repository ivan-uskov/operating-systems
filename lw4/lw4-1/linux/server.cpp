#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#define ERROR -1

using namespace std;

void ProcessClient(int client)
{
    cout << "New Client!" << endl;

    int rc;
    char buf[100];

    while ( (rc = read(client, buf, sizeof(buf))) > 0)
    {
        cout << "read " << rc << " bytes: " << string(buf, rc) << endl;
    }

    if (rc == ERROR)
    {
        cerr << "Read error occured!" << endl;
        exit(-1);
    }
    else if (rc == 0)
    {
        cout << "Client disconnected!" << endl;
    }

    close(client);
}

void Bind(int handle, string const& socketPath)
{
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);
    unlink(socketPath.c_str());

    if (bind(handle, (struct sockaddr*)&addr, sizeof(addr)) == ERROR)
    {
        perror("bind error");
        exit(-1);
    }
}

void Listen(int handle, int const MAX_CLIENTS)
{
    if (listen(handle, MAX_CLIENTS) == ERROR)
    {
        perror("listen error");
        exit(-1);
    }
}

int NewSocket(int domain)
{
    int handle = socket(domain, SOCK_STREAM, 0);
    if (handle == ERROR)
    {
        perror("socket error");
        exit(-1);
    }

    return handle;
}

int main(int argc, char *argv[])
{
    string socketPath("./chat-socket.sock");
    int const MAX_CLIENTS = 5;

    int handle = NewSocket(AF_UNIX);

    Bind(handle, socketPath);
    Listen(handle, MAX_CLIENTS);

    while (1)
    {
        int client = accept(handle, NULL, NULL);

        if (client == ERROR)
        {
            perror("accept error");
            continue;
        }

        ProcessClient(client);
    }

    return 0;
}
