/* cmos.h the cmos header for Realix.
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#ifndef __realix_cmos__
#define __realix_cmos__

#define CMOS_PORT_INDEX 0x70
#define CMOS_PORT_DATA  0x71
#define CMOS_FLAG_INDEX 0x31
#define CMOS_FLAG_CHECK 0x32
#define CMOS_FLAG_LOAD_RMKERNEL 0xAA
#define CMOS_FLAG_CHECK_MAGIC 0x55

void cmos_set_lrmk(void); // ставить строго перед перезагрузкой!!!
void cmos_unset_flagz(void); 

#endif /* __realix_cmos__ */