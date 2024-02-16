#ifndef PROCESSES_LIST_H_
#define PROCESSES_LIST_H_
#include <tlhelp32.h>
#include <windows.h>

void get_process_info(PROCESSENTRY32* ptrProcessEntry);
void list_processes();

SYSTEMTIME* get_times(HANDLE processHandle);

DWORD get_process_open_handles_number(HANDLE processHandle);
DWORD get_process_private_vmem_usage(HANDLE processHandle);

#endif // PROCESSES_LIST_H_
