#include "vmc.h"

// External reference to titles database
extern const TitleEntry ps2_titles[];
extern const unsigned int ps2_titles_count;

// Load and validate superblock
int vmc_load_superblock(FILE *fp, VmcSuperblock *sb) {
  if (!read_exact(fp, sb, sizeof(*sb)))
    return -1;
  if (strncmp(sb->magic, "Sony PS2 Memory Card Format ", 28) != 0)
    return -2;
  return 0;
}

// Load FAT table
FatTable vmc_load_fat(FILE *fp, const VmcSuperblock *sb) {
  size_t entries_per_cluster = sb->cluster_size / sizeof(uint32_t);
  uint32_t *fat_cluster_ptrs =
      (uint32_t *)calloc(32 * entries_per_cluster, sizeof(uint32_t));
  size_t fat_ptr_count = 0;

  for (int i = 0; i < 32; i++) {
    uint32_t ifc = sb->ifc_ptr_list[i];
    if (ifc == 0 || ifc == INVALID_CLUSTER_PTR)
      break;
    uint32_t *tmp = (uint32_t *)malloc(sb->cluster_size);
    seek_or_die(fp, (uint64_t)ifc * sb->cluster_size);
    fread(tmp, sb->cluster_size, 1, fp);
    for (size_t j = 0; j < entries_per_cluster; j++) {
      if (tmp[j] == INVALID_CLUSTER_PTR)
        break;
      fat_cluster_ptrs[fat_ptr_count++] = tmp[j];
    }
    free(tmp);
  }

  size_t fat_entries_per_cluster = sb->cluster_size / sizeof(uint32_t);
  size_t fat_entries_total = fat_ptr_count * fat_entries_per_cluster;
  uint32_t *fat = malloc(fat_entries_total * sizeof(uint32_t));
  size_t written = 0;

  for (size_t i = 0; i < fat_ptr_count; i++) {
    seek_or_die(fp, (uint64_t)fat_cluster_ptrs[i] * sb->cluster_size);
    fread(&fat[written], sizeof(uint32_t), fat_entries_per_cluster, fp);
    written += fat_entries_per_cluster;
  }

  free(fat_cluster_ptrs);
  FatTable ft = {fat, written};
  return ft;
}

// Free FAT table memory
void vmc_free_fat(FatTable *ft) {
  free(ft->fat);
  ft->fat = NULL;
  ft->count = 0;
}

// Build cluster chain from FAT
ClusterChain vmc_build_chain(const FatTable *ft, uint32_t start,
                             uint32_t limit_max) {
  uint32_t *arr = malloc(ft->count * sizeof(uint32_t));
  size_t n = 0, cap = limit_max ? limit_max : ft->count;
  uint32_t c = start;

  while (c != INVALID_CLUSTER_PTR && n < cap) {
    arr[n++] = c;
    if (c >= ft->count)
      break;
    uint32_t raw = ft->fat[c];
    if (fat_flag(raw) == 0xFF)
      break;
    c = fat_next(raw);
  }

  ClusterChain ch = {arr, n};
  return ch;
}

// Free cluster chain memory
void vmc_free_chain(ClusterChain *ch) {
  free(ch->arr);
  ch->arr = NULL;
  ch->count = 0;
}

// Count free clusters in FAT - Fixed calculation
size_t vmc_count_free_clusters(const FatTable *ft) {
  size_t free_count = 0;
  for (size_t i = 0; i < ft->count; i++) {
    uint32_t raw_entry = ft->fat[i];
    uint8_t flag = fat_flag(raw_entry);
    uint32_t cluster = fat_next(raw_entry);

    // Check if cluster is free (flag == 0x7F and cluster == 0xFFFFFF indicates
    // free)
    if (flag == 0x7F && cluster == 0xFFFFFF) {
      free_count++;
    }
  }
  return free_count;
}

char *normalize_game_id(char *game_id) {
  // Ubah jadi huruf besar semua
  for (char *p = game_id; *p; p++) {
    *p = toupper((unsigned char)*p);
  }

  // Cek tabel mapping
  for (int i = 0; prefix_map[i].old_prefix != NULL; i++) {
    size_t old_len = strlen(prefix_map[i].old_prefix);
    if (strncmp(game_id, prefix_map[i].old_prefix, old_len) == 0) {
      // geser sisa string ke kanan setelah 4 huruf
      memmove(game_id + 4, game_id + old_len, strlen(game_id + old_len) + 1);
      memcpy(game_id, prefix_map[i].new_prefix, 4);
      break;
    }
  }

  return game_id;
}

