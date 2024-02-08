#ifndef ARGPARSE_H
#define ARGPARSE_H

typedef struct {
    int verbose;
    char *input_file;
    char *output_file;
} Args;

Args parse_args(int argc, char *argv[]);
void print_usage();

#endif
