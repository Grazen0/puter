#include "disk_format.h"
#include "numeric.h"
#include "sd_card.h"
#include "vga.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static bool memeq(const void *const s1, const void *const s2, size_t n)
{
    const u8 *s1_b = s1;
    const u8 *s2_b = s2;

    while (n--) {
        if (*s1_b++ != *s2_b++)
            return false;
    }

    return true;
}

static bool find_kernel_img(FatWalker walker[static const 1],
                            const Fat32 fat[static const 1],
                            DirEntry out_dirent[static const 1])
{
    constexpr char FILENAME[] = "KERNEL  BIN"; // 8.3 filename format

    fatwlk_init(walker, fat->bpb.root_clus);

    while (true) {
        if (!fatwlk_read(walker, fat, sizeof(*out_dirent), out_dirent))
            return false;

        if (out_dirent->filename[0] == '\0')
            return false;

        if (dirent_is_item(out_dirent) &&
            memeq(out_dirent->filename, FILENAME, sizeof(FILENAME) - 1))
            return true;
    }

    unreachable();
}

void main()
{
    vga_init();

    const SdInitResult sd_result = sd_init();

    if (sd_result != SdInitResult_Ok) {
        if (sd_result == SdInitResult_ErrGoIdleTimedOut) {
            vga_print("No SD card detected.\n");
        } else {
            vga_print("SD card init sequence failed: ");
            vga_print(sd_init_result_str(sd_result));
            vga_putchar('\n');
        }
        return;
    }

    diskfmt_init();

    MbrEntry mbr_entry;
    if (!mbr_parse_first_entry(&mbr_entry)) {
        vga_print("SD card does not contain a valid MBR.\n");
        return;
    }

    if (mbr_entry.part_type == MbrPartType_Empty) {
        vga_print("SD card is empty.\n");
        return;
    }

    if (mbr_entry.part_type != MbrPartType_Fat32_Lba) {
        vga_print("First partition is not FAT32.\n");
        return;
    }

    Fat32 fat;

    if (!fat32_parse(mbr_entry.start, &fat)) {
        vga_print("First partition is not valid FAT32.\n");
        return;
    }

    FatWalker walker;
    DirEntry kernel_dirent;

    if (!find_kernel_img(&walker, &fat, &kernel_dirent)) {
        vga_print("kernel.bin not found.\n");
        return;
    }

    extern u8 _kernel_start;

    fatwlk_init(&walker, dirent_first_clus(&kernel_dirent));

    if (!fatwlk_read(&walker, &fat, kernel_dirent.size, &_kernel_start)) {
        vga_print("Reached end of kernel.bin unexpectedly.\n");
        return;
    }

    auto const kmain = (void (*)())(uintptr_t)&_kernel_start;
    kmain();

    unreachable();
}
