#include "stdafx.h"
#include "Connection.h"

using namespace std;

Connection::Connection(wstring const& pipePath)
    : pipe(Pipe(pipePath))
{
    pipe.Create();
    pipe.Connect();
}


Connection::~Connection()
{
    pipe.Flush();
    pipe.Disconnect();
}

wstring Connection::Recive()
{
    return pipe.Read();
}

void Connection::Send(wstring const& message)
{
    pipe.Write(message);
}