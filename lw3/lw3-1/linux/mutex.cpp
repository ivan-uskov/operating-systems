#include <iostream>
#include <stdexcept>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

using namespace std;

class Mutex
{
public:
    Mutex(char const* name)
    {
        m_id = sem_open(name, O_CREAT | O_EXCL);

        if (errno == EEXIST)
        {
            throw runtime_error("Mutex already created!");
        }
    }

    ~Mutex()
    {
        sem_close(m_id);
    }

private:
    sem_t * m_id;
};

int main(int argc, char* argv[])
{
    try
    {
        Mutex mutex("My first mutex blah blah blah!!!");

        cout << "Application started succesfully!" << endl;
        cin.get();
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
        return 1;
    }

    return 0;
}
