#include "stdafx.h"
#include "ConnectionManager.h"

using namespace std;

int main(int argc, char *argv[])
{
    ConnectionManager cm(L"myserver");
    cout << "Server wait for connections!" << endl;
    cm.ListenAndServe();

    return 0;
}