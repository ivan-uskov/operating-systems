#pragma once

class Pipe
{
public:
    Pipe(std::wstring const& name, std::wstring const& userName, long buffSize = 512);
    ~Pipe();

public:
    std::wstring Read();
    void Write(std::wstring const& message);

    void Create();
    void Open();

    void Connect();
    void Disconnect();

    void Flush();

private:
    bool ReadOnce(std::wstring & msg);

    void ProcessErrors(std::string const& subMsg = "");
    void SetReadMode();
    void Wait();

private:
    HANDLE pipe;
    std::wstring name, userName;
    unsigned long buffSize;
};

