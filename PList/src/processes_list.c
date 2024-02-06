#include "processes_list.h"
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

void ListProcesses() {
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

        const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
        DWORD priority = 0;
        if (hProcess != NULL) {
            priority = GetPriorityClass(hProcess);
            CloseHandle(hProcess);
        }

        printf("%-30s %-6lu %-7lu %-3lu\n", processName, pe32.th32ProcessID, priority, pe32.cntThreads);
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}
