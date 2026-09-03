/* thrfs.c the THRFS Implementation for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    THRFS or Thirdix Filesystem is POSIX ustar-based Filesystem for Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/

#include "../../Include/thrfs.h"
#include "../../Include/string.h"

#define USTAR_BLOCK_SIZE 512

typedef struct __attribute__((packed)) {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} ustar_header_t;

static const uint8_t *g_base = NULL;
static size_t g_size = 0;

void thrfs_new(const void *base, size_t size)
{
    g_base = (const uint8_t*)base;
    g_size = size;
}

static size_t parse_octal(const char *field, size_t len)
{
    size_t value = 0;
    for (size_t i = 0; i < len && field[i]; i++)
    {
        if (field[i] < '0' || field[i] > '7') break;
        value = (value << 3) + (size_t)(field[i] - '0');
    }
    return value;
}

const void *thrfs_find(const char *name, size_t *out_size)
{
    if (g_base == NULL) return NULL;

    size_t offset = 0;
    while (offset + USTAR_BLOCK_SIZE <= g_size)
    {
        const ustar_header_t *head = (const ustar_header_t*)(g_base + offset);

        if (head->name[0] == '\0') break;

        if (MemCompare(head->magic, "ustar", 5) != 0) break;

        size_t file_size = parse_octal(head->size, sizeof(head->size));

        if (StrCompare(head->name, name) == 0)
        {
            if (out_size) *out_size = file_size;
            return g_base + offset + USTAR_BLOCK_SIZE;
        }

        size_t data_blocks = (file_size + USTAR_BLOCK_SIZE - 1) / USTAR_BLOCK_SIZE;
        offset += USTAR_BLOCK_SIZE + data_blocks * USTAR_BLOCK_SIZE;
    }

    return NULL;
}