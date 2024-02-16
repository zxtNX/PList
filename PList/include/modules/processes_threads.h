#ifndef PROCESSES_THREADS_H_
#define PROCESSES_THREADS_H_
#include <winternl.h>
#include <winbase.h>
#include <tlhelp32.h> // TH32CS_SNAPPROCESS


typedef struct _SYSTEM_THREAD_INFORMATION_ARRAY {
	SYSTEM_THREAD_INFORMATION* array;
	ULONG arrayLength;
} SYSTEM_THREAD_INFORMATION_ARRAY ;


// Source : https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/win32-thread
enum THREAD_STATE_NUM
{
	_Other = 1,
	_Running = 2,
	_Ready = 3,
	_Blocked = 4,
	_Waiting = 5
};

// Source : https://www.nirsoft.net/kernel_struct/vista/KWAIT_REASON.html
enum WAIT_REASON_NUM
{
    _Executive = 0,
	_FreePage = 1,
	_PageIn = 2,
	_PoolAllocation = 3,
	_DelayExecution = 4,
	_Suspended = 5,
	_UserRequest = 6,
	_WrExecutive = 7,
	_WrFreePage = 8,
	_WrPageIn = 9,
	_WrPoolAllocation = 10,
	_WrDelayExecution = 11,
	_WrSuspended = 12,
	_WrUserRequest = 13,
	_WrEventPair = 14,
	_WrQueue = 15,
	_WrLpcReceive = 16,
	_WrLpcReply = 17,
	_WrVirtualMemory = 18,
	_WrPageOut = 19,
	_WrRendezvous = 20,
	_Spare2 = 21,
	_Spare3 = 22,
	_Spare4 = 23,
	_Spare5 = 24,
	_WrCalloutStack = 25,
	_WrKernel = 26,
	_WrResource = 27,
	_WrPushLock = 28,
	_WrMutex = 29,
	_WrQuantumEnd = 30,
	_WrDispatchInt = 31,
	_WrPreempted = 32,
	_WrYieldExecution = 33,
	_WrFastMutex = 34,
	_WrGuardedMutex = 35,
	_WrRundown = 36,
	_MaximumWaitReason  = 37
};

struct _SYSTEM_THREAD_INFORMATION_ARRAY* get_thr_info_by_id(DWORD pId);
struct _SYSTEM_THREAD_INFORMATION_ARRAY* get_thrInfo_querySystemResults_by_pId(
	BYTE* ntQuerySystemInformationResults,
	DWORD pId);

char* get_thr_state_str(LONG thrState);
char* get_thr_wait_reason_str(LONG waitReason);

void get_thrInfo(THREADENTRY32* thrEntry, SYSTEM_THREAD_INFORMATION_ARRAY* thrInfo);
void list_thr_by_pid(DWORD pId);

SYSTEMTIME* get_thr_times(HANDLE thrHandle);
LONG* get_ctxSwitches_by_thrId(SYSTEM_THREAD_INFORMATION_ARRAY* thrInfo, DWORD thrId);

#endif // PROCESSES_THREADS_H_


