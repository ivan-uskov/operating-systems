#pragma once

class ConnectionManager
{
public:
    ConnectionManager(std::wstring const& groupName);

    void ListenAndServe()const;

private:
    void ProcessConnection()const;

private:
    std::wstring pipeName;
};

