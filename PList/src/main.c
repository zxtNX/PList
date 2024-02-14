#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <Lmcons.h>
#include "argparse.h"

int main(const int argc, char *argv[]) {
    char username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserName(username, &size)) {
        printf("Le programme run actuellement en tant qu'utilisateur : %s\n", username);
    } else {
        fprintf(stderr, "Impossible de récupérer le nom de l'utilisateur : %lu\n", GetLastError());
    }

    parse_args(argc, argv);

    return 0;
}
