#include "stdafx.h"

#define BUFSIZE 512
using namespace std;

void SetPipeMode(HANDLE pipe)
{
    DWORD dwMode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(pipe, &dwMode, NULL, NULL))
    {
        throw domain_error("ERROR: Can't set pipe mode");
    }
}

HANDLE OpenNamedPipe(LPTSTR name)
{
    HANDLE pipe;

    while (1)
    {
        pipe = CreateFile(
            name,
            GENERIC_READ |
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (pipe != INVALID_HANDLE_VALUE)
        {
            break;
        }

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            cout << "ERROR: Could not open pipe " << GetLastError();
            return NULL;
        }

        if (!WaitNamedPipe(name, 3000))
        {
            cout << "Could not open pipe: 3 second wait timed out." << endl;
            return NULL;
        }
    }

    return pipe;
}

void WriteToServer(HANDLE pipe, LPTSTR message)
{
    DWORD length = (lstrlen(message) + 1) * sizeof(TCHAR);

    cout << "Sending message!" << endl;

    if (!WriteFile(pipe, message, length, NULL, NULL))
    {
        throw domain_error("WriteFile to pipe failed. error " + to_string(GetLastError()));
    }

    cout << "Message sent!" << endl;
}

void ReadFromServer(HANDLE pipe)
{
    TCHAR chBuf[BUFSIZE];
    BOOL fSuccess;
    do
    {
        fSuccess = ReadFile(
            pipe,    // pipe handle 
            chBuf,    // buffer to receive reply 
            BUFSIZE*sizeof(TCHAR),  // size of buffer 
            NULL,  // number of bytes read 
            NULL);    // not overlapped 

        if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
            break;

        _tprintf(TEXT("\"%s\"\n"), chBuf);
    } while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

    if (!fSuccess)
    {
        _tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
    }
}

int main(int argc, char *argv[])
{
    LPTSTR message = TEXT("Default message from client.");

    HANDLE pipe = OpenNamedPipe(TEXT("\\\\.\\pipe\\mynamedpipe"));
    if (pipe == NULL)
    {
        cout << "ERROR: Can't open pipe!" << endl;
    }

    SetPipeMode(pipe);
    WriteToServer(pipe, message);
    ReadFromServer(pipe);

    CloseHandle(pipe);

    return 0;
}