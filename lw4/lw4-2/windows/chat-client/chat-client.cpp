#include "stdafx.h"

using namespace std;

wstring ReadMessage(wistream & in)
{
    cout << "Write message you want to send!" << endl;

    wstring str;
    getline(in, str);

    if (!in)
    {
        throw domain_error("Error on reading message");
    }

    return str;
}

wstring MyGetUserName(wistream & in)
{
    cout << "Enter your name!" << endl;
    wstring name;
    getline(in, name);
    if (!in || name.empty())
    {
        throw runtime_error("Failed read name!");
    }

    return name;
}

int main(int argc, char *argv[])
{
    try
    {
        auto userName = MyGetUserName(wcin);
        Pipe pipe(Paths::PIPE + Paths::SERVER_NAME, userName);
        pipe.Open();

        while (1)
        {
            auto message = ReadMessage(wcin);
            cout << "Sending message!" << endl;
            pipe.Write(message);
            cout << "Message sent!" << endl;
            wcout << pipe.Read() << endl;
        }
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
    }

    cout << "Press enter too exit!" << endl;
    cin.get();

    return 0;
}