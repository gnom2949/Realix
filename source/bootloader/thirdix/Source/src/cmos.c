/* cmos.c the cmos functions for Realix.
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#include "../../Include/cmos.h"
#include "../../Include/io.h"

static void cmos_write_byte(uint8_t index, uint8_t value)
{
    outb(CMOS_PORT_INDEX, index & 0x7F);
    outb(CMOS_PORT_INDEX, value);
}

void cmos_set_lrmk(void)
{
    cmos_write_byte(CMOS_FLAG_INDEX, CMOS_FLAG_LOAD_RMKERNEL);
    cmos_write_byte(CMOS_FLAG_CHECK, CMOS_FLAG_CHECK_MAGIC);
}

void cmos_unset_flagz(void)
{
    cmos_write_byte(CMOS_FLAG_INDEX, 0x00);
    cmos_write_byte(CMOS_FLAG_CHECK, 0x00);
}