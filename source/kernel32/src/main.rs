// © Realix > Kernel32: Main
// (27.07.26) v0.1
// ================
// ❗️ Загружается Switcher по адресу KERNEL32_PHYS_ADDR, адрес PCINFO в ebx

#![no_std]
#![no_main]

// Объявление модулей
mod commands;
mod drivers;
mod memory;
mod shell;
mod utils;
mod x86;
mod config;
mod multiboot;

// Подключение функций
use core::arch::{asm, naked_asm};
use core::panic::PanicInfo;
use drivers::{keyboard, pit, vga, font};
use memory::{frame_allocator, pmm};
use crate::pmm::E820Entry;
use x86::{gdt, idt};
use crate::config::{PCINFO_ADDR, E820_MAX_ENTRIES};
use crate::multiboot::{MultibootInfo, MULTIBOOT_BL_MAGIC, MULTIBOOT_INFO_FRAMEBUFFER_INFO};
/// Структура PCINFO, формируемая загрузчиком
#[derive(Copy, Clone)]
#[repr(C, packed)]
pub struct PcInfo {
    pub memory_mb:    u32,
    pub low_memory_kb: u16,
    pub mmap_count: u16,
    pub boot_drive_num: u16,
    pub videomode: u16,
    pub memory_map: [pmm::E820Entry; E820_MAX_ENTRIES],
}

// Доступ-обёртка к элементам структуры
impl PcInfo {
    #[inline]
    fn mmap_entry(&self, i: usize) -> E820Entry {
        if i > E820_MAX_ENTRIES {
            return pmm::E820Entry {
                address: 0,
                size: 0,
                seg_type: 0,
                attributes: 0,
            };
        }
        unsafe { core::ptr::read_unaligned(core::ptr::addr_of!(self.memory_map[i as usize])) }
    }
}

/// Предоставление доступа-обёртки к структуре PcInfo
#[inline]
unsafe fn pcinfo() -> &'static PcInfo {
    &*(PCINFO_ADDR as *const PcInfo)
}

// Границы секции BSS (из linker.ld) для обнуления вручную
unsafe extern "C" {
    unsafe static __bss_start: u8;
    unsafe static __bss_end: u8;
}

/// Низкоуровневая точка входа Kernel32 (Настройка окружения)
/// Параметры:
///  - ebx: адрес PCINFO
///  - esp: стек от загрузчика
#[link_section = ".text.entry"]
#[no_mangle]
#[unsafe(naked)]
pub extern "C" fn _start() -> ! {
    naked_asm!(
        "cld",
        // eax(magic) нужно сохранить до обнуления BSS.
        "mov edx, eax",
        // Установка: edi - начало, ecx - размер
        "lea edi, [__bss_start]",
        "lea ecx, [__bss_end]",
        "sub ecx, edi",
        // Обнуляем BSS через eax
        "xor eax, eax",
        "rep stosb",
        // Аргументы kmain(magic, mb_info) по cdecl
        "push ebx",
        "push edx",
        "call kmain",
        // Защита на случай незапланированного возвращения из функции
        "2:",
        "cli",
        "hlt",
        "jmp 2b"
    );
}

