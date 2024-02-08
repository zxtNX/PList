#include "processes_list.h"
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <psapi.h>

void ListProcesses() {
    const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Erreur lors de la creation d'un snapshot des processus.\n");
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    printf("%-30s %-6s %-7s %-3s %-4s\n", "Name", "Pid", "Pri", "Thd", "Hnd");

    if (!Process32First(hSnapshot, &pe32)) {
        fprintf(stderr, "Erreur dans la recuperation des informations sur le premier processus.\n");
        CloseHandle(hSnapshot);
        return;
    }

    do {
        const char* processName = (pe32.th32ProcessID == 0) ? "Idle" : pe32.szExeFile;

        const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        DWORD dwHandleCount = GetProcessHandleCount(hProcess, &dwHandleCount);

        printf("%-30s %-6lu %-7lu %-3lu %-4lu\n", processName, pe32.th32ProcessID, pe32.pcPriClassBase, pe32.cntThreads, dwHandleCount);

    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}

void ListProcessesMemory() {
    const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Erreur lors de la creation d'un snapshot des processus.\n");
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    PROCESS_MEMORY_COUNTERS pmc;

    printf("%-30s %-30s %-30s %-30s %-30s %-30s %-30s %-25s\n",
        "Name",
        "Pid",
        "PageFaultCount",
        "PeakWorkingSetSize",
        "WorkingSetSize",
        "QuotaPeakPagedPoolUsage",
        "QuotaPagedPoolUsage",
        "QuotaNonPagedPoolUsage");

    if (!Process32First(hSnapshot, &pe32)) {
        fprintf(stderr, "Erreur dans la recuperation des informations sur le premier processus.\n");
        CloseHandle(hSnapshot);
        return;
    }

    do {
        const char* processName = (pe32.th32ProcessID == 0) ? "Idle" : pe32.szExeFile;

        const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        DWORD dwHandleCount = GetProcessHandleCount(hProcess, &dwHandleCount);

        if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
        {
            printf("%-30s %-30lu %-30lu %-30lu %-30lu %-30lu %-30lu %-25lu\n",
            processName,
            pe32.th32ProcessID,
            pmc.PageFaultCount,
            pmc.PeakWorkingSetSize,
            pmc.WorkingSetSize,
            pmc.QuotaPeakPagedPoolUsage,
            pmc.QuotaPagedPoolUsage,
            pmc.QuotaNonPagedPoolUsage);
        }

    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}

void ListDetailedProcessInfo(const DWORD processID)
{
    //Creates a snapshot of all threads in the system. The TH32CS_SNAPTHREAD flag specifies that it should capture thread information. 0 indicates it's capturing all threads without a specific process context.
    HANDLE threadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (threadSnap == INVALID_HANDLE_VALUE)
    {
        printf("Failed to snapshot.\n");
        return;
    }

    printf("Infos du processus %lu:\n", processID);
    printf("Tid\tPri\tCswtch\tState\tUser Time\t\tKernel Time\t\tElapsed Time\n");

    //Declares a THREADENTRY32 structure that will store information about each thread in the snapshot.
    THREADENTRY32 te;
    //Initializes the size of the structure, which is required for the Thread32First and Thread32Next functions to work correctly.
    te.dwSize = sizeof(THREADENTRY32);

    //Attempts to retrieve information about the first thread in the snapshot. Returns true if successful.
    if (Thread32First(threadSnap, &te))
    {
        do
        {
            //Checks if the current thread belongs to the process identified by processID.
            if (te.th32OwnerProcessID == processID)
            {
                HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);

                printf("%7-lu\t%4-lu\t...\t...\t%02d:%02d:%02d\t\t%02d:%02d:%02d\t\t%02d:%02d:%02d\n",
                       te.th32ThreadID,
                       te.tpBasePri);
            }
            //loop to identify all threads one by one.
        } while (Thread32Next(threadSnap, &te));
    }
    else
    {
        printf("Failed to get process.\n");
    }
    CloseHandle(threadSnap);
}



