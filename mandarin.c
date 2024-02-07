#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>

void printHelp()
{
    printf("Usage: pslist.exe [flag]\n");
    printf("  -h  -> Show help message.\n");
    printf("  -d  -> Show thread detail.\n");
}

void listThreads(unsigned long processID)
{
    //Creates a snapshot of all threads in the system. The TH32CS_SNAPTHREAD flag specifies that it should capture thread information. 0 indicates it's capturing all threads without a specific process context.
    HANDLE threadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (threadSnap == INVALID_HANDLE_VALUE)
    {
        printf("Failed to snapshot.\n");
        return;
    }
    //Declares a THREADENTRY32 structure that will store information about each thread in the snapshot.
    THREADENTRY32 te;
    //Initializes the size of the structure, which is required for the Thread32First and Thread32Next functions to work correctly.
    te.dwSize = sizeof(THREADENTRY32);

    //Attempts to retrieve information about the first thread in the snapshot. Returns true if successful.
    if (Thread32First(threadSnap, &te))
    {
        do
        {   //Checks if the current thread belongs to the process identified by processID.
            if (te.th32OwnerProcessID == processID)
            {
                printf("\nThread ID: %5d\n", te.th32ThreadID);
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

void listProcess(int processDetail)
{   //Creates a snapshot of all processes in the system.
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        printf("Failed to snapshot.\n");
        return;
    }

    //Declares a THREADENTRY32 structure that will store information about each thread in the snapshot.
    PROCESSENTRY32 pe;
    //Initializes the size of the structure.
    pe.dwSize = sizeof(PROCESSENTRY32);

    //Attempts to get information about the first process in the snapshot.
    if (Process32First(snapshot, &pe))
    {
        do
        {   //print Name, PID, threads, priority
            printf("Name: %-25s PID: %5d Thd: %5d Pri: %5d\n", pe.szExeFile, pe.th32ProcessID, pe.cntThreads, pe.pcPriClassBase);
            if (processDetail)
            {   //If the detail flag is true, calls ListThreads for the current process to print its threads.
                listThreads(pe.th32ProcessID);
            }
        } while (Process32Next(snapshot, &pe));
    }
    else
    {
        printf("Failed to get process.\n");
    }
    CloseHandle(snapshot);
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        listProcess(0);
    }
    else if (argc == 2 && strcmp(argv[1], "-h") == 0)
    {
        printHelp();
    }
    else if (argc == 2 && strcmp(argv[1], "-d") == 0)
    {
        listProcess(1);
    }
    else
    {
        printHelp();
    }

    return 0;
}