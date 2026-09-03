/* ui.h the UI for Realix Thirdix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#ifndef __realix_ui__
#define __realix_ui__

#include "pcinfo.h"

typedef enum {
    UI_CHOICE_16BIT,
    UI_CHOICE_32BIT
} ui_choice_t;

ui_choice_t ui_run(pcinfo_t *pcinfo);

#endif 