/// Основной цикл работы ядра
/// Параметры:
///  - pcinfo_addr: адрес структуры PCINFO, собранной загрузчиком
///  - mb_info_addr: адрес структуры MultibootInfo, собранной и передаваемой загрузчиком
#[no_mangle]
extern "C" fn kmain(magic: u32, mb_info_addr: *const MultibootInfo) -> ! {
    // Инициализация модулей
    idt::interrupts_disable();
    gdt::init();
    idt::init();
    pit::init(100);
    pmm::init_kernel_page_allocator();
    idt::interrupts_enable();

    // проверка Magic - если не совпал то это не Multiboot-загрузчик или eax потерялся.
    if magic != MULTIBOOT_BL_MAGIC {
        vga::clear_screen();
        vga::print_line("[KERNEL PANIC] Invalid Multiboot magic.\n", vga::Color::Red);
        halt_loop();
    }

    // проверка указателя multiboot_info_t
    if mb_info_addr.is_null() {
        vga::clear_screen();
        vga::print_line("[KERNEL PANIC] Invalid Multiboot info address.\n", vga::Color::Red);
        halt_loop();
    }

    let mb_info: &MultibootInfo = unsafe { &*mb_info_addr };

    // сборка карты памяти из multiboot mmap.
    let mut mmap_buf = [pmm::E820Entry { address: 0, size: 0, seg_type: 0, attributes: 0}; E820_MAX_ENTRIES];
    let entry_count = unsafe { mb_info.read_mmap(&mut mmap_buf) };

    let has_fb = mb_info.flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO != 0;
    let fb_info = if has_fb {
        Some(vga::FrameBufferInfo { addr: mb_info.framebuffer_addr as u32, pitch: mb_info.framebuffer_pitch, width: mb_info.framebuffer_width, height: mb_info.framebuffer_height, bpp: mb_info.framebuffer_bpp })
    } else {
        None
    };

    unsafe {
        frame_allocator::init(&mmap_buf[..entry_count]);
        vga::init(if has_fb { 1 } else { 0 }, fb_info);

        if has_fb {
            vga::set_comfortaa_font();
            vga::fill_screen(vga::Color::LightCyan);

            vga::vbe_print_line("Bajoding", vga::Color::White);
        }
    }

    // Вывод логотипа и приглашения
    vga::clear_screen();
    draw_logo(4, 2);

    vga::print_line("   Press any key to continue...", vga::Color::LightGray);
    keyboard::read_key();

    // Вывод заголовка Shell с его бесконечной работой
    vga::clear_screen();
    vga::print_line(
        "Welcome to Realix (Protected Mode with Rust kernel)...\n",
        vga::Color::Cyan,
    );

    // ! Вывод заметки о экспериментальной функции NovaAI
    vga::print_line(
        "Integration with NovaAI (type 'nova -a' to chat)\n",
        vga::Color::LightCyan,
    );

    shell::run();
    halt_loop();
}

/// Отрисовка логотипа Realix
fn draw_logo(start_x: usize, start_y: usize) {
    // Массив из 2 уровней:
    // 1. Массивы для каждой буквы
    // 2. Массивы для каждой строки буквы (0 - пробел, 1 - блок)
    let letters: [[[u8; 6]; 8]; 6] = [
        [[1,1,1,1,0,0],[1,0,0,1,0,0],[1,0,0,1,0,0],[1,1,1,1,0,0],
         [1,1,0,0,0,0],[1,0,1,0,0,0],[1,0,0,1,0,0],[1,0,0,1,0,0]],
        [[1,1,1,1,1,0],[1,0,0,0,0,0],[1,0,0,0,0,0],[1,1,1,1,0,0],
         [1,0,0,0,0,0],[1,0,0,0,0,0],[1,0,0,0,0,0],[1,1,1,1,1,0]],
        [[0,1,1,1,0,0],[1,0,0,0,1,0],[1,0,0,0,1,0],[1,1,1,1,1,0],
         [1,0,0,0,1,0],[1,0,0,0,1,0],[1,0,0,0,1,0],[1,0,0,0,1,0]],
        [[1,0,0,0,0,0],[1,0,0,0,0,0],[1,0,0,0,0,0],[1,0,0,0,0,0],
         [1,0,0,0,0,0],[1,0,0,0,0,0],[1,0,0,0,0,0],[1,1,1,1,1,0]],
        [[0,1,1,1,0,0],[0,0,1,0,0,0],[0,0,1,0,0,0],[0,0,1,0,0,0],
         [0,0,1,0,0,0],[0,0,1,0,0,0],[0,0,1,0,0,0],[0,1,1,1,0,0]],
        [[1,0,0,0,1,0],[0,1,0,1,0,0],[0,0,1,0,0,0],[0,0,1,0,0,0],
         [0,0,1,0,0,0],[0,1,0,1,0,0],[1,0,0,0,1,0],[1,0,0,0,1,0]],
    ];

    // Список цветов для букв (1 буква - 1 цвет)
    let colors = [
        vga::Color::Red,
        vga::Color::Yellow,
        vga::Color::Green,
        vga::Color::Cyan,
        vga::Color::Blue,
        vga::Color::Magenta,
    ];

    for (letter_idx, letter) in letters.iter().enumerate() {
        let color = colors[letter_idx % colors.len()];
        let offset_x = start_x + letter_idx * letter[0].len();

        for (row, line) in letter.iter().enumerate() {
            for (col, &pixel) in line.iter().enumerate() {
                if pixel == 1 {
                    vga::write_char_at(start_y + row, offset_x + col, 0xDB, color);
                }
            }
        }
    }

    for _ in 0..11 {
        vga::print_new_line();
    }
}

/// Бесконечная остановка процессора
pub fn halt_loop() -> ! {
    idt::interrupts_disable();
    loop {
        unsafe {
            asm!("hlt", options(nomem, nostack));
        }
    }
}

/// Обработчик паники
#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    halt_loop()
}
