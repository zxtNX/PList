#ifndef THREAD_QUERY_LIMITED_INFORMATION
	#define THREAD_QUERY_LIMITED_INFORMATION 0x0800
#endif

#define _WIN32_WINNT 0x0501
#define PSAPI_VERSION 2
#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004
#define STATUS_SUCCESS 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <winbase.h>
#include <tlhelp32.h>

#include "modules/processes_threads.h"
#include "utils/utils.h"

char* get_thr_state_str(LONG threadState) {
	switch (threadState) {
		case -1 : return "N/A";
		case _Other : return "Other";
		case _Running : return "Running";
		case _Ready : return "Standby";
		case _Blocked : return "Blocked";
		case _Waiting : return "Wait";

		default : return "Unknown";
	}
}

char* get_thr_wait_reason_str(LONG waitReason) {
	switch (waitReason) {
		case -1 : return "N/A";
		case _Executive : return "Executive";
		case _FreePage : return "FreePage";
		case _PageIn : return "PageIn";
		case _PoolAllocation : return "PoolAllocation";
		case _DelayExecution : return "DelayExecution";
		case _Suspended : return "Suspended";
		case _UserRequest : return "UserReq";
		case _WrExecutive : return "Executive";
		case _WrFreePage : return "FreePage";
		case _WrPageIn : return "PageIn";
		case _WrPoolAllocation : return "PoolAllocation";
		case _WrDelayExecution : return "DelayExecution";
		case _WrSuspended : return "Suspended";
		case _WrUserRequest : return "UserReq";
		case _WrEventPair : return "EventPair";
		case _WrQueue : return "Queue";
		case _WrLpcReceive : return "LpcReceive";
		case _WrLpcReply : return "LpcReply";
		case _WrVirtualMemory : return "VirtualMemory";
		case _WrPageOut : return "PageOut";
		case _WrRendezvous : return "Rendezvous";
		case _Spare2 : return "Spare2";
		case _Spare3 : return "Spare3";
		case _Spare4 : return "Spare4";
		case _Spare5 : return "Spare5";
		case _WrCalloutStack : return "CalloutStack";
		case _WrKernel : return "Kernel";
		case _WrResource : return "Resource";
		case _WrPushLock : return "PushLock";
		case _WrMutex : return "Mutex";
		case _WrQuantumEnd : return "QuantumEnd";
		case _WrDispatchInt : return "DispatchInt";
		case _WrPreempted : return "Preempted";
		case _WrYieldExecution : return "YieldExecution";
		case _WrFastMutex : return "FastMutex";
		case _WrGuardedMutex : return "GuardedMutex";
		case _WrRundown : return "Rundown";
		case _MaximumWaitReason : return "MaximumWaitReason";

		default : return "Unknown";
	}
}

LONG* get_ctxSwitches_by_thrId(SYSTEM_THREAD_INFORMATION_ARRAY* thrInfo, DWORD threadId) {
	LONG results[3] = {-1};

	BOOL found = FALSE;
	for(int i = 0; thrInfo != NULL && i < thrInfo->arrayLength && !found; i++) {
		if (thrInfo->array[i].ClientId.UniqueThread == (HANDLE)(DWORD_PTR) threadId) {
			results[0] = *((ULONG*)((BYTE*) &(thrInfo->array[i].ThreadState) - sizeof(ULONG)));
			results[1] = thrInfo->array[i].ThreadState;

			if (thrInfo->array[i].ThreadState == _Waiting) {
				results[2] = thrInfo->array[i].WaitReason;
			}
			found = TRUE;
		}
	}

	// Alloue dynamiquement un tableau pour stocker les résultats à renvoyer
	LONG* finalResults = malloc(3 * sizeof(LONG));
	if (!finalResults) {
		fprintf(stderr, "Erreur lors de l'allocation dynamique de mémoire.");
		exit(EXIT_FAILURE);
	}

	// Copie les résultats locaux dans le tableau alloué dynamiquement
	for (int j = 0; j < 3; j++) {
		finalResults[j] = results[j];
	}

	return finalResults;
}

