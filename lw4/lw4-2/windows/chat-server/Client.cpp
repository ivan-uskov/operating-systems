#include "stdafx.h"
#include "Client.h"

using namespace std;

Client::Client(wstring const& serverName)
    : pipe(Pipe(Paths::PIPE + serverName))
{
    pipe.Create();
    pipe.Connect();
}


Client::~Client()
{
    pipe.Flush();
    pipe.Disconnect();

    cout << "Client disconnected!" << endl;
}

wstring Client::Recive()
{
    return pipe.Read();
}

void Client::Send(wstring const& message)
{
    pipe.Write(message);
}