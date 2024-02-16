#ifndef UTILS_H_
#define UTILS_H_
#include <windows.h>
#include <tlhelp32.h>

void set_SeDebugPrivilege();
void catch_function_error(LPCSTR functionName);
void close_handle(HANDLE handle);

void show_thr_list_titles();
void show_process_list_titles();
void show_mem_list_titles();

void display_process_entry(
	PROCESSENTRY32* ptrProcessEntry,
	LONG processHandleCount,
	LONG processPrivateVirtualMemoryUsage,
	SYSTEMTIME* ptrProcessCpuTime,
	SYSTEMTIME* ptrProcessElapsedTime);
void display_thread_entry(
	THREADENTRY32* ptrThreadEntry,
	LONG contextSwitches,
	LONG threadState,
	LONG waitReason,
	SYSTEMTIME* ptrThreadUserTime,
	SYSTEMTIME* ptrThreadKernelTime,
	SYSTEMTIME* ptrThreadElapsedTime);

FILETIME* get_elapsed_time(FILETIME* time);
FILETIME* get_cpu_time(FILETIME* kernelTime, FILETIME* userTime);

#endif // UTILS_H_
