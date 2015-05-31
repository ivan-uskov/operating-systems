#pragma once
#include "../chat-client/Pipe.h"

class Connection
{
public:
    Connection(std::wstring const& pipePath);
    ~Connection();

    void Send(std::wstring const& message);
    std::wstring Recive();

private:
    Pipe pipe;
};

