#include "processes_list.h"
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <psapi.h>

void ListProcesses(const int showMemoryUsage) {
    const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Erreur lors de la création d'un snapshot des processus.\n");
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    printf("%-30s %-6s %-7s %-3s\n", "Name", "Pid", "Pri", "Thd");

    if (!Process32First(hSnapshot, &pe32)) {
        fprintf(stderr, "Erreur dans la récupération des informations sur le premier processus.\n");
        CloseHandle(hSnapshot);
        return;
    }

    do {
        const char* processName = (pe32.th32ProcessID == 0) ? "Idle" : pe32.szExeFile;

        const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        PROCESS_MEMORY_COUNTERS pmc;
        DWORD priority = 0;
        if (hProcess != NULL) {
            if (showMemoryUsage) {
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    printf("Utilisation mémoire : %lu\n", pmc.WorkingSetSize);
                }
            }
            priority = GetPriorityClass(hProcess);
            CloseHandle(hProcess);
        }

        printf("%-30s %-6lu %-7lu %-3lu\n", processName, pe32.th32ProcessID, priority, pe32.cntThreads);
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}

void ListDetailedProcessInfo(const DWORD pid) {
    const HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Erreur lors de la création d'un snapshot des threads.\n");
        return;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    printf("Infos du processus %lu:\n", pid);
    printf("Tid\t\tPri\tCswtch\tState\tUser Time\t\tKernel Time\t\tElapsed Time\n");

    // TO-DO
    if (Thread32First(hThreadSnap, &te32)) {
        do {
            if (te32.th32OwnerProcessID == pid) {
                printf("%lu\t\t...\t...\t...\t...\t\t\t...\t\t\t...\n", te32.th32ThreadID);
            }
        } while (Thread32Next(hThreadSnap, &te32));
    }

    CloseHandle(hThreadSnap);
}

