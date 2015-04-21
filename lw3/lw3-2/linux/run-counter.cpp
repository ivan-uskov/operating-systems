#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

using namespace std;

class Application
{
public:
    Application(string name, int count) //new
        : m_uses(false)
        , semName(name)
        , m_count(count)
    {
        m_semId = sem_open(name.c_str(), O_CREAT | O_EXCL, S_IRWXU | S_IRWXO | S_IRWXG, m_count);

        if (m_semId == SEM_FAILED)
        {
            ProcessError();
        }
    }

    Application(string name) //join
        : m_uses(false)
        , m_count(0)
    {
        m_semId = sem_open(name.c_str(), O_CREAT);

        if (m_semId == SEM_FAILED)
        {
            ProcessError();
        }
    }

    ~Application()
    {
        if (m_uses)
        {
            sem_post(m_semId);
        }

        if (IsSemaphoreNotUsed())
        {
            cout << "Semaphore deleted!" << endl;
            sem_unlink(semName.c_str());
        }
        else
        {
            sem_close(m_semId);
        }
    }

    void Lock()
    {
        if (sem_trywait(m_semId) != 0)
        {
             ProcessError();
        }

        m_uses = true;
    }

private:
    bool IsSemaphoreNotUsed()
    {
        if (m_count) // if this process create sem
        {
            int currVal;
            return (sem_getvalue(m_semId, &currVal) == 0 && currVal == m_count);
        }

         return false;
    }

    void ProcessError()
    {
        switch (errno)
        {
            case EAGAIN:
                throw runtime_error("Semaphore value is zero, has no free resourses!");
            case EEXIST:
                throw runtime_error("Can't create semaphore with this exists name!");
            default:
                throw runtime_error(("Unexpected error: " + to_string(errno)).c_str());
        }
    }

private:
    sem_t * m_semId;
    bool m_uses;
    int m_count;
    string semName;
};

bool ParseNew(vector<string> const& params, string & name, int & count)
{
    if (params.size() == 4 && params[1] == "new")
    {
        count = stoi(params[3]);
        name = params[2];
        return count > 0;
    }

    return false;
}

bool ParseJoin(vector<string> const& params, string & name)
{
    if (params.size() == 3 && params[1] == "join")
    {
        name = params[2];
        return true;
    }

    return false;
}

Application InitAppFromCmdParams(vector<string> const& params)
{
    string name;
    int count;

    if (ParseNew(params, name, count))
    {
        return Application(name, count);
    }
    else if (ParseJoin(params, name))
    {
        return Application(name);
    }
    else
    {
        throw runtime_error("Invalid params!");
    }
}

int main(int argc, char* argv[])
{
    try
    {
        auto app = InitAppFromCmdParams(vector<string>(argv, argv + argc));

        app.Lock();

        cout << "1 resourses uses" << endl;
        cout << "Enter any key for exit!" << endl;
        cin.get();
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
        cout << "Usage:: <new|join> <program name> [if <new>: programs count]" << endl;
        cout << "Like:" << endl;
        cout << "\tnew MyProg 50" << endl;
        cout << "\tjoin MyProg" << endl;

        return 1;
    }

    return 0;
}