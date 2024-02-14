#include "processes_list.h"
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <psapi.h>
#include <winternl.h>
#include <ntstatus.h>

typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    KPRIORITY Priority;
    LONG BasePriority;
    ULONG ContextSwitches;
    UCHAR Padding[64];
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;


typedef NTSTATUS (NTAPI *PFN_NT_QUERY_INFORMATION_THREAD) (
    HANDLE ThreadHandle,
    THREADINFOCLASS ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
);

char* formatelapsedTime(ULONGLONG milliseconds) {
    const int seconds = (int)(milliseconds / 1000) % 60;
    const int minutes = milliseconds / (1000 * 60) % 60;
    const int hours = milliseconds / (1000 * 60 * 60) % 24;

    static char timeStr[20];
    sprintf(timeStr, "%d:%02d:%02d.%03d", hours, minutes, seconds, (int)(milliseconds % 1000));

    return timeStr;
}

char* formatCPUTime(LPFILETIME lpKernelTime) {
    const ULONGLONG kernelTimeMs = ((ULONGLONG)lpKernelTime->dwHighDateTime << 32) + lpKernelTime->dwLowDateTime;
    const int seconds = (int)(kernelTimeMs / 1000) % 60;
    const int minutes = kernelTimeMs / (1000 * 60) % 60;
    const int hours = kernelTimeMs / (1000 * 60 * 60) % 24;

    static char timeStr[20];
    sprintf(timeStr, "%d:%02d:%02d.%03d", hours, minutes, seconds, (int)(kernelTimeMs % 1000));

    return timeStr;
}

void ListProcesses() {
    const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Erreur lors de la creation d'un snapshot des processus.\n");
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    printf("%-30s %-6s %-7s %-3s %-4s %-8s %-15s %-15s\n", "Name", "Pid", "Pri", "Thd", "Hnd", "Priv", "CPU Time", "Elapsed Time");
    printf("----------------------------------------------------------------------------------------------\n");

    if (!Process32First(hSnapshot, &pe32)) {
        fprintf(stderr, "Erreur dans la recuperation des informations sur le premier processus.\n");
        CloseHandle(hSnapshot);
        return;
    }

    do {
        const char* processName = (pe32.th32ProcessID == 0) ? "Idle" : pe32.szExeFile;

        FILETIME creationTime, exitTime, kernelTime, userTime;

        const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);

        DWORD dwHandleCount;
        BOOL hasHandles = GetProcessHandleCount(hProcess, &dwHandleCount);

        PROCESS_MEMORY_COUNTERS_EX pmc;
        ULONG priv = 0; // Affiche 0 si on ne peut pas obtenir les informations sur la mémoire du processus

        // Obtenir les informations sur la mémoire du processus
        if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            priv = pmc.PrivateUsage / 1024; // Convertir en kilo-octets
        }

        GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
        const ULONGLONG elapsedTime = GetTickCount64();

        SYSTEMTIME kernelSystemTime, userSystemTime;
        FileTimeToSystemTime(&kernelTime, &kernelSystemTime);
        FileTimeToSystemTime(&userTime, &userSystemTime);

        char kernelTimeString[20];
        char userTimeString[20];
        sprintf(userTimeString, "%02d:%02d:%02d", userSystemTime.wHour, userSystemTime.wMinute, userSystemTime.wSecond);
        sprintf(kernelTimeString, "%02d:%02d:%02d", kernelSystemTime.wHour, kernelSystemTime.wMinute, kernelSystemTime.wSecond);
        
        if (hasHandles) {
            printf("%-30s %-8lu %-4lu %-4lu %-4lu %-8lu %-15s %-15s\n",
                processName,
                pe32.th32ProcessID,
                pe32.pcPriClassBase,
                pe32.cntThreads,
                dwHandleCount,
                priv,
                formatCPUTime(&kernelTime),
                formatelapsedTime(elapsedTime));
        } else {
            printf("%-30s %-8lu %-4lu %-4lu %-4d %-8lu %-15s %-15s\n",
                processName,
                pe32.th32ProcessID,
                pe32.pcPriClassBase,
                pe32.cntThreads,
                0,
                priv,
                formatCPUTime(&kernelTime),
                formatelapsedTime(elapsedTime));
        }
        CloseHandle(hProcess);
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

    printf("--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

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

const char* GetThreadState(const DWORD dwFlags) {
    switch (dwFlags) {
        case 0x00000000:
            return "Initialized";
        case 0x00000001:
            return "Ready";
        case 0x00000002:
            return "Running";
        case 0x00000004:
            return "Standby";
        case 0x00000008:
            return "Terminated";
        case 0x00000010:
            return "Wait";
        case 0x00000020:
            return "Transition";
        case 0x00000040:
            return "DeferredReady";
        case 0x00000080:
            return "GateWait";
        default:
            return "Unknown";
    }
}

// Fonction pour obtenir la raison d'attente d'un thread
DWORD GetThreadWaitReason(DWORD dwThreadId) {
    // Obtenir le handle du thread
    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadId);
    if (hThread == NULL) {
        return 0; // Erreur lors de l'ouverture du thread
    }

    // Obtenir la raison d'attente du thread
    DWORD dwWaitReason = 0;
    BOOL success = GetThreadWaitChain(hThread, &dwWaitReason, NULL, NULL, NULL, 0);
    CloseHandle(hThread);
    if (!success) {
        return 0; // Erreur lors de l'obtention de la raison d'attente
    }

    return dwWaitReason;
}

