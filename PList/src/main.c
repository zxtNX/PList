#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Lmcons.h>

#include "utils/utils.h"
#include "modules/argparse.h"

static void initialize();
static void show_current_user();

static void initialize() {
	set_SeDebugPrivilege();
}

static void show_current_user() {
	char username[UNLEN + 1];
	DWORD size = UNLEN + 1;
	if (GetUserName(username, &size)) {
		printf("L'utilisateur qui a exécuté la commande est : %s\n", username);
	} else {
		fprintf(stderr, "Impossible de récupérer le nom de l'utilisateur : %lu\n", GetLastError());
	}
}

int main(const int argc, const char *argv[]) {
	initialize();
	show_current_user();
	parse_args(argc, argv);
	return 0;
}
