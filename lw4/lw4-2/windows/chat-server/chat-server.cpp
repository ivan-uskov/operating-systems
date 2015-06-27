#include "stdafx.h"

using namespace std;

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
} PIPEINST, *LPPIPEINST;

VOID DisconnectAndClose(LPPIPEINST);
BOOL CreateAndConnectInstance(LPOVERLAPPED);
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
VOID GetAnswerToRequest(LPPIPEINST);

VOID WINAPI CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED);
VOID WINAPI CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED);

HANDLE hPipe;

int _tmain(VOID)
{
    HANDLE hConnectEvent;
    OVERLAPPED oConnect;
    LPPIPEINST lpPipeInst;
    DWORD dwWait, cbRet;
    BOOL fSuccess, fPendingIO;

    hConnectEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (hConnectEvent == NULL)
    {
        printf("CreateEvent failed with %d.\n", GetLastError());
        return 0;
    }

    oConnect.hEvent = hConnectEvent;
    fPendingIO = CreateAndConnectInstance(&oConnect);

    while (1)
    {
        dwWait = WaitForSingleObjectEx(hConnectEvent, INFINITE, TRUE);

        switch (dwWait)
        {
        case 0:
            if (fPendingIO)
            {
                fSuccess = GetOverlappedResult(hPipe, &oConnect, &cbRet, FALSE);
                if (!fSuccess)
                {
                    printf("ConnectNamedPipe (%d)\n", GetLastError());
                    return 0;
                }
            }

            lpPipeInst = (LPPIPEINST)GlobalAlloc(
                GPTR, sizeof(PIPEINST));
            if (lpPipeInst == NULL)
            {
                printf("GlobalAlloc failed (%d)\n", GetLastError());
                return 0;
            }

            lpPipeInst->hPipeInst = hPipe;

            lpPipeInst->cbToWrite = 0;
            CompletedWriteRoutine(0, 0, (LPOVERLAPPED)lpPipeInst);

            fPendingIO = CreateAndConnectInstance(&oConnect);
            break;

        case WAIT_IO_COMPLETION:
            break;

        default:
        {
            printf("WaitForSingleObjectEx (%d)\n", GetLastError());
            return 0;
        }
        }
    }
    return 0;
}

VOID WINAPI CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fRead = FALSE;
    lpPipeInst = (LPPIPEINST)lpOverLap;

    if ((dwErr == 0) && (cbWritten == lpPipeInst->cbToWrite))
        fRead = ReadFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chRequest,
            BUFSIZE*sizeof(TCHAR),
            (LPOVERLAPPED)lpPipeInst,
            (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine);


    if (!fRead)
        DisconnectAndClose(lpPipeInst);
}

VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fWrite = FALSE;

    lpPipeInst = (LPPIPEINST)lpOverLap;

    if ((dwErr == 0) && (cbBytesRead != 0))
    {
        GetAnswerToRequest(lpPipeInst);

        fWrite = WriteFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chReply,
            lpPipeInst->cbToWrite,
            (LPOVERLAPPED)lpPipeInst,
            (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedWriteRoutine);
    }

    if (!fWrite)
        DisconnectAndClose(lpPipeInst);
}

VOID DisconnectAndClose(LPPIPEINST lpPipeInst)
{
    if (!DisconnectNamedPipe(lpPipeInst->hPipeInst))
    {
        printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }

    CloseHandle(lpPipeInst->hPipeInst);

    if (lpPipeInst != NULL)
        GlobalFree(lpPipeInst);
}

BOOL CreateAndConnectInstance(LPOVERLAPPED lpoOverlap)
{
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\myserver");

    hPipe = CreateNamedPipe(
        lpszPipename,
        PIPE_ACCESS_DUPLEX |
        FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE |
        PIPE_READMODE_MESSAGE |
        PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        BUFSIZE*sizeof(TCHAR),
        BUFSIZE*sizeof(TCHAR),
        PIPE_TIMEOUT,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        printf("CreateNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }

    return ConnectToNewClient(hPipe, lpoOverlap);
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
        {
            fPendingIO = TRUE;
            break;
        }
        case ERROR_PIPE_CONNECTED:
        {
            if (SetEvent(lpo->hEvent))
                break;
        }
        default:
        {
            printf("ConnectNamedPipe failed with %d.\n", GetLastError());
            return 0;
        }
    }
    return fPendingIO;
}

VOID GetAnswerToRequest(LPPIPEINST pipe)
{
    _tprintf(TEXT("[%d] %s\n"), pipe->hPipeInst, pipe->chRequest);
    StringCchCopy(pipe->chReply, BUFSIZE, TEXT("Default answer from server"));
    pipe->cbToWrite = (lstrlen(pipe->chReply) + 1)*sizeof(TCHAR);
}
