#include "argparse.h"

#include <processes_list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage() {
    printf(" PSList.exe [options]\n");
    printf("Options :\n");
    printf("  -h, --help\t\tAfficher l'aide\n");
    printf("  -v, --verbose\t\tActiver le mode verbeux (pas encore fonctionnel)\n");
    printf("  -m, --memory\t\tAffiche l'utilisation mémoire\n");
}

Args parse_args(const int argc, char *argv[]) {
    const Args args = {0};

    if (argc == 1) {
        ListProcesses();
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            // Afficher l'aide
            print_usage();
            exit(0);
        }

        if (strcmp(argv[i], "-d") == 0) {
            // Afficher le détail des threads
            if (i + 1 < argc) {
                // Récupérer le PID à partir de l'argument suivant
                const DWORD pid = atoi(argv[i + 1]);
                ListDetailedProcessInfo(pid);
                i++; // Passer à l'argument suivant
            } else {
                fprintf(stderr, "Erreur : L'option -d nécessite un PID.\n");
                print_usage();
                exit(1);
            }
        } else if (strcmp(argv[i], "-m") == 0) {
            ListProcessesMemory();
        } else {
            fprintf(stderr, "Erreur - Argument inconnu : %s\n", argv[i]);
            print_usage();
            exit(1);
        }
    }

    return args;
}
