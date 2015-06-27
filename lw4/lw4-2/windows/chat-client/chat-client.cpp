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

int main(int argc, char *argv[])
{
    try
    {
        Pipe pipe(Paths::PIPE + L"myserver");
        pipe.Open();

        while (1)
        {
            auto message = ReadMessage(wcin);
            cout << "Sending message!" << endl;
            pipe.Write(message);
            cout << "Message sent!" << endl;
            cout << "Response: ";
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