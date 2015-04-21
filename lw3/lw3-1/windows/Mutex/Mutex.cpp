#include "stdafx.h"

using namespace std;

class CApplication
{
public:
    CApplication()
    {
        InitMutex();

        cout << "Application started successful!" << endl;
    }

    ~CApplication()
    {
        CloseHandle(m_mutex);
    }

private:
    void InitMutex()
    {
        m_mutex = CreateMutex(NULL, FALSE, TEXT("My app"));

        if (m_mutex == NULL)
        {
            throw domain_error("Unexpected error: can not create mutex!");
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            throw domain_error("Application already started!");
        }
    }

private:
    HANDLE m_mutex;
};

int _tmain(int argc, _TCHAR* argv[])
{
    try
    {
        CApplication app;
        cin.get();
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
        return 1;
    }

    return 0;
}

