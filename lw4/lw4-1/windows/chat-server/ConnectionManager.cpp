#include "stdafx.h"
#include "ConnectionManager.h"
#include "Connection.h"

using namespace std;

ConnectionManager::ConnectionManager(wstring const& serverName)
    : pipeName(Paths::PIPE + serverName)
{
}


void ConnectionManager::ListenAndServe()const
{
    while (1)
    {
        try
        {
            cout << "Waiting for new user" << endl;
            ProcessConnection();
            cout << "User Disconnected" << endl;
        }
        catch (exception const& e)
        {
            cout << e.what() << endl;
        }
    }
}

void ConnectionManager::ProcessConnection()const
{
    Connection con(pipeName);

    cout << "Data drom user: ";
    wcout << con.Recive() << endl;
    cout << "Sending status" << endl;
    con.Send(L"200 OK");
    cout << "Close Connection!" << endl;
}