#include "stdafx.h"

using namespace std;

#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
#define INSTANCES 4 
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096

typedef struct
{
    OVERLAPPED oOverlap;
    HANDLE hPipeInst;
    TCHAR chRequest[BUFSIZE];
    DWORD cbRead;
    TCHAR chReply[BUFSIZE];
    DWORD cbToWrite;
    DWORD dwState;
    BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;

HANDLE MyCreateEvent()
{
    HANDLE handle = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (handle == NULL)
    {
        throw runtime_error("CreateEvent failed with: " + to_string(GetLastError()));
    }

    return handle;
}

HANDLE MyCreateNamedPipe()
{
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\myserver");

    HANDLE handle = CreateNamedPipe(
        lpszPipename,
        PIPE_ACCESS_DUPLEX |
        FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE |
        PIPE_READMODE_MESSAGE |
        PIPE_WAIT,
        INSTANCES,
        BUFSIZE*sizeof(TCHAR),
        BUFSIZE*sizeof(TCHAR),
        PIPE_TIMEOUT,
        NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        throw runtime_error("CreateNamedPipe failed with: " + to_string(GetLastError()));
    }

    return handle;
}

VOID DisconnectAndReconnect(DWORD);
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);

PIPEINST Pipe[INSTANCES];
HANDLE hEvents[INSTANCES];

int _tmain(VOID)
{
    DWORD cbRet, dwErr;
    BOOL fSuccess;

    for (DWORD i = 0; i < INSTANCES; ++i)
    {
        hEvents[i] = MyCreateEvent();

        Pipe[i].oOverlap.hEvent = hEvents[i];

        Pipe[i].hPipeInst = MyCreateNamedPipe();

        Pipe[i].fPendingIO = ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);

        Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
    }

    while (1)
    {
        DWORD dwWait = WaitForMultipleObjects(INSTANCES, hEvents, FALSE, INFINITE);

        DWORD i = dwWait - WAIT_OBJECT_0;
        if (i < 0 || i >(INSTANCES - 1))
        {
            throw runtime_error("out of range");
        }

        if (Pipe[i].fPendingIO)
        {
            fSuccess = GetOverlappedResult(
                Pipe[i].hPipeInst,
                &Pipe[i].oOverlap,
                &cbRet,
                FALSE);

            switch (Pipe[i].dwState)
            {
            case CONNECTING_STATE:
                if (!fSuccess)
                {
                    throw runtime_error("Err: " + to_string(GetLastError()));
                }
                Pipe[i].dwState = READING_STATE;
                break;

            case READING_STATE:
                if (!fSuccess || cbRet == 0)
                {
                    DisconnectAndReconnect(i);
                    continue;
                }
                Pipe[i].cbRead = cbRet;
                Pipe[i].dwState = WRITING_STATE;
                break;

            case WRITING_STATE:
                if (!fSuccess || cbRet != Pipe[i].cbToWrite)
                {
                    DisconnectAndReconnect(i);
                    continue;
                }
                Pipe[i].dwState = READING_STATE;
                break;

            default:
            {
                throw runtime_error("Invalid pipe state");
            }
            }
        }

        switch (Pipe[i].dwState)
        {
        case READING_STATE:
            fSuccess = ReadFile(
                Pipe[i].hPipeInst,
                Pipe[i].chRequest,
                BUFSIZE*sizeof(TCHAR),
                &Pipe[i].cbRead,
                &Pipe[i].oOverlap);

            if (fSuccess && Pipe[i].cbRead != 0)
            {
                Pipe[i].fPendingIO = FALSE;
                Pipe[i].dwState = WRITING_STATE;
                continue;
            }

            dwErr = GetLastError();
            if (!fSuccess && (dwErr == ERROR_IO_PENDING))
            {
                Pipe[i].fPendingIO = TRUE;
                continue;
            }

            DisconnectAndReconnect(i);
            break;

        case WRITING_STATE:
            Pipe[i].fPendingIO = FALSE;
            Pipe[i].dwState = READING_STATE;

            for (auto it = Pipe; it < Pipe + INSTANCES; ++it)
            {
                if (it == Pipe + i || it->dwState == CONNECTING_STATE) continue;

                fSuccess = WriteFile(
                    it->hPipeInst,
                    Pipe[i].chRequest,
                    Pipe[i].cbRead,
                    &cbRet,
                    &it->oOverlap);

                if (fSuccess && cbRet == Pipe[i].cbRead)
                {
                    Pipe[i].fPendingIO = FALSE;
                    Pipe[i].dwState = READING_STATE;
                    continue;
                }

                dwErr = GetLastError();
                if (!fSuccess && (dwErr == ERROR_IO_PENDING))
                {
                    Pipe[i].fPendingIO = TRUE;
                    continue;
                }

                DisconnectAndReconnect(i);
            }

            break;

        default:
        {
            printf("Invalid pipe state.\n");
            return 0;
        }
        }
    }

    return 0;
}

VOID DisconnectAndReconnect(DWORD i)
{
    if (!DisconnectNamedPipe(Pipe[i].hPipeInst))
    {
        printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }

    Pipe[i].fPendingIO = ConnectToNewClient(
        Pipe[i].hPipeInst,
        &Pipe[i].oOverlap);

    Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
}

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
    BOOL fConnected, fPendingIO = FALSE;

    fConnected = ConnectNamedPipe(hPipe, lpo);

    if (fConnected)
    {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }

    switch (GetLastError())
    {
    case ERROR_IO_PENDING:
        fPendingIO = TRUE;
        break;

    case ERROR_PIPE_CONNECTED:
        if (SetEvent(lpo->hEvent))
            break;

    default:
    {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }
    }

    return fPendingIO;
}