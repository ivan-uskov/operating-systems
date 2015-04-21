#include <iostream>
#include <pthread.h>

using namespace std;

const int THREADCOUNT = 3;
bool threads[THREADCOUNT];
int turn;

void InitThreadsInterests()
{
    for (int i = 0; i < THREADCOUNT; ++i)
    {
        threads[i] = false;
    }

    turn = 0;
}

int Next(int id)
{
    return ((++id) < THREADCOUNT) ? id : 0;
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

void * Thread(void * threadId)
{
    int id = *((int*)threadId);

    for (int i = 0; i < 5; ++i)
    {
        ResourseLock(id);
        CriticalSection(id);
        ResourseFree(id);
    }

    return 0;
}

void InitIds(int * ids)
{
    for (int i = 0; i < THREADCOUNT; ++i)
    {
        ids[i] = i;
    }
}

int main(int argc, char* argv[])
{
    InitThreadsInterests();
    pthread_t pthreads[THREADCOUNT];
    int ids[THREADCOUNT];
    InitIds(ids);

    for (int i = 0; i < THREADCOUNT; ++i)
    {
        int resultCode = pthread_create(pthreads + i, NULL, Thread, (void*)(ids + i));
        if (resultCode != 0)
        {
            cout << "CreateThread error: " << i << endl;
            return 1;
        }
    }

    for (int i = 0; i < THREADCOUNT; ++i)
    {
        int resultCode = pthread_join(pthreads[i], NULL);
        if (resultCode != 0)
        {
            cout << "Process error: " << resultCode << endl;
            return 1;
        }
    }

    return 0;
}