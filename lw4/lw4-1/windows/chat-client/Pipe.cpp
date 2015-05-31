#include "stdafx.h"
#include "Pipe.h"

using namespace std;

Pipe::Pipe(wstring const& name, unsigned long buffSize)
    : name(name)
    , buffSize(buffSize)
{
}

Pipe::~Pipe()
{
    CloseHandle(pipe);
}

wstring Pipe::Read()
{
    wstring msg, data;
    bool reading;

    do
    {
        reading = ReadOnce(data);
        msg += data;
    } while (reading);

    return msg;
}

bool Pipe::ReadOnce(wstring & msg)
{
    vector<TCHAR> buffer(buffSize);
    DWORD readedLength;

    BOOL success = ReadFile(pipe, buffer.data(), buffer.size() * sizeof(TCHAR), &readedLength, NULL);

    if (!success && GetLastError() != ERROR_MORE_DATA)
    {
        ProcessErrors("ReadFile");
    }

    auto readedCount = readedLength / sizeof(TCHAR);
    msg = wstring(buffer.begin(), buffer.begin() + readedCount);

    return !success && GetLastError() == ERROR_MORE_DATA;
}

void Pipe::Write(wstring const& message)
{
    DWORD length = message.size() * sizeof(TCHAR);
    DWORD bytesWritten;

    if (!WriteFile(pipe, message.c_str(), length, &bytesWritten, NULL))
    {
        ProcessErrors("WriteFile");
    }
}

void Pipe::Connect()
{
    if (!ConnectNamedPipe(pipe, NULL) && GetLastError() != ERROR_PIPE_CONNECTED)
    {
        throw domain_error("Error pipe unconnectable!");
    }
}

void Pipe::Disconnect()
{
    DisconnectNamedPipe(pipe);
}

void Pipe::Flush()
{
    FlushFileBuffers(pipe);
}

void Pipe::Create()
{
    pipe = CreateNamedPipe(
        name.c_str(),             // pipe name 
        PIPE_ACCESS_DUPLEX,       // read/write access 
        PIPE_TYPE_MESSAGE |       // message type pipe 
        PIPE_READMODE_MESSAGE |   // message-read mode 
        PIPE_WAIT,                // blocking mode 
        PIPE_UNLIMITED_INSTANCES, // max. instances  
        buffSize,                  // output buffer size 
        buffSize,                  // input buffer size 
        0,                        // client time-out 
        NULL);                    // default security attribute 

    if (pipe == INVALID_HANDLE_VALUE)
    {
        ProcessErrors("create new pipe");
    }
}

void Pipe::Open()
{
    pipe = INVALID_HANDLE_VALUE;

    while (pipe == INVALID_HANDLE_VALUE)
    {
        pipe = CreateFile(
            name.c_str(),
            GENERIC_READ |
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (pipe == INVALID_HANDLE_VALUE)
        {
            ProcessErrors();
        }
    }

    SetReadMode();
}

void Pipe::ProcessErrors(string const& subMsg)
{
    switch (GetLastError())
    {
    case ERROR_PIPE_BUSY:
        Wait();
        break;
    case ERROR_BROKEN_PIPE:
    case ERROR_NO_DATA:
        throw domain_error("Disconnect from server!");
    case ERROR_FILE_NOT_FOUND:
        throw runtime_error("Server doesn't started!");
    default:
        throw runtime_error("ERROR: [" + subMsg + "] code: " + to_string(GetLastError()));
    }
}

void Pipe::Wait()
{
    if (!WaitNamedPipe(name.c_str(), 3000))
    {
        throw domain_error("Could not open pipe: 3 second wait timed out.");
    }
}

void Pipe::SetReadMode()
{
    DWORD dwMode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(pipe, &dwMode, NULL, NULL))
    {
        throw domain_error("ERROR: Can't set pipe mode");
    }
}