// Fonction pour obtenir une chaîne descriptive de la raison d'attente
const char* GetThreadWaitReasonString(const DWORD dwWaitReason) {
    switch (dwWaitReason) {
        case 0:
            return "Executive";
        case 1:
            return "FreePage";
        case 2:
            return "PageIn";
        case 3:
            return "PoolAllocation";
        case 4:
            return "DelayExecution";
        case 5:
            return "Suspended";
        case 6:
            return "UserRequest";
        // Ajoutez d'autres cas selon vos besoins
        default:
            return "Unknown";
    }
}

DWORD GetContextSwitchCount(HANDLE hThread, PFN_NT_QUERY_INFORMATION_THREAD pNtQueryInformationThread) {
    THREAD_BASIC_INFORMATION tbi;
    ULONG returnLength;
    NTSTATUS status = pNtQueryInformationThread(hThread, ThreadBasicInformation, &tbi, sizeof(tbi), &returnLength);

    if (status == STATUS_SUCCESS) {
        return tbi.ContextSwitches;
    }

    // printf("NtQueryInformationThread failed with status: %x\n", status);
    // printf("returnLength value is: %x\n", returnLength);

    return 0;
}

DWORD GetThreadKernelUserTimes(DWORD dwThreadId, char* kernelTimeStr, char* userTimeStr) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadId);
    if (hThread == NULL) {
        return 0;  // Échec de l'ouverture du thread
    }

    if (!GetThreadTimes(hThread, &creationTime, &exitTime, &kernelTime, &userTime)) {
        CloseHandle(hThread);
        return 0;  // Échec de l'obtention des temps du thread
    }

    // Convertir FILETIME en string lisible
    strcpy(kernelTimeStr, formatCPUTime(&kernelTime));
    strcpy(userTimeStr, formatCPUTime(&userTime));

    CloseHandle(hThread);
    return 1;  // Succès
}

void ListDetailedProcessInfo(const DWORD processID) {
    HMODULE hNtdll = LoadLibrary("ntdll.dll");
    PFN_NT_QUERY_INFORMATION_THREAD pNtQueryInformationThread = (PFN_NT_QUERY_INFORMATION_THREAD)GetProcAddress(hNtdll, "NtQueryInformationThread");

    if (!pNtQueryInformationThread) {
        printf("Impossible de charger NtQueryInformationThread.\n");
        return;
    }

    // Crée un snapshot de tous les threads du système.
    const HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnapshot == INVALID_HANDLE_VALUE) {
        printf("Failed to snapshot.\n");
        return;
    }

    printf("Infos du processus %lu:\n", processID);
    printf("%-8s %-8s %-6s %-16s %-16s %-30s %-30s %-30s\n",
        "Tid",
        "PPid",
        "Pri",
        "State",
        "Wait Reason",
        "Context Switches",
        "User Time",
        "Kernel Time");
    printf("-----------------------------------------------------------------------------------------------------------------------------------------\n");

    // Déclare une structure THREADENTRY32 pour stocker les informations sur chaque thread dans le snapshot.
    THREADENTRY32 te32;
    // Initialise la taille de la structure, qui est nécessaire pour que les fonctions Thread32First et Thread32Next fonctionnent correctement.
    te32.dwSize = sizeof(THREADENTRY32);

    do {
        if (te32.th32OwnerProcessID == processID) {
            // Obtenir l'état et la raison d'attente du thread
            const char* state = GetThreadState(te32.dwFlags);
            const DWORD dwWaitReason = GetThreadWaitReason(te32.th32ThreadID);
            const char* waitReason = GetThreadWaitReasonString(dwWaitReason);

            HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
            if (hThread != NULL) {
                DWORD contextSwitches = GetContextSwitchCount(hThread, pNtQueryInformationThread);

                char kernelTimeString[20];
                char userTimeString[20];
                GetThreadKernelUserTimes(te32.th32ThreadID, kernelTimeString, userTimeString);

                printf("%-8lu %-8lu %-6ld %-16s %-16s %-30lu %-30s %-30s\n",
                    te32.th32ThreadID,
                    te32.th32OwnerProcessID,
                    te32.tpBasePri,
                    waitReason,
                    state,
                    contextSwitches,
                    userTimeString,
                    kernelTimeString);
            }
            CloseHandle(hThread);
        }
    } while (Thread32Next(hThreadSnapshot, &te32));

    CloseHandle(hThreadSnapshot);
    FreeLibrary(hNtdll);
}