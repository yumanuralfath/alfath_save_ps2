#ifndef VMC_H
#define VMC_H

#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define INVALID_CLUSTER_PTR 0xFFFFFFFF
#define NULL_CLUSTER_PTR 0

// VMC Superblock structure
typedef struct {
  char magic[28];                    // 0x00
  char version[12];                  // 0x1C
  int16_t pagesize;                  // 0x28
  uint16_t pages_per_cluster;        // 0x2A
  uint16_t pages_per_block;          // 0x2C
  uint16_t unused;                   // 0x2E
  uint32_t clusters_per_card;        // 0x30
  uint32_t alloc_offset;             // 0x34
  uint32_t alloc_end;                // 0x38
  uint32_t rootdir_cluster;          // 0x3C
  uint32_t backup_block1;            // 0x40
  uint32_t backup_block2;            // 0x44
  uint8_t unused2[8];                // 0x48
  uint32_t ifc_ptr_list[32];         // 0x50
  uint32_t bad_block_list[32];       // 0xD0
  uint8_t cardtype;                  // 0x150
  uint8_t cardflags;                 // 0x151
  uint16_t unused3;                  // 0x152
  uint32_t cluster_size;             // 0x154
  uint32_t fat_entries_per_cluster;  // 0x158
  uint32_t clusters_per_block;       // 0x15C
  int32_t cardform;                  // 0x160
  uint32_t rootdir_cluster2;         // 0x164
  uint32_t unknown1;                 // 0x168
  uint32_t unknown2;                 // 0x16C
  uint32_t max_allocatable_clusters; // 0x170
  uint32_t unknown3;                 // 0x174
  uint32_t unknown4;                 // 0x178
  int32_t unknown5;                  // 0x17C
} __attribute__((packed)) VmcSuperblock;

// File system date/time structure
typedef struct {
  uint8_t resv2;
  uint8_t sec;
  uint8_t min;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint16_t year;
} __attribute__((packed)) FSDateTime;

// File system entry structure
typedef struct {
  uint16_t mode;        // 0x00
  uint16_t unused;      // 0x02
  uint32_t length;      // 0x04
  FSDateTime created;   // 0x08
  uint32_t cluster;     // 0x10
  uint32_t dir_entry;   // 0x14
  FSDateTime modified;  // 0x18
  uint32_t attr;        // 0x20
  uint32_t unused2[7];  // 0x24
  char name[32];        // 0x40
  uint8_t unused3[416]; // 0x60
} __attribute__((packed)) FSEntry;

// File system entry modes
typedef enum {
  EM_READ = 0x1,
  EM_WRITE = 0x2,
  EM_EXECUTE = 0x4,
  EM_PROTECTED = 0x8,
  EM_FILE = 0x10,
  EM_DIRECTORY = 0x20,
  EM_POCKETSTATION = 0x800,
  EM_PLAYSTATION = 0x1000,
  EM_HIDDEN = 0x2000,
  EM_EXISTS = 0x8000
} FSEntryMode;

// FAT table structure
typedef struct {
  uint32_t *fat;
  size_t count;
} FatTable;

// Cluster chain structure
typedef struct {
  uint32_t *arr;
  size_t count;
} ClusterChain;

// Title database structure
typedef struct {
  const char *id;
  const char *title;
  const char *publisher;
  const char *genre;
  const char *language;
  const char *developer;
  const char *region;
  const char *release_date;
  const char *serial;
} TitleEntry;

typedef struct {
  const char *old_prefix;
  const char *new_prefix;
} PrefixMap;

typedef struct {
  char id[32];     // normalized game id (contoh "SLES-55673")
  char suffix[32]; // sisa yang dibuang (contoh "2014OPT")
} ExtractedID;

static const PrefixMap prefix_map[] = {
    {"BASLUS", "SLUS"},
    {"BESLUS", "SLUS"},
    {"BISLUS", "SLUS"},
    {"BASCUS", "SCUS"},
    {"BESCUS", "SCUS"},
    {"BISCUS", "SCUS"},
    {"BASLES", "SLES"},
    {"BESLES", "SLES"},
    {"BISLES", "SLES"},
    {"BASLPS", "SLPS"},
    {"BESLPS", "SLPS"},
    {"BISLPS", "SLPS"},
    {"BASLPM", "SLPM"},
    {"BESLPM", "SLPM"},
    {"BISLPM", "SLPM"},
    // bisa tambah lagi kalau ketemu prefix lain
    {NULL, NULL}};

// Function prototypes

// Core VMC functions
int vmc_load_superblock(FILE *fp, VmcSuperblock *sb);
FatTable vmc_load_fat(FILE *fp, const VmcSuperblock *sb);
void vmc_free_fat(FatTable *ft);
ClusterChain vmc_build_chain(const FatTable *ft, uint32_t start,
                             uint32_t limit_max);
void vmc_free_chain(ClusterChain *ch);
size_t vmc_count_free_clusters(const FatTable *ft);

// Display functions
void vmc_print_info(const VmcSuperblock *sb, const FatTable *ft);
void vmc_list_root(FILE *fp, const VmcSuperblock *sb, const FatTable *ft);
void print_datetime(const FSDateTime *dt);

// Title database functions
char *normalize_game_id(char *game_id);
const char *lookup_game_title(const char *save_id);
// char *extract_game_id_from_save(const char *save_name);
ExtractedID extract_game_id_from_save(const char *save_name);

// Helper functions
static inline uint8_t fat_flag(uint32_t e) {
  return (uint8_t)((e >> 24) & 0xFF);
}

static inline uint32_t fat_next(uint32_t e) { return (e & 0xFFFFFFu); }

static inline void seek_or_die(FILE *fp, uint64_t off) {
  if (fseeko(fp, (off_t)off, SEEK_SET) != 0) {
    perror("seek");
    exit(1);
  }
}

static inline int read_exact(FILE *fp, void *buf, size_t n) {
  return fread(buf, 1, n, fp) == n;
}

#endif // VMC_H
