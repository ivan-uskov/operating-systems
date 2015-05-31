#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#define ERROR -1

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

void Connect(int handle, string const& socketPath)
{
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

void ProcessMessages(int handle)
{
    char buf[100];
    int rc;

    cout << "Write message:" << endl;

    while( (rc = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
    {
        if (write(handle, buf, rc) != rc)
        {
            if (rc > 0)
            {
                cerr << "partial write" << endl;
            }
            else
            {
                cerr << "write error" << endl;
                exit(-1);
            }
        }
    }
}


int main(int argc, char *argv[])
{
    string socketPath("./chat-socket.sock");

    int handle = NewSocket(AF_UNIX);

    Connect(handle, socketPath);
    ProcessMessages(handle);

    return 0;
}