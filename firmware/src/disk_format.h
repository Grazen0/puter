#ifndef FIRMWARE_DISK_FORMAT_H
#define FIRMWARE_DISK_FORMAT_H

#include "numeric.h"
#include "sd_card.h"

constexpr size_t MBR_CHS_SIZE = 3;

typedef struct {
    u8 boot;
    u8 start_chs[MBR_CHS_SIZE];
    u8 part_type;
    u8 end_chs[MBR_CHS_SIZE];
    u32 start;
    u32 size;
} MbrEntry;

static_assert(sizeof(MbrEntry) == 16);

typedef enum : u8 {
    MbrPartType_Empty = 0x00,
    MbrPartType_Fat32_Lba = 0x0C,
} MbrPartType;

constexpr size_t MBR_CODE_BASE = 0x000;
constexpr size_t MBR_CODE_SIZE = 0x1BE;

constexpr size_t MBR_ENTRIES_BASE = MBR_CODE_BASE + MBR_CODE_SIZE;
constexpr size_t MBR_ENTRIES = 4;
constexpr size_t MBR_ENTRY_SIZE = sizeof(MbrEntry);
constexpr size_t MBR_ENTRIES_SIZE = MBR_ENTRY_SIZE * MBR_ENTRIES;

constexpr size_t MBR_MAGIC_BASE = MBR_ENTRIES_BASE + MBR_ENTRIES_SIZE;
constexpr size_t MBR_MAGIC_SIZE = 0x002;

constexpr size_t MBR_SIZE = MBR_MAGIC_BASE + MBR_MAGIC_SIZE;
static_assert(MBR_SIZE == SD_BLOCK_SIZE);

constexpr u16 MBR_MAGIC = 0xAA55;

typedef struct {
    u16 bytes_per_sec;
    u8 secs_per_clus;
    u16 reserved_secs;
    u8 num_fats;
    u32 total_secs_32;
    u32 secs_per_fat_32;
    u32 root_clus;
} ExtendedBpb;

typedef struct {
    ExtendedBpb bpb;
    u32 eoc_marker;
    u32 first_sec;
    u32 first_fat_sec;
    u32 first_data_sec;
} Fat32;

typedef enum : u8 {
    FATTR_READ_ONLY = 1 << 0,
    FATTR_HIDDEN = 1 << 1,
    FATTR_SYSTEM = 1 << 2,
    FATTR_VOLUME_ID = 1 << 3,
    FATTR_DIRECTORY = 1 << 4,
    FATTR_ARCHIVE = 1 << 5,
    FATTR_LONG_NAME = 0x0F,
} FileAttribute;

constexpr size_t FILENAME_SIZE = 11;
constexpr size_t FILENAME_DELETED = 0xE5;

typedef struct {
    char filename[FILENAME_SIZE];
    u8 attrs;
    u8 _padding;
    u8 created_ds;
    u8 created_hms;
    u16 created_ymd;
    u16 last_access_ymd;
    u16 clus_hi;
    u16 modified_hms;
    u16 modified_ymd;
    u16 clus_lo;
    u32 size;
} DirEntry;

typedef struct {
    u32 cur_clus;
    u8 cur_clus_block;
    size_t block_idx;
} FatWalker;

typedef struct {
    u8 data[SD_BLOCK_SIZE];
    u32 block_addr;
} BlockBuffer;

[[nodiscard]] static inline bool
dirent_is_item(const DirEntry dirent[static const 1])
{
    return dirent->attrs != FATTR_LONG_NAME &&
           dirent->filename[0] != FILENAME_DELETED;
}

[[nodiscard]] static inline u32
dirent_first_clus(const DirEntry dirent[static const 1])
{
    return concat_u32(dirent->clus_hi, dirent->clus_lo);
}

void diskfmt_init();

[[nodiscard]] bool mbr_parse_first_entry(MbrEntry out_entry[static 1]);

bool fat32_parse(u32 first_sec, Fat32 out_fat[static 1]);

void fatwlk_init(FatWalker walker[static 1], u32 init_clus);

bool fatwlk_read(FatWalker walker[static 1], const Fat32 fat[static 1],
                 size_t n, void *dest);

bool fatwlk_read_v(FatWalker walker[static 1], const Fat32 fat[static 1],
                   size_t n, volatile void *dest);

#endif
