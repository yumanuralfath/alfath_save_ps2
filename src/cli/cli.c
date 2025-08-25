#include "cli.h"
#include <stdio.h>

#ifndef VERSION
#define VERSION "dev"
#endif

void print_help(const char *progname) {
  printf("Usage: %s [options] <file>\n", progname);
  printf("\nOptions:\n");
  printf("  --help      Show this help message\n");
  printf("  --version   Show program version\n");
}

void print_version(void) { printf("vmcreader version %s\n", VERSION); }
