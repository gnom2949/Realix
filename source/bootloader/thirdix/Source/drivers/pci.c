/* pci.c the PCI Driver for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#include "../../Include/pci.h"
#include "../../Include/io.h"
#include "../../Include/com.h"

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	uint32_t b = ((uint32_t)bus)    & 0xFF;  // 8 бит
    uint32_t s = ((uint32_t)slot)   & 0x1F;  // 5 бит (0-31)
    uint32_t f = ((uint32_t)func)   & 0x07;  // 3 бита (0-7)
    uint32_t o = ((uint32_t)offset) & 0xFC;  // 6 бит, выровненных по 4 байтам

    uint32_t address = (1            << 31) | 
                       (b            << 16) | 
                       (s            << 11) | 
                       (f            << 8)  | 
                       o;

    outl(PCI_CONFIG_ADDRESS, address);

	io_wait();
    
    return inl(PCI_CONFIG_DATA);
}

void pci_check_device(uint8_t bus, uint8_t device)
{
    uint32_t reg0 = pci_read_config(bus, device, 0, 0);
    uint16_t vendor_id = reg0 & 0xFFFF;
    if (vendor_id == 0xFFFF || vendor_id == 0x0000) return;

    uint32_t reg3 = pci_read_config(bus, device, 0, 0x0C);
    uint8_t header_type = (reg3 >> 16) & 0xFF;

    uint8_t total_functions = (header_type & 0x80) ? 8 : 1;

    for (uint8_t function = 0; function < total_functions; function++)
    {
        uint32_t current_reg0 = pci_read_config(bus, device, function, 0);
        uint16_t current_vendor = current_reg0 & 0xFFFF;
        uint16_t current_device = (current_reg0 >> 16) & 0xFFFF;

        if (current_vendor == 0xFFFF || current_vendor == 0x0000) continue;

        uint32_t reg2 = pci_read_config(bus, device, function, 0x08);
        uint8_t base_class = (reg2 >> 24) & 0xFF;
        uint8_t sub_class = (reg2 >> 16) & 0xFF;

        serial_print("FOUND DEVICE\n");

        if (base_class == 0x01 && sub_class == 0x01) {
            serial_print("FOUND ATA DISK\n");
        }

        if (base_class == 0x03) {
            serial_print("	[^] Found Videocard.\n");
        }
    }
}

void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t b = ((uint32_t)bus)    & 0xFF;
    uint32_t s = ((uint32_t)slot)   & 0x1F;
    uint32_t f = ((uint32_t)func)   & 0x07;
    uint32_t o = ((uint32_t)offset) & 0xFC;

    uint32_t address = (1            << 31) | 
                       (b            << 16) | 
                       (s            << 11) | 
                       (f            << 8)  | 
                       o;

    outl(PCI_CONFIG_ADDRESS, address);
    io_wait();
    
    outl(PCI_CONFIG_DATA, value);
    io_wait();
}

bool pci_find_vga_device(pci_device_t *out)
{
    for (uint16_t slot = 0; slot < 32; slot++)
    {
        uint32_t reg0 = pci_read_config(0, (uint8_t)slot, 0, 0x00);
        uint16_t venid = reg0 & 0xFFFF;
        if (venid == 0xFFFF || venid == 0x0000) continue;
        uint32_t reg3 = pci_read_config(0, (uint8_t)slot, 0, 0x0C);
        uint8_t header_type = (reg3 >> 16) & 0xFF;
        uint8_t total_functions = (header_type & 0x80) ? 8 : 1;
        
        for (uint8_t func = 0; func < total_functions; func++)
        {
            uint32_t current_reg0 = pci_read_config(0, (uint8_t)slot, func, 0x00);
            uint16_t current_vendor = current_reg0 & 0xFFFF;
            if (current_vendor == 0xFFFF || current_vendor == 0x0000) continue;

            uint32_t reg2 = pci_read_config(0, (uint8_t)slot, func, 0x08);
            uint8_t base_class = (reg2 >> 24) & 0xFF;
            uint8_t sub_class = (reg2 >> 16) & 0xFF;

            if (base_class == 0x03 && sub_class == 0x00)
            {
                uint32_t bar0 = pci_read_config(0, (uint8_t)slot, func, 0x10);
                if ((bar0 & 0x1) == 0) /* memory-mapped, не IO BAR */
                {
                    out->bus = 0;
                    out->slot = (uint8_t)slot;
                    out->func = func;
                    out->venid = current_vendor;
                    out->devid = (uint16_t)(current_reg0 >> 16);
                    out->bar0 = bar0 & 0xFFFFFFF0u;
                    return true;
                }
            }
        }
    }
    return false;
}