SYSTEM_THREAD_INFORMATION_ARRAY* get_thrInfo_querySystemResults_by_pId(BYTE* ptrNtQuerySystemInformationResults, DWORD pId) {
	SYSTEM_PROCESS_INFORMATION* ptrProcess = (SYSTEM_PROCESS_INFORMATION*) ptrNtQuerySystemInformationResults;

	while (ptrProcess != NULL && ptrProcess->NextEntryOffset != 0) {
		DWORD_PTR ptrpId = (DWORD_PTR) ptrProcess->UniqueProcessId;

		if (pId == ptrpId) {
			if (ptrProcess->NumberOfThreads == 0) {
				return NULL;
			}
			SYSTEM_THREAD_INFORMATION_ARRAY* ptrResultStruct = malloc(sizeof(SYSTEM_THREAD_INFORMATION_ARRAY));
			if (!ptrResultStruct) {
				fprintf(stderr, "Erreur lors de l'allocation mémoire pour obtenir les changements de contexte et l'état des threads, "
						"le tableau sera affiché sans ces informations.\n");
				return NULL;
			}

			ptrResultStruct->array = (SYSTEM_THREAD_INFORMATION*)((BYTE*)ptrProcess + sizeof(SYSTEM_PROCESS_INFORMATION));
			ptrResultStruct->arrayLength = ptrProcess->NumberOfThreads;
			return ptrResultStruct;
		}
		ptrProcess = (SYSTEM_PROCESS_INFORMATION*)((BYTE*)ptrProcess + ptrProcess->NextEntryOffset);
	}

	return NULL;
}

SYSTEM_THREAD_INFORMATION_ARRAY* get_thr_info_by_id(DWORD pId) {
	// Demander la taille requise pour stocker les informations sur les processus
	ULONG returnLength = 0;
	NTSTATUS ntQueryResult = NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &returnLength);
	if (ntQueryResult != STATUS_INFO_LENGTH_MISMATCH) {
		fprintf(stderr, "Erreur lors de l'obtention de la taille du tampon nécessaire.\n");
		return NULL;
	}

	// Allouer de la mémoire pour stocker les informations sur les processus
	BYTE* memoryForNtQuerySystem = malloc(returnLength);
	if (!memoryForNtQuerySystem) {
		fprintf(stderr, "Erreur lors de l'allocation mémoire pour obtenir les informations sur les processus.\n");
		return NULL;
	}

	// Obtenir les informations sur les processus
	ntQueryResult = NtQuerySystemInformation(SystemProcessInformation, memoryForNtQuerySystem, returnLength, &returnLength);
	if (ntQueryResult != STATUS_SUCCESS) {
		fprintf(stderr, "Erreur lors de l'obtention des informations sur les processus.\n");
		free(memoryForNtQuerySystem);
		return NULL;
	}

	// Récupérer les informations sur les threads pour le processus spécifié
	SYSTEM_THREAD_INFORMATION_ARRAY* thrInfoArray = get_thrInfo_querySystemResults_by_pId(memoryForNtQuerySystem, pId);

	// Libérer la mémoire utilisée pour stocker les informations sur les processus
	free(memoryForNtQuerySystem);

	return thrInfoArray;
}


