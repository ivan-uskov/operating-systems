#pragma once
#include "../chat-server/Pipe.h"

class Client
{
public:
    Client(std::wstring const& serverName);
    ~Client();

    void Send(std::wstring const& message);
    std::wstring Recive();
private:
    Pipe pipe;
};