ExtractedID extract_game_id_from_save(const char *save_name) {
  static ExtractedID result;
  char tmp[64];
  strncpy(tmp, save_name, sizeof(tmp) - 1);
  tmp[sizeof(tmp) - 1] = '\0';

  // default kosong
  result.id[0] = '\0';
  result.suffix[0] = '\0';

  // normalisasi id
  normalize_game_id(tmp);

  // daftar suffix
  const char *suffixes[] = {
      "2014OPT",  "2014000", "SAVEDATA", "GAMEDATA", "DAT0", "DAT1", "DAT2",
      "BEMU5YYY", "TCNYC",   "000",      "001",      "002",  "003",  "004",
      "005",      "006",     "007",      "008",      "009",  "DATA", "SAVE",
      "SYS",      "SYSTEM",  "CONFIG",   "OPT",      NULL};

  size_t len = strlen(tmp);
  for (int i = 0; suffixes[i] != NULL; i++) {
    size_t slen = strlen(suffixes[i]);
    if (len > slen && strcmp(tmp + len - slen, suffixes[i]) == 0) {
      strcpy(result.suffix, suffixes[i]); // simpan suffix
      tmp[len - slen] = '\0';             // hapus dari id
      break;
    }
  }

  strncpy(result.id, tmp, sizeof(result.id) - 1);
  result.id[sizeof(result.id) - 1] = '\0';
  return result;
}

// char *extract_game_id_from_save(const char *save_name) {
//   static char game_id[32];
//   strncpy(game_id, save_name, sizeof(game_id) - 1);
//   game_id[sizeof(game_id) - 1] = '\0';
//
//   normalize_game_id(game_id);
//
//   // Hapus suffix umum
//   const char *suffixes[] = {
//       "2014OPT",  "2014000", "SAVEDATA", "GAMEDATA", "DAT0", "DAT1", "DAT2",
//       "BEMU5YYY", "TCNYC",   "000",      "001",      "002",  "003",  "004",
//       "005",      "006",     "007",      "008",      "009",  "DATA", "SAVE",
//       "SYS",      "SYSTEM",  "CONFIG",   "OPT",      NULL};
//
//   size_t len = strlen(game_id);
//   for (int i = 0; suffixes[i] != NULL; i++) {
//     size_t suffix_len = strlen(suffixes[i]);
//     if (len > suffix_len &&
//         strcmp(game_id + len - suffix_len, suffixes[i]) == 0) {
//       game_id[len - suffix_len] = '\0';
//       break;
//     }
//   }
//
//   return game_id;
// }

const char *lookup_game_title(const char *save_id) {
  static char title_buf[128]; // buffer judul final

  ExtractedID ex = extract_game_id_from_save(save_id);
  char *game_id = ex.id;

  if (!save_id)
    return NULL;

  const char *base_title = NULL;

  // Cari di database
  for (unsigned int i = 0; i < ps2_titles_count; i++) {
    if (strcmp(ps2_titles[i].id, game_id) == 0) {
      base_title = ps2_titles[i].title;
      break;
    }
    size_t game_id_len = strlen(game_id);
    size_t db_id_len = strlen(ps2_titles[i].id);
    if ((game_id_len >= db_id_len &&
         strncmp(ps2_titles[i].id, game_id, db_id_len) == 0) ||
        (db_id_len >= game_id_len &&
         strncmp(game_id, ps2_titles[i].id, game_id_len) == 0)) {
      base_title = ps2_titles[i].title;
      break;
    }
  }

  if (!base_title)
    return NULL;

  // daftar suffix generik → jangan ditampilkan
  const char *generic_suffixes[] = {
      "OPT", "CONFIG", "SYSTEM", "SAVE", "SAVEDATA", "GAMEDATA", "DATA",
      "SYS", "000",    "001",    "002",  "003",      "004",      "005",
      "006", "007",    "008",    "009",  NULL};

  int show_suffix = 1;
  if (ex.suffix[0] != '\0') {
    for (int i = 0; generic_suffixes[i] != NULL; i++) {
      if (strcmp(ex.suffix, generic_suffixes[i]) == 0) {
        show_suffix = 0;
        break;
      }
    }
  } else {
    show_suffix = 0;
  }

  if (show_suffix) {
    snprintf(title_buf, sizeof(title_buf), "%s (%s)", base_title, ex.suffix);
    return title_buf;
  } else {
    return base_title;
  }
}

// Look up game title from internal database
// const char *lookup_game_title(const char *save_id) {
//   char *game_id = extract_game_id_from_save(save_id);
//
//   // fprintf(stderr, "[DEBUG] No match for save_id='%s' (extracted='%s')\n",
//   //         save_id, game_id);
//   if (!save_id)
//     return NULL;
//
//   // Direct match first
//   for (unsigned int i = 0; i < ps2_titles_count; i++) {
//     if (strcmp(ps2_titles[i].id, game_id) == 0) {
//       return ps2_titles[i].title;
//     }
//   }
//
//   // Partial match (for variations like BESLES-55673 matching
//   BESLES-556732014) size_t game_id_len = strlen(game_id); for (unsigned int i
//   = 0; i < ps2_titles_count; i++) {
//     // Check if the database ID is a prefix of our game ID
//     size_t db_id_len = strlen(ps2_titles[i].id);
//     if (game_id_len >= db_id_len &&
//         strncmp(ps2_titles[i].id, game_id, db_id_len) == 0) {
//       return ps2_titles[i].title;
//     }
//     // Check if our game ID is a prefix of the database ID
//     if (db_id_len >= game_id_len &&
//         strncmp(game_id, ps2_titles[i].id, game_id_len) == 0) {
//       return ps2_titles[i].title;
//     }
//   }
//
//   return NULL;
// }
