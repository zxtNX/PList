#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <stdlib.h>
#include <string.h>

#include "utils/utils.h"
#include "modules/processes_memory.h"

#include <tlhelp32.h>

void list_processes_memory() {
    const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Erreur lors de la création d'un snapshot des processus.\n");
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    show_mem_list_titles(); // Affiche les titres des colonnes

    if (!Process32First(hSnapshot, &pe32)) {
        fprintf(stderr, "Erreur dans la récupération des informations sur le premier processus.\n");
        CloseHandle(hSnapshot);
        return;
    }

    do {
        const char* processName = (pe32.th32ProcessID == 0) ? "Idle" : pe32.szExeFile;

        // Ouvrir le processus pour obtenir les informations de mémoire
        const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        if (hProcess != NULL) {
            DWORD dwHandleCount;
            if (GetProcessHandleCount(hProcess, &dwHandleCount)) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
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
            }
            CloseHandle(hProcess);
        }

    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}
