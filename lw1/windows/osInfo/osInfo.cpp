#include "stdafx.h"
#pragma comment(lib, "Version.lib")

using namespace std;

PLONG GetFileVersionInfoBuffer(TCHAR const* filePath)
{
    DWORD infoSize = GetFileVersionInfoSize(filePath, NULL);
    if (infoSize <= 0)
    {
        throw invalid_argument("GetFileVersionInfoSize error!");
    }

    PLONG infoBuffer = new LONG[infoSize];
    if (!GetFileVersionInfo(filePath, NULL, infoSize, infoBuffer))
    {
        throw domain_error("GetFileVersionInfo error!");
    }

    return infoBuffer;
}

PLONG GetKernel32FileVersionInfoBuffer()
{
    HMODULE handle = GetModuleHandle(_T("kernel32.dll"));
    if (!handle)
    {
        throw exception("Can't read system version!");
    }

    TCHAR filePath[MAX_PATH + 1] = { 0 };
    GetModuleFileName(handle, filePath, MAX_PATH);

    return GetFileVersionInfoBuffer(filePath);
}

void PrintWindowsVersion()
{
    PLONG infoBuffer = GetKernel32FileVersionInfoBuffer();

    UINT cpSize;
    LANGANDCODEPAGE * pLangCodePage;
    if (!VerQueryValue(infoBuffer, _T("\\VarFileInfo\\Translation"), (LPVOID*)&pLangCodePage, &cpSize))
    {
        throw exception("Can't read translations!");
    }

    TCHAR paramName[256];
    _stprintf_s(paramName,
        _T("\\StringFileInfo\\%04x%04x\\ProductVersion"),
        pLangCodePage->wLanguage,
        pLangCodePage->wCodePage);

    TCHAR * paramValue = NULL;
    VerQueryValue(infoBuffer, paramName, (LPVOID*)&paramValue, NULL);

    if (!paramValue)
    {
        throw exception("Can't read system version!");
    }

    wcout << paramValue << endl;
    delete[] infoBuffer;
}

int main(int argc, TCHAR * argv[])
{
    try
    {
        PrintWindowsVersion();
    }
    catch (exception const& e)
    {
        cout << e.what() << endl;
        return 1;
    }

    return 0;
}
