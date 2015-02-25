
#include "stdafx.h"

#pragma hdrstop

#include <tchar.h>
#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <string>

//---------------------------------------------------------------------------
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "Version.lib")

#define ARRAY_SIZE 1024

using namespace std;

//Вывод русских букв в консоль
ostream & operator << (ostream & out, TCHAR const* str)
{
    char s[256];
    CharToOem(str, s);

    for (int i = 0; s[i] != 0; i++)
    {
        out << s[i];
    }

    return out;
}

struct LANGANDCODEPAGE
{ // структура для получения кодовых страниц по языкам трансляции ресурсов файла
    WORD wLanguage;
    WORD wCodePage;
};

PLONG GetFileVersionInfoBuffer(TCHAR const* fileName)
{
    DWORD infoSize = GetFileVersionInfoSize(fileName, NULL);
    if (infoSize <= 0)
    {
        throw invalid_argument("File not exists!");
    }

    PLONG infoBuffer = new LONG[infoSize];
    if (!GetFileVersionInfo(fileName, NULL, infoSize, infoBuffer))
    {
        throw domain_error("GetFileVersionInfo error!");
    }

    return infoBuffer;
}

string const GetWindowsVersion()
{
    TCHAR const* fileName = TEXT("c:/windows/system32/kernel32.dll");
    PLONG infoBuffer = GetFileVersionInfoBuffer(fileName);

    UINT cpSize;
    LANGANDCODEPAGE *pLangCodePage;
    if (VerQueryValue(infoBuffer, _T("\\VarFileInfo\\Translation"), (LPVOID*)&pLangCodePage, &cpSize))
    {
        TCHAR * paramValue = nullptr;
        for (int cpIdx = 0; cpIdx < (int)(cpSize / sizeof(struct LANGANDCODEPAGE)); ++cpIdx)
        {
            TCHAR paramNameBuf[256];
            _stprintf_s(paramNameBuf, 
                _T("\\StringFileInfo\\%04x%04x\\ProductVersion"),
                pLangCodePage[cpIdx].wLanguage,
                pLangCodePage[cpIdx].wCodePage);
                
            cout << paramNameBuf;
            UINT paramSz;
            VerQueryValue(infoBuffer, TEXT("\\StringFileInfo\\040904b0\\ProductVersion"), (LPVOID*)&paramValue, &paramSz);
        }

        cout << paramValue << endl;
    }

    delete [] infoBuffer;
    return string();
}

int main(int argc, TCHAR * argv[])
{
    try
    {
        cout << GetWindowsVersion();
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
        return 1;
    }

    return 0;
}