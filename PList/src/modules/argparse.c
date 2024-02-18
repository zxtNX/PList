#include "modules/argparse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "modules/processes_list.h"
#include "modules/processes_memory.h"
#include "modules/processes_threads.h"
#include "utils/utils.h"

void show_help() {
    printf(" PList.exe [options]\n");
    printf("Le comportement de base, donc sans option, permet de lister tous les processus en cours\n");
    printf("Options :\n");
    printf("  -h, --help\t\tAfficher l'aide\n");
    printf("  -v, --verbose\t\tActiver le mode verbeux (pas encore fonctionnel)\n");
    printf("  -d PID, --details\t\tAfficher les détails des threads du processus en question\n");
    printf("  -m, --memory\t\tAffiche l'utilisation mémoire\n");
}

Args parse_args(const int argc, const char *argv[]) {
    const Args args = {0};

    if (argc == 1) {
        show_process_list_titles();
        list_processes();
        return args;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmpi(argv[i], "-h") == 0 || strcmpi(argv[i], "--help") == 0) {
            show_help();
            exit(EXIT_SUCCESS);
        }

        if (strcmpi(argv[i], "-d") == 0 || strcmpi(argv[i], "--details") == 0) {
            if (i + 1 < argc) {
                const DWORD pid = atoi(argv[i + 1]);
                show_thr_list_titles();
                list_thr_by_pid(pid);
                i++;
                continue;
            } else {
                fprintf(stderr, "Erreur : L'option -d a beosin d'un PID.\n");
                show_help();
                exit(EXIT_FAILURE);
            }
        }

        if (strcmpi(argv[i], "-m") == 0 || strcmpi(argv[i], "--memory") == 0) {
            list_processes_memory();
            continue;
        }

        fprintf(stderr, "Erreur - Argument inconnu : %s\n", argv[i]);
        show_help();
        exit(EXIT_FAILURE);
    }

    return args;
}
