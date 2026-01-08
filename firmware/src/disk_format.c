#include "disk_format.h"
#include "sd_card.h"
#include "vga.h"
#include <stddef.h>
#include <string.h>

static BlockBuffer scratch_bbuf;
static BlockBuffer fat_bbuf;

static inline void bbuf_init(BlockBuffer bbuf[static const 1])
{
    bbuf->block_addr = UINT32_MAX;
}

static void bbuf_load(BlockBuffer bbuf[static const 1], const u32 block_addr)
{
    if (bbuf->block_addr == block_addr)
        return;

    const SdReadResult sd_result = sd_read_block(block_addr, bbuf->data);

    if (sd_result != SdReadResult_Ok) {
        vga_print("Failed to read a block (code = ");
        vga_print_hex(sd_result);
        vga_print(")\n");
        while (true)
            ;
    }

    bbuf->block_addr = block_addr;
}

static inline u32 fat32_clus_first_sec(const Fat32 fat[static const 1],
                                       const size_t clus)
{
    return fat->first_data_sec + mul(fat->bpb.secs_per_clus, clus - 2);
}

[[nodiscard]] static inline bool
fat32_parse_bpb(const u8 bpb_data[], ExtendedBpb out_bpb[static const 1])
{
    enum : size_t {
        BPB_BYTES_PER_SEC = 0x00,
        BPB_SECS_PER_CLUS = 0x02,
        BPB_RESERVED_SECS = 0x03,
        BPB_NUM_FATS = 0x05,
        BPB_ROOT_ENTRIES = 0x06,
        BPB_TOTAL_SECS = 0x08,
        BPB_SECS_PER_FAT = 0x0B,
        BPB_TOTAL_SECS_32 = 0x15,
        BPB_SECS_PER_FAT_32 = 0x19,
        BPB_ROOT_CLUS = 0x21,
    };

    const u16 bytes_per_sec = get_u16_le(bpb_data, BPB_BYTES_PER_SEC);

    if (bytes_per_sec != SD_BLOCK_SIZE)
        return false;

    const u16 root_entries = get_u16_le(bpb_data, BPB_ROOT_ENTRIES);

    if (root_entries != 0)
        return false;

    const u16 secs_per_fat = get_u16_le(bpb_data, BPB_SECS_PER_FAT);

    if (secs_per_fat != 0)
        return false;

    const u16 total_secs = get_u16_le(bpb_data, BPB_TOTAL_SECS);

    if (total_secs != 0)
        return false;

    const u32 root_clus = get_u32_le(bpb_data, BPB_ROOT_CLUS);

    if (root_clus < 2)
        return false;

    *out_bpb = (ExtendedBpb){
        .bytes_per_sec = bytes_per_sec,
        .secs_per_clus = bpb_data[BPB_SECS_PER_CLUS],
        .reserved_secs = get_u16_le(bpb_data, BPB_RESERVED_SECS),
        .num_fats = bpb_data[BPB_NUM_FATS],
        .total_secs_32 = get_u32_le(bpb_data, BPB_TOTAL_SECS_32),
        .secs_per_fat_32 = get_u32_le(bpb_data, BPB_SECS_PER_FAT_32),
        .root_clus = root_clus,
    };

    return true;
}

static inline u32 fatwlk_cur_block(FatWalker walker[static const 1],
                                   const Fat32 fat[static const 1])
{
    return fat32_clus_first_sec(fat, walker->cur_clus) + walker->cur_clus_block;
}

void diskfmt_init()
{
    bbuf_init(&scratch_bbuf);
    bbuf_init(&fat_bbuf);
}

void *memcpy(void *const restrict dest, const void *const restrict src,
             size_t n)
{
    u8 *dest_b = dest;
    const u8 *src_b = src;

    while (n--)
        *dest_b++ = *src_b++;

    return dest;
}

[[nodiscard]] bool mbr_parse_first_entry(MbrEntry out_entry[static const 1])
{
    bbuf_load(&scratch_bbuf, 0);

    vga_putchar('\n');

    const u16 magic = get_u16_le(scratch_bbuf.data, MBR_MAGIC_BASE);

    if (magic != MBR_MAGIC)
        return false;

    memcpy(out_entry, &scratch_bbuf.data[MBR_ENTRIES_BASE], sizeof(*out_entry));
    return true;
}

bool fat32_parse(const u32 first_sec, Fat32 out_fat[static const 1])
{
    bbuf_load(&scratch_bbuf, first_sec);

    constexpr size_t FAT_BPB_BASE = 0xB;

    if (!fat32_parse_bpb(&scratch_bbuf.data[FAT_BPB_BASE], &out_fat->bpb))
        return false;

    out_fat->first_sec = first_sec;
    out_fat->first_fat_sec = first_sec + out_fat->bpb.reserved_secs;
    out_fat->first_data_sec =
        out_fat->first_fat_sec +
        mul(out_fat->bpb.num_fats, out_fat->bpb.secs_per_fat_32);

    bbuf_load(&fat_bbuf, out_fat->first_fat_sec);

    out_fat->eoc_marker = get_u32_le(fat_bbuf.data, 4);

    bbuf_load(&scratch_bbuf,
              fat32_clus_first_sec(out_fat, out_fat->bpb.root_clus));

    return true;
}

void fatwlk_init(FatWalker walker[static const 1], const u32 init_clus)
{
    *walker = (FatWalker){
        .cur_clus = init_clus,
        .cur_clus_block = 0,
        .block_idx = 0,
    };
}

bool fatwlk_read(FatWalker walker[static const 1],
                 const Fat32 fat[static const 1], size_t n, void *const dest)
{
    constexpr u32 CLUS_PER_FAT_BLOCK = SD_BLOCK_SIZE / 4;

    u8 *dest_b = dest;

    while (n > 0) {
        if (walker->cur_clus >= fat->eoc_marker)
            return false;

        bbuf_load(&scratch_bbuf, fatwlk_cur_block(walker, fat));

        if (walker->block_idx + n < SD_BLOCK_SIZE) {
            // Partial block read
            memcpy(dest_b, &scratch_bbuf.data[walker->block_idx], n);
            walker->block_idx += n;
            break;
        }

        // Read what is left of current block
        const size_t read_len = SD_BLOCK_SIZE - walker->block_idx;
        memcpy(dest_b, &scratch_bbuf.data[walker->block_idx], read_len);

        if (++walker->cur_clus_block >= fat->bpb.secs_per_clus) {
            bbuf_load(&fat_bbuf, fat->first_fat_sec +
                                     (walker->cur_clus / CLUS_PER_FAT_BLOCK));

            // Grab next cluster
            walker->cur_clus = get_u32_le(fat_bbuf.data, 4 * walker->cur_clus);
            walker->cur_clus_block = 0;
        }

        n -= read_len;
        dest_b += read_len;
        walker->block_idx = 0;
    }

    return true;
}
