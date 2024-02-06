#include <stdio.h>
#include <string.h>
#include "processes_list.h"

int main(const int argc, char *argv[]) {
    printf("Recherche des processus en cours...\n");

    if (argc == 1) {
        // No arguments passed, default to listing processes
        ListProcesses();
        getchar();
    } else {
        // Process the arguments
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--list") == 0 || strcmp(argv[i], "-l") == 0) {
                ListProcesses();
                getchar();
            } else {
                printf("Unknown argument: %s\n", argv[i]);
            }
        }
    }
    return 0;
}
