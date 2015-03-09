#include "stdafx.h"

void PrintProcessName(HANDLE hProcess)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    if (NULL != hProcess)
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
            &cbNeeded))
        {
            GetModuleBaseName(hProcess, hMod, szProcessName,
                sizeof(szProcessName) / sizeof(TCHAR));
        }
    }
    _tprintf(TEXT("Process name: %s\n"), szProcessName);
}

int PrintProcessModules(HANDLE hProcess)
{
    HMODULE hMods[1024];
    DWORD cbNeeded;

    printf("Modules:\n");

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (unsigned i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];

            if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
                sizeof(szModName) / sizeof(TCHAR)))
            {
                _tprintf(TEXT("  %s (0x%08X)\n"), szModName, hMods[i]);
            }
        }
    }

    CloseHandle(hProcess);
    return 0;
}

void PrintProcessMemoryInfo(HANDLE hProcess)
{
    PROCESS_MEMORY_COUNTERS pmc;

    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        printf("Memory info:\n");
        printf("  PageFaultCount: 0x%08X\n", pmc.PageFaultCount);
        printf("  PeakWorkingSetSize: 0x%08X\n", pmc.PeakWorkingSetSize);
        printf("  WorkingSetSize: 0x%08X\n", pmc.WorkingSetSize);
        printf("  QuotaPeakPagedPoolUsage: 0x%08X\n", pmc.QuotaPeakPagedPoolUsage);
        printf("  QuotaPagedPoolUsage: 0x%08X\n", pmc.QuotaPagedPoolUsage);
        printf("  QuotaPeakNonPagedPoolUsage: 0x%08X\n", pmc.QuotaPeakNonPagedPoolUsage);
        printf("  QuotaNonPagedPoolUsage: 0x%08X\n", pmc.QuotaNonPagedPoolUsage);
        printf("  PagefileUsage: 0x%08X\n", pmc.PagefileUsage);
        printf("  PeakPagefileUsage: 0x%08X\n", pmc.PeakPagefileUsage);
    }
}

void PrintProcessInfo(DWORD processID)
{
    auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess)
        return;

    printf("\n\n---------- Process: %d ------------\n", processID);
    PrintProcessName(hProcess);
    PrintProcessModules(hProcess);
    PrintProcessMemoryInfo(hProcess);

    CloseHandle(hProcess);
}

int main(void)
{
    DWORD aProcesses[1024], cbNeeded;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return 1;
    }

    for (unsigned i = 0; i < cbNeeded / sizeof(DWORD); ++i)
    {
        if (aProcesses[i])
        {
            PrintProcessInfo(aProcesses[i]);
        }
    }

    return 0;
}