SYSTEMTIME* get_thr_times(HANDLE thrHandle) {
	FILETIME threadCreationTime;
	FILETIME threadExitTime;
	FILETIME threadKernelTime;
	FILETIME threadUserTime;

	// BOOL GetThreadTimes([in] HANDLE hThread, [out] LPFILETIME lpCreationTime, [out] LPFILETIME lpExitTime,
	// [out] LPFILETIME lpKernelTime, [out] LPFILETIME lpUserTime );
	BOOL getThreadTimesResult = GetThreadTimes(thrHandle, &threadCreationTime, &threadExitTime,
			&threadKernelTime, &threadUserTime);
	if (!getThreadTimesResult)
		catch_function_error("GetThreadTimes()");

	FILETIME* threadElapsedTime = get_elapsed_time(&threadCreationTime);

	SYSTEMTIME* threadTimes = (SYSTEMTIME*) malloc(sizeof(SYSTEMTIME) *3);
	if (threadTimes == NULL) {
		fprintf(stderr, "Une erreur s'est produite lors d'une tentative "
				"d'allocation dynamique de mémoire.");
		exit(EXIT_FAILURE);
	}

	// FileTimeToSystemTime() : Pour convertir une structure FILETIME en une heure facile à afficher pour un utilisateur.
	BOOL threadUserTimeConverted = FileTimeToSystemTime(&threadUserTime, &threadTimes[0]);
	BOOL threadKernelTimeConverted = FileTimeToSystemTime(&threadKernelTime, &threadTimes[1]);
	BOOL threadElapsedTimeConverted = FileTimeToSystemTime(threadElapsedTime, &threadTimes[2]);

	if (!threadUserTimeConverted || !threadKernelTimeConverted || !threadElapsedTimeConverted)
		catch_function_error("FileTimeToSystemTime()");

	return threadTimes;
}

void get_thrInfo(THREADENTRY32* thrEntry, SYSTEM_THREAD_INFORMATION_ARRAY* thrInfo) {
	// Valeurs par défaut qui seront utilisées s'il n'est pas possible d'obtenir un handle du thread
	SYSTEMTIME threadTimes[3] = {0};
	LONG ctxSwitchesAndThrStates[3] = {-1};

	HANDLE thrHandle = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, thrEntry->th32ThreadID);

	if (thrHandle != NULL) {
		SYSTEMTIME* ptrThreadTimes = get_thr_times(thrHandle);
		if (ptrThreadTimes != NULL) {
			memcpy(threadTimes, ptrThreadTimes, 3 * sizeof(SYSTEMTIME)); // Copier les données
		}
		close_handle(thrHandle);
	}

	LONG* ptrCtxSwitchesAndThrStates = get_ctxSwitches_by_thrId(thrInfo, thrEntry->th32ThreadID);
	if (ptrCtxSwitchesAndThrStates != NULL) {
		memcpy(ctxSwitchesAndThrStates, ptrCtxSwitchesAndThrStates, 3 * sizeof(LONG)); // Copier les données
		free(ptrCtxSwitchesAndThrStates); // Libérer la mémoire allouée
	}

	display_thread_entry(
		thrEntry,
		ctxSwitchesAndThrStates[0],
		ctxSwitchesAndThrStates[1],
		ctxSwitchesAndThrStates[2],
		&threadTimes[0],
		&threadTimes[1],
		&threadTimes[2]);
}

void list_thr_by_pid(DWORD pId) {
	SYSTEM_THREAD_INFORMATION_ARRAY* thrInfo = get_thr_info_by_id(pId);

	if (thrInfo == NULL) {
		fprintf(stderr, "Erreur lors de la récupération des informations sur les threads du processus.\n");
		return;
	}

	HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (snapshotHandle == INVALID_HANDLE_VALUE) {
		catch_function_error("CreateToolhelp32Snapshot()");
		return;
	}

	THREADENTRY32 threadEntry;
	threadEntry.dwSize = sizeof(THREADENTRY32);

	if (!Thread32First(snapshotHandle, &threadEntry)) {
		DWORD errorCode = GetLastError();
		if (errorCode == ERROR_NO_MORE_FILES) {
			fprintf(stderr, "Aucun thread n'a été détecté sur le système.\n");
		} else {
			catch_function_error("Thread32First()");
		}
		close_handle(snapshotHandle);
		return;
	}

	do {
		if (threadEntry.th32OwnerProcessID == pId) {
			get_thrInfo(&threadEntry, thrInfo);
		}
	} while (Thread32Next(snapshotHandle, &threadEntry));

	close_handle(snapshotHandle);
}
