/* pci.h the PCI Driver header for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#ifndef __realix_pci__
#define __realix_pci__

#include "stdint.h"
#include "stdbool.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
void pci_check_device(uint8_t bus, uint8_t device);

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t venid;
    uint16_t devid;
    uint32_t bar0;
} pci_device_t;

bool pci_find_vga_device(pci_device_t *out);
#endif /*__realix_pci_h__*/