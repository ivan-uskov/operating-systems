#include "stdafx.h"

using namespace std;

const int THREADCOUNT = 3;
bool threads[THREADCOUNT];
int turn = 0;

void InitThreadsInterests()
{
    for (int i = 0; i < THREADCOUNT; ++i)
    {
        threads[i] = false;
    }
}

int Next(int id)
{
    return (++id < THREADCOUNT) ? id : 0;
}

bool IsResourseLocked(int threadId)
{
    bool free = false;

    for (int i = 0; i < THREADCOUNT; ++i)
    {
        if (i != threadId)
        {
            free = free || threads[i];
        }
    }

    return free;
}

void CriticalSection(int threadId)
{
    cout << "Start writing with thread: " << threadId << endl;
    for (int i = 0; i < 5; ++i)
    {
        cout << "Thread: " << threadId << " data: " << i << endl;
    }
    cout << "End writing with thread: " << threadId << endl;
}

void ResourseLock(int threadId)
{
    threads[threadId] = true;
    while (IsResourseLocked(threadId))
    {
        if (turn != threadId)
        {
            threads[threadId] = false;
            while (turn != threadId) {}
            threads[threadId] = true;
        }
    }
}

void ResourseFree(int threadId)
{
    turn = Next(threadId);
    threads[threadId] = false;
}

DWORD WINAPI Thread(int threadId)
{
    for (int i = 0; i < 5; ++i)
    {
        ResourseLock(threadId);
        CriticalSection(threadId);
        ResourseFree(threadId);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    InitThreadsInterests();
    HANDLE aThread[THREADCOUNT];

    for (int i = 0; i < THREADCOUNT; ++i)
    {
        aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread, (LPVOID)i, 0, NULL);
        if (aThread[i] == NULL)
        {
            cout << "CreateThread error: " + to_string(GetLastError()) << endl;
            return 1;
        }
    }

    WaitForMultipleObjects(THREADCOUNT, aThread, TRUE, INFINITE);

    for (int i = 0; i < THREADCOUNT; i++)
        CloseHandle(aThread[i]);

    return 0;
}

