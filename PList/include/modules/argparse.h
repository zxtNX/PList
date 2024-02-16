#ifndef ARGPARSE_H
#define ARGPARSE_H

typedef struct {
    int verbose;
} Args;

Args parse_args(const int argc, const char *argv[]);
void show_help();

#endif
