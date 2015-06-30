#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string>
#include <iostream>

using namespace std;

#define MAX_CLIENTS 30
#define SOCKET_FILE "./chat-socket.sock"

void InitClientSD(int clients[])
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i] = 0;
    }
}

int NewSocket(int type)
{
    int sock = socket(type, SOCK_STREAM, 0);
    if (sock == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    return sock;
}

void AllowMultipleConnections(int sock)
{
    int opt = 1;
    if( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
}

void Bind(int sock, struct sockaddr_un * addr)
{
    string socketPath(SOCKET_FILE);

    memset(addr, 0, sizeof(*addr));
    addr->sun_family = AF_UNIX;
    strncpy(addr->sun_path, socketPath.c_str(), sizeof(addr->sun_path) - 1);

    if (bind(sock, (struct sockaddr *)addr, sizeof(*addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

int InitReadFDS(fd_set * readfds, int master_sock, int clients[])
{
    FD_ZERO(readfds);

    FD_SET(master_sock, readfds);
    int max_sd = master_sock;

    for (int i = 0 ; i < MAX_CLIENTS ; ++i)
    {
        int sd = clients[i];
        if(sd > 0)
        {
            FD_SET(sd , readfds);
        }

        if(sd > max_sd)
        {
            max_sd = sd;
        }
    }

    return max_sd;
}

void Listen(int sock)
{
    if (listen(sock, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

void AppendNewClient(int clients[], int newSock)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if ( clients[i] == 0 )
        {
            clients[i] = newSock;
            printf("Adding to list of sockets as %d\n" , i);

            break;
        }

        if (i == MAX_CLIENTS - 1)
        {
            printf("Error: limit of clients");
            close(newSock);
        }
    }
}

void AppendNewClients(int clients[], int master_sock, fd_set * readfds, struct sockaddr_un * address)
{
    int addrlen = sizeof(*address);
    if (FD_ISSET(master_sock, readfds))
    {
        int new_socket = accept(master_sock, (struct sockaddr *)address, (socklen_t*)(&addrlen));
        if (new_socket < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        AppendNewClient(clients, new_socket);
    }
}

void SendForClients(int sender, char buffer[], int clients[])
{
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (sender != i)
        {
            send(clients[i], buffer, strlen(buffer), 0);
        }
    }

}

void ProcessClientsMessages(int clients[], fd_set * readfds)
{
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        int sd = clients[i];
        char buffer[1025];

        if (FD_ISSET( sd , readfds))
        {
            int valread = read( sd , buffer, 1024);
            if (valread)
            {
                buffer[valread] = '\0';
                SendForClients(i, buffer, clients);
            }
            else
            {
                close( sd );
                clients[i] = 0;
            }
        }
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

int main(int argc , char *argv[])
{
    int clients[30];
    InitClientSD(clients);

    int master_socket = NewSocket(AF_UNIX);
    AllowMultipleConnections(master_socket);

    struct sockaddr_un address;
    Bind(master_socket, &address);

    Listen(master_socket);
    std::cout << "Waiting for connections on chat-socket.sock" << endl;

    while(1)
    {
        fd_set readfds;
        int max_sd = InitReadFDS(&readfds, master_socket, clients);

        WaitForActivity(max_sd, &readfds);

        AppendNewClients(clients, master_socket, &readfds, &address);
        ProcessClientsMessages(clients, &readfds);
    }

    return 0;
}