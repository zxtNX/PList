#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Psapi.h>

#include "modules/processes_list.h"
#include "utils/utils.h"

DWORD get_process_open_handles_number(HANDLE processHandle) {
	// Récupérer le nombre de handles ouverts appartenant au processus spécifié.
	DWORD resultBuffer;
	BOOL getProcessHandleCountResult = GetProcessHandleCount(processHandle, &resultBuffer);

	// Vérifier si la fonction a réussi
	if (!getProcessHandleCountResult) {
		catch_function_error("GetProcessHandleCount()");
	}

	return resultBuffer;
}

DWORD get_process_private_vmem_usage(HANDLE processHandle) {
	// Déclarer une variable pour stocker les informations sur la mémoire du processus
	PROCESS_MEMORY_COUNTERS_EX extendedProcessMemoryStatistics = {0};

	// Récupérer des informations sur l'utilisation de la mémoire du processus spécifié.
	if (!GetProcessMemoryInfo(
		processHandle,
		(PROCESS_MEMORY_COUNTERS*)&extendedProcessMemoryStatistics,
		sizeof(extendedProcessMemoryStatistics))
		) {
		// Gérer l'erreur si la fonction échoue
		catch_function_error("GetProcessMemoryInfo()");
	}

	// Retourner la quantité totale de mémoire privée utilisée par le processus (en kilooctets)
	return (DWORD)(extendedProcessMemoryStatistics.PrivateUsage / 1024);
}

SYSTEMTIME* get_times(HANDLE processHandle) {
	FILETIME processCreationTime = {0}; // L'heure de création du processus
	FILETIME processExitTime = {0}; // L'heure de sortie du processus
	FILETIME processKernelTime = {0}; // La durée d'exécution du processus en mode noyau
	FILETIME processUserTime = {0}; // La durée d'exécution du processus en mode utilisateur

	// Récupérer les informations de temps pour le processus spécifié
	if (!GetProcessTimes(
			processHandle,
			&processCreationTime,
			&processExitTime,
			&processKernelTime,
			&processUserTime)
	) {
		// Gérer l'erreur si la fonction échoue
		catch_function_error("GetProcessTimes()");
	}

	// Calculer le temps CPU du processus
	FILETIME* processCpuTime = get_cpu_time(&processKernelTime, &processUserTime);

	// Calculer le temps écoulé depuis la création du processus
	FILETIME* processElapsedTime = get_elapsed_time(&processCreationTime);

	// Allocation mémoire pour stocker un tableau de taille 2 pour les résultats
	SYSTEMTIME* processTimes = malloc(sizeof(SYSTEMTIME) * 2);
	if (processTimes == NULL) {
		fprintf(stderr, "Une erreur s'est produite lors d'une tentative d'allocation dynamique de mémoire.");
		exit(EXIT_FAILURE);
	}

	// Convertir les temps CPU et écoulé en temps système
	if (!FileTimeToSystemTime(processCpuTime, &processTimes[0]) ||
		!FileTimeToSystemTime(processElapsedTime, &processTimes[1])) {
		// Gérer l'erreur si la fonction de conversion échoue
		catch_function_error("FileTimeToSystemTime()");
	}

	// Libérer la mémoire allouée pour les temps CPU et écoulé
	free(processCpuTime);
	free(processElapsedTime);

	return processTimes;
}

void get_process_info(PROCESSENTRY32* ptrProcessEntry) {
	// Valeurs par défaut
	LONG processHandleCount = 0;
	LONG processPrivateVirtualMemoryUsage = 0;
	SYSTEMTIME* ptrProcessCpuTime = NULL;
	SYSTEMTIME* ptrProcessElapsedTime = NULL;

	// Ouvrir le handle du processus
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ptrProcessEntry->th32ProcessID);

	if (processHandle != NULL) {
		// Récupérer les informations sur le nombre de handles ouverts et l'utilisation de la mémoire virtuelle privée
		processHandleCount = get_process_open_handles_number(processHandle);
		processPrivateVirtualMemoryUsage = get_process_private_vmem_usage(processHandle);

		// Récupérer les temps CPU et écoulé du processus
		SYSTEMTIME* processCpuTimeAndElapsedTime = get_times(processHandle);
		ptrProcessCpuTime = &processCpuTimeAndElapsedTime[0];
		ptrProcessElapsedTime = &processCpuTimeAndElapsedTime[1];

		// Fermer le handle du processus
		close_handle(processHandle);
	} else {
		// Gérer l'échec de l'ouverture du handle du processus
		DWORD lastErrorCode = GetLastError();
		if (lastErrorCode != ERROR_INVALID_PARAMETER && lastErrorCode != ERROR_ACCESS_DENIED) {
			fprintf(stderr, "Erreur lors de l'appel à OpenProcess() pour le processus %lu. Code d'erreur : %ld \n", ptrProcessEntry->th32ProcessID, lastErrorCode);
			fprintf(stderr, "Certaines informations relatives à ce processus ne seront pas affichées. \n");
		}
	}

	// Afficher les informations sur le processus
	display_process_entry(
		ptrProcessEntry,
		processHandleCount,
		processPrivateVirtualMemoryUsage,
		ptrProcessCpuTime,
		ptrProcessElapsedTime);
}

void list_processes() {
	// Créer un instantané des processus
	HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// Vérifier si la fonction a réussi
	if (snapshotHandle == INVALID_HANDLE_VALUE) {
		catch_function_error("CreateToolhelp32Snapshot()");
		return;
	}

	// Initialiser la structure PROCESSENTRY32
	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32);

	// Récupérer le premier processus dans l'instantané
	if (!Process32First(snapshotHandle, &processEntry)) {
		DWORD lastErrorCode = GetLastError();
		if (lastErrorCode != ERROR_NO_MORE_FILES) {
			catch_function_error("Process32First()");
		}
		fprintf(stderr, "Aucun processus détecté !\n");
		close_handle(snapshotHandle);
		return;
	}

	// Parcourir tous les processus dans l'instantané
	do {
		// Obtenir les informations sur le processus et les afficher
		get_process_info(&processEntry);
	} while (Process32Next(snapshotHandle, &processEntry));

	// Fermer le handle de l'instantané
	close_handle(snapshotHandle);
}
