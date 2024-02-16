#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "modules/processes_threads.h"
#include "utils/utils.h"

#define TIME_STR_FORMAT "  %01d:%02d:%02d:%03d  "

void close_handle(HANDLE handle) {
	if (handle == NULL) {
		fprintf(stderr, "Erreur : Tentative de fermeture d'un handle NULL.\n");
		return;
	}

	if (!CloseHandle(handle)) {
		DWORD errorCode = GetLastError();
		fprintf(stderr, "Erreur : CloseHandle a échoué avec le code d'erreur %lu.\n", errorCode);
	}
}

void catch_function_error(LPCTSTR functionName) {
	DWORD errorCode = GetLastError();
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	fprintf(
		stderr,
		"Une erreur s'est produite lors de l'appel à la fonction %s : %s\n",
		functionName,
		(LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);

	exit(EXIT_FAILURE);
}

void display_system_time(SYSTEMTIME* time) {
	printf(TIME_STR_FORMAT, time->wHour, time->wMinute, time->wSecond, time->wMilliseconds);
}

void display_process_entry(
	PROCESSENTRY32* ptrProcessEntry,
	LONG processHandleCount,
	LONG processPrivateVirtualMemoryUsage,
	SYSTEMTIME* ptrProcessCpuTime,
	SYSTEMTIME* ptrProcessElapsedTime
	) {
	char phcBuffer[10];
	char pvmBuffer[30];

	printf("%-28s %-8lu %-8lu %-8lu %-8s %-8s",
		(ptrProcessEntry->th32ProcessID == 0) ? "Idle" : ptrProcessEntry->szExeFile,
		ptrProcessEntry->th32ProcessID,
		ptrProcessEntry->pcPriClassBase,
		ptrProcessEntry->cntThreads,
		// Convertir processHandleCount en chaîne de caractères
		(processHandleCount == 0) ? "N/A" : itoa(processHandleCount, phcBuffer, 10),
		// Convertir processPrivateVirtualMemoryUsage en chaîne de caractères
		(processPrivateVirtualMemoryUsage == 0) ? "N/A" : itoa(processPrivateVirtualMemoryUsage, pvmBuffer, 10));

	// Affichage du temps CPU
	if (ptrProcessCpuTime != NULL)
		display_system_time(ptrProcessCpuTime);
	else
		printf("N/A ");

	// Affichage du temps écoulé
	if (ptrProcessElapsedTime != NULL)
		display_system_time(ptrProcessElapsedTime);
	else
		printf("N/A ");

	printf("\n");
}

void display_thread_entry(
	THREADENTRY32* ptrThreadEntry,
	LONG contextSwitches,
	LONG threadState,
	LONG waitReason,
	SYSTEMTIME* ptrThreadUserTime,
	SYSTEMTIME* ptrThreadKernelTime,
	SYSTEMTIME* ptrThreadElapsedTime
	) {
	printf("%-8lu %-8lu %-8ld ", ptrThreadEntry->th32ThreadID, ptrThreadEntry->tpBasePri, contextSwitches);

	// Affichage de l'état du thread
	const char* threadStateStr = get_thr_state_str(threadState);
	if (threadState != _Waiting)
		printf("%-8s ", threadStateStr);
	else
		printf("%s:%s ", threadStateStr, get_thr_wait_reason_str(waitReason));

	// Affichage du temps utilisateur
	if (ptrThreadUserTime != NULL)
		display_system_time(ptrThreadUserTime);
	else
		printf("N/A ");

	// Affichage du temps kernel
	if (ptrThreadKernelTime != NULL)
		display_system_time(ptrThreadKernelTime);
	else
		printf("N/A ");

	// Affichage du temps écoulé
	if (ptrThreadElapsedTime != NULL)
		display_system_time(ptrThreadElapsedTime);
	else
		printf("N/A ");

	printf("\n");
}

FILETIME* get_cpu_time(FILETIME* kernelTime, FILETIME* userTime) {
	ULARGE_INTEGER kernelTime64bits;
	ULARGE_INTEGER userTime64bits;

	kernelTime64bits.LowPart = kernelTime->dwLowDateTime;
	kernelTime64bits.HighPart = kernelTime->dwHighDateTime;

	userTime64bits.LowPart = userTime->dwLowDateTime;
	userTime64bits.HighPart = userTime->dwHighDateTime;

	ULARGE_INTEGER processCpuTime64bits;
	processCpuTime64bits.QuadPart = kernelTime64bits.QuadPart + userTime64bits.QuadPart;

	FILETIME* processCpuTime = (FILETIME*)malloc(sizeof(FILETIME));
	if (processCpuTime == NULL) {
		fprintf(stderr, "Erreur lors de l'allocation dynamique de mémoire.");
		exit(EXIT_FAILURE);
	}

	processCpuTime->dwLowDateTime = processCpuTime64bits.LowPart;
	processCpuTime->dwHighDateTime = processCpuTime64bits.HighPart;

	return processCpuTime;
}

FILETIME* get_elapsed_time(FILETIME* time) {
	FILETIME currentTime;

	GetSystemTimeAsFileTime(&currentTime);

	ULARGE_INTEGER currentTime64bits;
	ULARGE_INTEGER time64bits;

	currentTime64bits.LowPart = currentTime.dwLowDateTime;
	currentTime64bits.HighPart = currentTime.dwHighDateTime;

	time64bits.LowPart = time->dwLowDateTime;
	time64bits.HighPart = time->dwHighDateTime;

	ULARGE_INTEGER elapsedTime64bits;
	elapsedTime64bits.QuadPart = currentTime64bits.QuadPart - time64bits.QuadPart;

	FILETIME* elapsedTime = (FILETIME*)malloc(sizeof(FILETIME));
	if (elapsedTime == NULL) {
		fprintf(stderr, "Erreur lors de l'allocation dynamique de mémoire.");
		exit(EXIT_FAILURE);
	}

	elapsedTime->dwLowDateTime = elapsedTime64bits.LowPart;
	elapsedTime->dwHighDateTime = elapsedTime64bits.HighPart;

	return elapsedTime;
}

void set_SeDebugPrivilege() {
	LUID privilegeLocallyUniqueIdentifier;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &privilegeLocallyUniqueIdentifier)) {
		catch_function_error("LookupPrivilegeValue()");
	}

	HANDLE currentProcessHandle = GetCurrentProcess();

	HANDLE currentProcessAccessTokenHandle;
	if (!OpenProcessToken(currentProcessHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
			&currentProcessAccessTokenHandle)) {
		catch_function_error("OpenProcessToken()");
			}

	TOKEN_PRIVILEGES accessTokenNewPrivileges;
	accessTokenNewPrivileges.PrivilegeCount = 1;
	accessTokenNewPrivileges.Privileges[0].Luid = privilegeLocallyUniqueIdentifier;
	accessTokenNewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(
			currentProcessAccessTokenHandle,
			FALSE,
			&accessTokenNewPrivileges,
			sizeof(TOKEN_PRIVILEGES),
			NULL, NULL) || GetLastError() == ERROR_NOT_ALL_ASSIGNED
			) {
				fprintf(
					stderr,
					"Les infos système ne sont pas toutes disponibles car la console n'est pas en mode administrateur.\n");
			}

	CloseHandle(currentProcessAccessTokenHandle);
}

