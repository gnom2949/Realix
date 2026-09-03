// © Realix > Kernel32 > Multiboot 1
// =================================
// Раскладка полей соответствует bootloader/thirdix/Include/multiboot.h
// mem_lower/upper - грубая оценка, тут mmap будет поточнее.
// framebuffer_ поля читаемы только когда выставлен флаг MULTIBOOT_INFO_FRAMEBUFFER_INFO.

use crate::memory::pmm;
use crate::config::E820_MAX_ENTRIES;

pub const MULTIBOOT_BL_MAGIC: u32 = 0x2BADB002;

pub const MULTIBOOT_INFO_MEMORY: u32 = 0x00000001;
pub const MULTIBOOT_INFO_BOOTDEV: u32 = 0x00000002;
pub const MULTIBOOT_INFO_MEM_MAP: u32 = 0x00000040;
pub const MULTIBOOT_INFO_FRAMEBUFFER_INFO: u32 = 0x00001000;

/// multiboot_info_t. Union-ы(symtab/elf_sec, framebuffer_palette/color-mask)
/// представлены как сырые байты, не используются напрямую, только резервируют место под раскладку.
#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootInfo {
    pub flags: u32,
    pub mem_lower: u32,
    pub mem_upper: u32,
    pub boot_device: u32,
    pub cmdline: u32,
    pub mods_count: u32,
    pub mods_addr: u32,
    pub syms: [u32; 4], // union multiboot_aout_symbol_table_t | multiboot_elf_section_header_table_t.
    pub mmap_length: u32,
    pub mmap_addr: u32,
    pub drives_length: u32,
    pub drives_addr: u32,
    pub config_table: u32,
    pub apm_table: u32,
    pub vbe_control_info: u32,
    pub vbe_mode_info: u32,
    pub vbe_mode: u16,
    pub vbe_interface_seg: u16,
    pub vbe_interface_off: u16,
    pub vbe_interface_len: u16,
    pub framebuffer_addr: u64,
    pub framebuffer_pitch: u32,
    pub framebuffer_width: u32,
    pub framebuffer_height: u32,
    pub framebuffer_bpp: u8,
    pub framebuffer_type: u8,
    pub color_info: [u8; 6], // union framebuffer_palette_ | framebuffer_{red,green,blue}_
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
struct MultibootMmapEntry {
    size: u32,
    addr: u64,
    len:  u64,
    entry_type: u32,
}

impl MultibootInfo {
    pub unsafe fn read_mmap(&self, out: &mut [pmm::E820Entry; E820_MAX_ENTRIES]) -> usize
    {
        if self.flags & MULTIBOOT_INFO_MEM_MAP == 0 { return 0; }

        let mmap_len = self.mmap_length;
        let mmap_addr = self.mmap_addr;

        let mut offset: u32 = 0;
        let mut count:  usize = 0;

        while offset < mmap_len && count < E820_MAX_ENTRIES
        {
            let entry_ptr = (mmap_addr + offset) as *const MultibootMmapEntry;
            let entry = unsafe { core::ptr::read_unaligned(entry_ptr) };

            out[count] = pmm::E820Entry {
                address:  entry.addr,
                size:     entry.len,
                seg_type: entry.entry_type,
                attributes: 0,
            };
            count += 1;

            offset += entry.size + 4;
        }

        count
    }
}