#include "vmc.h"

// Print date/time
void print_datetime(const FSDateTime *dt) {
  printf("%04d/%02d/%02d-%02d:%02d:%02d", dt->year, dt->month, dt->day,
         dt->hour, dt->min, dt->sec);
}

// Print VMC information with corrected free space calculation
void vmc_print_info(const VmcSuperblock *sb, const FatTable *ft) {
  printf("=== VMC Information ===\n");
  printf("Magic: %.28s\n", sb->magic);
  printf("Version: %.12s\n", sb->version);
  printf("Page size: %d bytes\n", sb->pagesize);
  printf("Cluster size: %u bytes\n", sb->cluster_size);
  printf("Pages per cluster: %u\n", sb->pages_per_cluster);
  printf("Clusters per card: %u\n", sb->clusters_per_card);
  printf("Allocation offset: %u\n", sb->alloc_offset);
  printf("Root directory cluster: %u\n", sb->rootdir_cluster);
  printf("Max allocatable clusters: %u\n", sb->max_allocatable_clusters);
  printf("Card type: %u\n", sb->cardtype);
  printf("Card flags: 0x%02X\n", sb->cardflags);

  // Corrected free space calculation
  size_t free_clusters = vmc_count_free_clusters(ft);
  size_t used_clusters = sb->max_allocatable_clusters - free_clusters;

  // Use actual allocatable space, not total card space
  double used_mb = (used_clusters * sb->cluster_size) / (1024.0 * 1024.0);
  double free_mb = (free_clusters * sb->cluster_size) / (1024.0 * 1024.0);
  double total_mb =
      (sb->max_allocatable_clusters * sb->cluster_size) / (1024.0 * 1024.0);

  printf("Used clusters: %zu (%.2f MB)\n", used_clusters, used_mb);
  printf("Free clusters: %zu (%.2f MB)\n", free_clusters, free_mb);
  printf("Total allocatable space: %.2f MB\n", total_mb);
  printf("========================\n\n");
}

// List root directory contents
void vmc_list_root(FILE *fp, const VmcSuperblock *sb, const FatTable *ft) {
  // Read rootdir first entry to get expected length
  FSEntry root_hdr;
  seek_or_die(fp, (uint64_t)(sb->alloc_offset + sb->rootdir_cluster) *
                      sb->cluster_size);
  fread(&root_hdr, sizeof(root_hdr), 1, fp);
  uint32_t expected_len = root_hdr.length;

  ClusterChain ch = vmc_build_chain(ft, sb->rootdir_cluster, 0);
  FSEntry *buf = malloc(sb->cluster_size);
  uint32_t entries_per_cluster = sb->cluster_size / sizeof(FSEntry);

  printf("=== Root Directory ===\n");
  printf("Expected entries: %u\n", expected_len);
  printf("%-32s %-4s %10s %-16s %-16s %s\n", "Save Name", "Type", "Size",
         "Created", "Modified", "Game Title");
  printf("%-32s %-4s %10s %-16s %-16s %s\n", "---------", "----", "----",
         "-------", "--------", "----------");

  uint32_t read_count = 0;

  // simpan daftar game unik
  char unique_ids[256][32]; // max 256 game
  int unique_count = 0;

  for (size_t i = 0; i < ch.count; i++) {
    seek_or_die(fp,
                (uint64_t)(sb->alloc_offset + ch.arr[i]) * sb->cluster_size);
    fread(buf, sb->cluster_size, 1, fp);

    for (uint32_t j = 0; j < entries_per_cluster; j++) {
      if (read_count++ >= expected_len)
        goto done;
      if (!(buf[j].mode & EM_EXISTS))
        continue;

      char name[33];
      memcpy(name, buf[j].name, 32);
      name[32] = 0;

      // Skip . and .. entries
      if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0)))
        continue;

      ExtractedID ex = extract_game_id_from_save(name);
      const char *game_title = lookup_game_title(name);

      // cek apakah game_id sudah ada di unique_ids
      int already_seen = 0;
      for (int u = 0; u < unique_count; u++) {
        if (strcmp(unique_ids[u], ex.id) == 0) {
          already_seen = 1;
          break;
        }
      }
      if (!already_seen && unique_count < 256) {
        snprintf(unique_ids[unique_count], sizeof(unique_ids[0]), "%s", ex.id);
        unique_count++;
      }

      printf("%-32s %-4s %10u ", name,
             (buf[j].mode & EM_DIRECTORY) ? "DIR" : "FILE", buf[j].length);
      print_datetime(&buf[j].created);
      printf(" ");
      print_datetime(&buf[j].modified);
      printf(" %s\n", game_title ? game_title : "Unknown Game");
    }
  }

done:
  printf("\nTotal unique games found: %d\n", unique_count);
  free(buf);
  vmc_free_chain(&ch);
}
