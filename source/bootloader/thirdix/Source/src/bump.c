/* bump.c the bump allocator for Realix.
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#include "../../Include/bump.h"
#include "../../Include/stdint.h"

static uint8_t *arena_base = NULL;
static uint8_t *arena_end = NULL;
static uint8_t *bump_ptr = NULL;

static uintptr_t align_up(uintptr_t value, size_t align)
{
    return (value + (align -1)) & ~(uintptr_t)(align - 1);
}

void binit(void *base, size_t size)
{
    arena_base = (uint8_t*)base;
    arena_end = arena_base + size;
    bump_ptr = arena_base;
}

void *ballocaligned(size_t size, size_t align)
{
    if (arena_base == NULL)
        return NULL;

    uintptr_t addr = align_up((uintptr_t)bump_ptr, align);
    uint8_t *result = (uint8_t*)addr;

    if (result + size > arena_end || result < bump_ptr)
        return NULL;

    bump_ptr = result + size;
    return result;
}

void *bump_alloc(size_t size)
{
    return ballocaligned(size, sizeof(void*));
}

void bump_reset(void)
{
    bump_ptr = arena_base;
}

size_t bump_used(void)
{
    return (size_t)(bump_ptr - arena_base);
}

size_t bump_remaining(void)
{
    return (size_t)(arena_end - bump_ptr);
}