#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "processes_list.h"

int main(const int argc, char *argv[]) {
    printf("Recherche des processus en cours...\n");

    int showMemoryUsage = 0;

    if (argc == 1) {
        ListProcesses(showMemoryUsage);
        getchar();
    } else {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-m") == 0) {
                showMemoryUsage = 1;
            } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
                const DWORD pid = atoi(argv[++i]);
                ListDetailedProcessInfo(pid);
            } else {
                printf("Argument inconnu: %s\n", argv[i]);
            }
        }
        ListProcesses(showMemoryUsage);
    }
    return 0;
}
