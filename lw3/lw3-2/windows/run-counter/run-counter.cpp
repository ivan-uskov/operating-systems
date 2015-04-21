#include "stdafx.h"

using namespace std;

class Application
{
public:
    Application(_TCHAR const* semName, unsigned processCount)
        : m_uses(false)
    {
        m_semHandle = CreateSemaphore(NULL, processCount, processCount, semName);
        CheckSemError();
    }

    Application(_TCHAR const* semName)
        : m_uses(false)
    {
        m_semHandle = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, semName);
        CheckSemError();
    }

    ~Application()
    {
        long previos;

        if (m_uses && ReleaseSemaphore(m_semHandle, 1, &previos))
        {
            cout << "Semaphore released free resourses count: " << previos + 1 << endl;
        }

        CloseHandle(m_semHandle);
    }

public:
    void Lock()
    {
        if (WaitForSingleObject(m_semHandle, 0L) != WAIT_OBJECT_0)
        {
            throw domain_error("Semaphore has no free resourses!");
        }

        m_uses = true;
    }

private:
    void CheckSemError()
    {
        if (m_semHandle == NULL)
        {
            auto str = string("Semaphore error: ") + to_string(GetLastError());
            throw invalid_argument(str.c_str());
        }
    }

private:
    HANDLE m_semHandle;
    bool m_uses;
};

vector<wstring> InitCmdParams()
{
    int argc;
    wchar_t * const * const argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

    return vector<wstring>(argv, argv + argc);
}

Application InitAppFromCmdParams(vector<wstring> const& params)
{
    if (params.size() == 4 && params[1] == L"new")
    {
        return Application(params[2].c_str(), stoi(params[3].c_str()));
    }
    else if (params.size() == 3 && params[1] == L"join")
    {
        return Application(params[2].c_str());
    }
    else
    {
        throw invalid_argument("Invalid params!");
    }
}

int main()
{
    try
    {
        auto params = InitCmdParams();
        auto app = InitAppFromCmdParams(params);

        app.Lock();

        cout << "1 resourse using!" << endl;
        cin.get();
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
        cout << "Usage:: <new|join> <program name> [if <new> programs count]" << endl;
        cout << "Like:" << endl;
        cout << "\tnew MyProg 50" << endl;
        cout << "\tjoin MyProg" << endl;
        return 1;
    }

    return 0;
}

