#include "cli.h"
#include "vmc.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_help(argv[0]);
    return 1;
  }

  // Argumen pertama bisa opsi (--help / --version) atau path file
  if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
    print_help(argv[0]);
    return 0;
  }

  if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
    print_version();
    return 0;
  }

  const char *vmc_file = argv[1];
  FILE *fp = fopen(vmc_file, "rb");
  if (!fp) {
    fprintf(stderr, "Error: Cannot open file %s\n", vmc_file);
    return 1;
  }

  // Get file size
  fseeko(fp, 0, SEEK_END);
  off_t file_size = ftello(fp);
  fseeko(fp, 0, SEEK_SET);

  printf("Successfully opened VMC file: %s\n", vmc_file);
  printf("File size: %lld bytes (%.2f MB)\n\n", (long long)file_size,
         file_size / (1024.0 * 1024.0));

  // Load superblock
  VmcSuperblock sb;
  if (vmc_load_superblock(fp, &sb) != 0) {
    fprintf(stderr, "Error: Failed to load superblock\n");
    fclose(fp);
    return 1;
  }

  // Load FAT
  FatTable ft = vmc_load_fat(fp, &sb);
  if (!ft.fat) {
    fprintf(stderr, "Error: Failed to load FAT\n");
    fclose(fp);
    return 1;
  }

  // Print VMC information
  vmc_print_info(&sb, &ft);

  // List root directory
  vmc_list_root(fp, &sb, &ft);

  // Cleanup
  vmc_free_fat(&ft);
  fclose(fp);

  return 0;
}