void print_line_of_dashes(const int numHeaders, const int headerWidth) {
	// '+1' pour chaque séparateur '|', et 1 pour le dernier '|'
	const int totalLength = numHeaders * (headerWidth + 1) + 1;

	// Affiche la ligne de séparation avec des tirets
	for (int i = 0; i < totalLength; i++) {
		printf("-");
	}
	printf("\n");
}

void print_table_header(char *headers[], const int numHeaders, const int headerWidth) {
	print_line_of_dashes(numHeaders, headerWidth);

	// Affiche les en-têtes
	for (int i = 0; i < numHeaders; i++) {
		printf("| %-*s ", headerWidth - 2, headers[i]);
	}
	printf("|\n");

	print_line_of_dashes(numHeaders, headerWidth);
}

void show_process_list_titles() {
	char *headers[] = {
		"Name",
		"Pid",
		"Pri",
		"Thd",
		"Hnd",
		"Priv",
		"CPU Time",
		"Elapsed Time"
	};
	const int numHeaders = sizeof(headers) / sizeof(headers[0]);
	const int headerWidth = 12;

	print_table_header(headers, numHeaders, headerWidth);
}

void show_thr_list_titles() {
	char *headers[] = {"Tid", "Pri", "Cswtch", "State", "User Time", "Kernel Time", "Elapsed Time"};
	const int numHeaders = sizeof(headers) / sizeof(headers[0]);
	const int headerWidth = 12;

	print_table_header(headers, numHeaders, headerWidth);
}

void show_mem_list_titles() {
	char *headers[] = {
		"Name",
		"Pid",
		"PageFaultCount",
		"PeakWorkingSetSize",
		"WorkingSetSize",
		"QuotaPeakPagedPoolUsage",
		"QuotaPagedPoolUsage",
		"QuotaNonPagedPoolUsage"
	};
	const int numHeaders = sizeof(headers) / sizeof(headers[0]);
	const int headerWidth = 28;

	print_table_header(headers, numHeaders, headerWidth);
}
