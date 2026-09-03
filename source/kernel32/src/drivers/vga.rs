// © Realix > Driver: VGA
// ø Inspired by @liquifield
// Modified by Alexander Silaev <thebinaryblob@gmail.com>
// (24.08.26) v0.12
// ================
// ! Не вызывать из IRQ прерываний (Гонка данных на константах)
// Добавлена поддержка VBE(Линейный фреймбуфер произвольного адреса/разрешения/bpp).
// Добавлена поддержка Real Type Font + 8x16 Bitmap для VBE режима.
// ! - Текст в VBE через RTF, если он не загружен то выводится фолбек в виде bitmap шрифта.
// Рендер RTF сейчас БЕЗ АНТИАЛИАСИНГА, я думаю доделать это позже, может TODO сделаю.

// Подключение функций
use core::sync::atomic::{AtomicUsize, Ordering::Relaxed};
use crate::utils::{self, outb};
use crate::drivers::font::{FONT_BITMAP, FONT_WIDTH, FONT_HEIGHT, RealTypeFontParser, RTFGlyphMetrics};

// Константы
const VGA_TEXT_BUFFER: *mut u8 = 0xB8000 as *mut u8;
const VGA_VIDEO_BUFFER: *mut u8 = 0xA0000 as *mut u8;
pub const VGA_TEXT_WIDTH: usize = 80;
pub const VGA_TEXT_HEIGHT: usize = 25;
pub const VGA_VIDEO_WIDTH: usize = 320;
pub const VGA_VIDEO_HEIGHT: usize = 200;
const EMPTY_CELL: u16 = (0x0F << 8) | b' ' as u16;  // Пробел + 0x0F (белый на чёрном)

// Регистры CRT-контроллера для управления аппаратным курсором
const VGA_CRTC_INDEX: u16 = 0x3D4;  // Индексный порт
const VGA_CRTC_DATA:  u16 = 0x3D5;  // Порт данных
const VGA_CURSOR_HIGH: u8 = 0x0E;   // Регистр старшего байта позиции курсора
const VGA_CURSOR_LOW:  u8 = 0x0F;   // Регистр младшего байта позиции курсора

// Позиция курсора
static CURSOR_ROW: AtomicUsize = AtomicUsize::new(0);
static CURSOR_COL: AtomicUsize = AtomicUsize::new(0);

// Позиция курсора (в пикселях шрифт может быть непропорциональным, поэтому это не колонка/строка).
static VBE_CURSOR_X: AtomicUsize = AtomicUsize::new(0);
static VBE_CURSOR_Y: AtomicUsize = AtomicUsize::new(0);

// Таблица цветов
#[allow(dead_code)]
#[derive(Clone, Copy)]
#[repr(u8)]
pub enum Color {
    Black = 0x0,
    Blue = 0x1,
    Green = 0x2,
    Cyan = 0x3,
    Red = 0x4,
    Magenta = 0x5,
    Brown = 0x6,
    LightGray = 0x7,
    DarkGray = 0x8,
    LightBlue = 0x9,
    LightGreen = 0xA,
    LightCyan = 0xB,
    LightRed = 0xC,
    Pink = 0xD,
    Yellow = 0xE,
    White = 0xF,
}

/// Общая структура для одноименных функций в видеорежиме и текстовом режиме
pub struct VideoOps {
    pub print_line:   fn(line: &str, color: Color),
    pub print_char:   fn(char_byte: u8, color: Color),
    pub clear_screen: fn(),
    pub set_pixel:    fn(x: usize, y: usize, color: Color),
    pub fill_screen:  fn(color: Color),
    pub draw_hline:   fn(x1: usize, x2: usize, y: usize, color: Color),
    pub draw_vline:   fn(x: usize, y1: usize, y2: usize, color: Color),
    pub draw_rect:    fn(x1: usize, x2: usize, y1: usize, y2: usize, color: Color),
}

#[derive(Clone, Copy)]
pub struct FrameBufferInfo {
    pub addr:   u32,
    pub pitch:  u32,
    pub width:  u32,
    pub height: u32,
    pub bpp:    u8,
}

static mut VBE_FB_ADDR: u32 = 0;
static mut VBE_PITCH:   u32 = 0;
static mut VBE_WIDTH:   u32 = 0;
static mut VBE_HEIGHT:  u32 = 0;
static mut VBE_BPP:     u8 = 0;

// RTF-Шрифт, пока в realix 32 нет файловой системы приходится загружать прямо вплотную,
// и тут не знаю когда лид проекта опомнится чтобы сделать хотя бы FAT32 драйвер, хотя он у меня вроде есть но завязан на ATA.
static mut RTF_FONT: Option<RealTypeFontParser> = None;
pub const COMFORTAA_DATA: &[u8] = include_bytes!("../../../../utils/comfortaa.rtf");
pub fn set_comfortaa_font()
{
    let parser = RealTypeFontParser::new(COMFORTAA_DATA);
    if let Some(header) = parser.get_header() {
        
    }
}

pub fn set_font(font: RealTypeFontParser) {
    unsafe { RTF_FONT = Some(font); }
}

pub fn clear_font() {
    unsafe { RTF_FONT = None; }
}

/// Инициализация OPS, используя структуру VideoOps
/// Параметры:
/// - videomode: 0 - VGA Text, 1 - VBE.
/// - framebuffer: обязателен для videomode 1.
pub fn init(videomode: u16, framebuffer: Option<FrameBufferInfo>) {
    unsafe {
        OPS = match videomode {
            0 => VideoOps {
                print_line:     text_print_line,
                print_char:     text_print_char,
                clear_screen:   text_clear_screen,
                set_pixel:      text_noop_set_pixel,
                fill_screen:    text_noop_fill_screen,
                draw_hline:     text_noop_draw_hline,
                draw_vline:     text_noop_draw_vline,
                draw_rect:      text_noop_draw_rect,
            },

            1 => {
                if let Some(fb) = framebuffer {
                    VBE_FB_ADDR = fb.addr;
                    VBE_PITCH   = fb.pitch;
                    VBE_BPP     = fb.bpp;
                    VBE_WIDTH   = fb.width;
                    VBE_HEIGHT  = fb.height;
                }

                VBE_CURSOR_X.store(0, Relaxed);
                VBE_CURSOR_Y.store(0, Relaxed);

                VideoOps {
                    print_line:     vbe_print_line,
                    print_char:     vbe_print_char,
                    clear_screen:   vbe_clear_screen,
                    set_pixel:      vbe_set_pixel,
                    fill_screen:    vbe_fill_screen,
                    draw_hline:     vbe_draw_hline,
                    draw_vline:     vbe_draw_vline,
                    draw_rect:      vbe_draw_rect,
                }
            },
            _ => panic!("unsupported video mode"),
        };
    }
}



// Основной интерфейс (Дефолтный)
static mut OPS: VideoOps = VideoOps {
    print_line: text_print_line,
    print_char: text_print_char,
    clear_screen: text_clear_screen,
    set_pixel: text_noop_set_pixel,
    fill_screen: text_noop_fill_screen,
    draw_hline: text_noop_draw_hline,
    draw_vline: text_noop_draw_vline,
    draw_rect: text_noop_draw_rect,
};

// Публичный интерфейс модуля
pub fn clear_screen() { unsafe { (OPS.clear_screen)() } }
pub fn print_char(char_byte: u8, color: Color) { unsafe { (OPS.print_char)(char_byte, color) } }
pub fn print_line(line: &str, color: Color) { unsafe { (OPS.print_line)(line, color) } }
pub fn fill_screen(color: Color) { unsafe { (OPS.fill_screen)(color) } }
pub fn draw_hline(x1: usize, x2: usize, y: usize, color: Color) { unsafe { (OPS.draw_hline)(x1, x2, y, color) } }
pub fn draw_vline(x: usize, y1: usize, y2: usize, color: Color) { unsafe { (OPS.draw_vline)(x, y1, y2, color) } }
pub fn draw_rect(x1: usize, x2: usize, y1: usize, y2: usize, color: Color) { unsafe { (OPS.draw_rect)(x1, x2, y1, y2, color) } }

// Заглушки для текстового режима
fn text_noop_set_pixel(_x: usize, _y: usize, _color: Color) {}
fn text_noop_fill_screen(_color: Color) {}
fn text_noop_draw_hline(_x1: usize, _x2: usize, _y: usize, _color: Color) {}
fn text_noop_draw_vline(_x: usize, _y1: usize, _y2: usize, _color: Color) {}
fn text_noop_draw_rect(_x1: usize, _x2: usize, _y1: usize, _y2: usize, _color: Color) {}

// VBE: RGB Truecolor
fn color_to_rgb32(color: Color) -> u32 {
    match color {
        Color::Black      => 0x000000,
        Color::Blue       => 0x0000AA,
        Color::Green      => 0x00AA00,
        Color::Cyan       => 0x00AAAA,
        Color::Red        => 0xAA0000,
        Color::Magenta    => 0xAA00AA,
        Color::Brown      => 0xAA5500,
        Color::LightGray  => 0xAAAAAA,
        Color::DarkGray   => 0x555555,
        Color::LightBlue  => 0x5555FF,
        Color::LightGreen => 0x55FF55,
        Color::LightCyan  => 0x55FFFF,
        Color::LightRed   => 0xFF5555,
        Color::Pink       => 0xFF55FF,
        Color::Yellow     => 0xFFFF55,
        Color::White      => 0xFFFFFF,
    }
}

pub fn vbe_set_pixel(x: usize, y: usize, color: Color) {
    unsafe {
        if VBE_FB_ADDR == 0 || x >= VBE_WIDTH as usize || y >= VBE_HEIGHT as usize {
            return;
        }
 
        let rgb = color_to_rgb32(color);
        let bytes_per_pixel = (VBE_BPP as usize) / 8;
        let offset = y * VBE_PITCH as usize + x * bytes_per_pixel;
        let pixel_ptr = (VBE_FB_ADDR as usize + offset) as *mut u8;
 
        match VBE_BPP {
            32 => {
                (pixel_ptr as *mut u32).write_volatile(rgb);
            }
            24 => {
                pixel_ptr.write_volatile((rgb & 0xFF) as u8);
                pixel_ptr.add(1).write_volatile(((rgb >> 8) & 0xFF) as u8);
                pixel_ptr.add(2).write_volatile(((rgb >> 16) & 0xFF) as u8);
            }
            _ => {} // 16/15/8bpp не поддержаны
        }
    }
}
 
pub fn vbe_fill_screen(color: Color) {
    unsafe {
        if VBE_FB_ADDR == 0 {
            return;
        }
        for y in 0..VBE_HEIGHT as usize {
            for x in 0..VBE_WIDTH as usize {
                vbe_set_pixel(x, y, color);
            }
        }
    }
}
 
pub fn vbe_draw_hline(x1: usize, x2: usize, y: usize, color: Color) {
    for curr_x in x1..=x2 {
        vbe_set_pixel(curr_x, y, color);
    }
}
 
pub fn vbe_draw_vline(x: usize, y1: usize, y2: usize, color: Color) {
    for curr_y in y1..=y2 {
        vbe_set_pixel(x, curr_y, color);
    }
}
 
pub fn vbe_draw_rect(x1: usize, x2: usize, y1: usize, y2: usize, color: Color) {
    for curr_y in y1..=y2 {
        vbe_draw_hline(x1, x2, curr_y, color);
    }
}
 
pub fn vbe_clear_screen() {
    unsafe {
        if VBE_FB_ADDR != 0 {
            let total_bytes = VBE_PITCH as usize * VBE_HEIGHT as usize;
            let fb = VBE_FB_ADDR as usize as *mut u8;
            for i in 0..total_bytes {
                fb.add(i).write_volatile(0);
            }
        }
    }
    VBE_CURSOR_X.store(0, Relaxed);
    VBE_CURSOR_Y.store(0, Relaxed);
}
 
pub fn vbe_width() -> u32 { unsafe { VBE_WIDTH } }
pub fn vbe_height() -> u32 { unsafe { VBE_HEIGHT } }

// VBE: текст RTF как основа, bitmap как фолбек

// Рисует один RTF глиф, жёсткий порог без сглаживания, сэмплирует атлас 1 к 1.
fn vbe_draw_glyph_rtf(font: &RealTypeFontParser, glyph: &RTFGlyphMetrics, x: usize, y: usize, fg: Color) {
    let draw_x = (x as i32 + glyph.offset_x as i32).max(0) as usize;
    let draw_y = (y as i32 + glyph.offset_y as i32).max(0) as usize;
 
    for row in 0..glyph.tex_h as usize {
        for col in 0..glyph.tex_w as usize {
            let sample = font.get_atlas_pixel(glyph.tex_x as usize + col, glyph.tex_y as usize + row);
            if sample >= 128 {
                vbe_set_pixel(draw_x + col, draw_y + row, fg);
            }
        }
    }
}
 
pub fn vbe_draw_char(x: usize, y: usize, char_byte: u8, fg: Color) -> usize {
    unsafe {
        if let Some(font) = &RTF_FONT {
            if let Some(glyph) = font.find_glyph(char_byte as u32) {
                vbe_draw_glyph_rtf(font, &glyph, x, y, fg);
                return glyph.advance_x as usize;
            }
            // Шрифт загружен, но глифа для этого символа нет — падаем в bitmap ниже, а не молчим.
        }
    }
 
    let index = if (char_byte as usize) < FONT_BITMAP.len() { char_byte as usize } else { b'?' as usize };
    let glyph_rows = &FONT_BITMAP[index];
 
    for row in 0..FONT_HEIGHT {
        let bits = glyph_rows[row];
        for col in 0..FONT_WIDTH {
            if (bits >> (7 - col)) & 1 != 0 {
                vbe_set_pixel(x + col, y + row, fg);
            }
        }
    }
 
    FONT_WIDTH
}
 
pub fn vbe_print_char(char_byte: u8, color: Color) {
    match char_byte {
        b'\n' => {
            VBE_CURSOR_X.store(0, Relaxed);
            VBE_CURSOR_Y.fetch_add(FONT_HEIGHT, Relaxed);
        }
        b'\r' => { VBE_CURSOR_X.store(0, Relaxed); }
        _ => {
            let x = VBE_CURSOR_X.load(Relaxed);
            let y = VBE_CURSOR_Y.load(Relaxed);
            let advance = vbe_draw_char(x, y, char_byte, color);
            VBE_CURSOR_X.fetch_add(advance, Relaxed);
        }
    }
 
    let width = unsafe { VBE_WIDTH as usize };
    if width > 0 && VBE_CURSOR_X.load(Relaxed) + FONT_WIDTH > width {
        VBE_CURSOR_X.store(0, Relaxed);
        VBE_CURSOR_Y.fetch_add(FONT_HEIGHT, Relaxed);
    }
 
    let height = unsafe { VBE_HEIGHT as usize };
    while height > 0 && VBE_CURSOR_Y.load(Relaxed) + FONT_HEIGHT > height {
        vbe_scroll_up(FONT_HEIGHT);
    }
}
 
pub fn vbe_print_line(line: &str, color: Color) {
    for byte in line.bytes() {
        vbe_print_char(byte, color);
    }
}
 
fn vbe_scroll_up(pixel_lines: usize) {
    unsafe {
        if VBE_FB_ADDR == 0 {
            return;
        }
 
        let pitch = VBE_PITCH as usize;
        let height = VBE_HEIGHT as usize;
 
        if pixel_lines >= height {
            vbe_clear_screen();
            return;
        }
 
        let fb = VBE_FB_ADDR as usize as *mut u8;
        let move_bytes = (height - pixel_lines) * pitch;
 
        core::ptr::copy(fb.add(pixel_lines * pitch), fb, move_bytes);
 
        // Затираем освободившиеся строки снизу
        let clear_start = fb.add(move_bytes);
        for i in 0..(pixel_lines * pitch) {
            clear_start.add(i).write_volatile(0);
        }
    }
 
    let row = VBE_CURSOR_Y.load(Relaxed);
    VBE_CURSOR_Y.store(row.saturating_sub(pixel_lines), Relaxed);
}

/// Отдельно от OPS — эта функция вообще не должна дёргаться в текстовом режиме
pub fn vga_video_set_pixel(x: usize, y: usize, color: Color) {
    if x >= VGA_VIDEO_WIDTH || y > VGA_VIDEO_HEIGHT {
        return;
    }

    unsafe {
        let offset = (y * 320 + x) as isize;
        VGA_VIDEO_BUFFER.offset(offset).write_volatile(color as u8);
    }
}

/// Вывод строки на экран (VGA Video)
pub fn vga_video_fill_screen(color: Color) {
    for i in 0..VGA_VIDEO_WIDTH * VGA_VIDEO_HEIGHT {
        unsafe { VGA_VIDEO_BUFFER.add(i).write_volatile(color as u8); }
    }
}


/// Отрисовка горизонтальной линии (VGA Video)
pub fn vga_video_draw_hline(x1: usize, x2: usize, y: usize, color: Color) {
    for curr_x in x1..=x2 {
        vga_video_set_pixel(curr_x, y, color);
    }
}

/// Отрисовка вертикальной линии (VGA Video)
pub fn vga_video_draw_vline(x: usize, y1: usize, y2: usize, color: Color) {
    for curr_y in y1..=y2 {
        vga_video_set_pixel(x, curr_y, color);
    }
}

/// Отрисовка вертикальной линии (VGA Video)
pub fn vga_video_draw_rect(x1: usize, x2: usize, y1: usize, y2: usize, color: Color) {
    // Рисуем циклично горизонтальные строки
    for curr_y in y1..=y2 {
        draw_hline(x1, x2, curr_y, color);
    }
}
    

/// Заполнение `count` ячеек экрана пустой ячейкой, начиная с `start_cell`
fn clear_cells(start_cell: usize, count: usize) {
    let cells: *mut u16 = VGA_TEXT_BUFFER as *mut u16;

    for i in start_cell..start_cell + count {
        unsafe { cells.add(i).write_volatile(EMPTY_CELL); }
    }
}


/// Очистка экрана и сброс курсора в начало
pub fn text_clear_screen() {
    // "Стираем" экран пробелами с чёрным фоном
    clear_cells(0, VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT);

    // Сбрасываем позицию курсора
    CURSOR_ROW.store(0, Relaxed);
    CURSOR_COL.store(0, Relaxed);
    update_cursor();
}


/// Поднять все строки на экране на `n` позиций
fn scroll_up(lines_count: usize) {
    // Если кол-во строк для прокрутки больше чем высота VGA
    if lines_count >= VGA_TEXT_HEIGHT {
        clear_screen();
        return;
    }

    // Копируем строки `n`..HEIGHT в начало экрана единым блоком (memmove).
    unsafe {
        core::ptr::copy(
            VGA_TEXT_BUFFER.add(lines_count * VGA_TEXT_WIDTH * 2),
            VGA_TEXT_BUFFER,
            (VGA_TEXT_HEIGHT - lines_count) * VGA_TEXT_WIDTH * 2
        );
    }

    // "Стираем" последние `n` строк пробелами с чёрным фоном
    clear_cells((VGA_TEXT_HEIGHT - lines_count) * VGA_TEXT_WIDTH, lines_count * VGA_TEXT_WIDTH);

    // Обновляем позицию курсора на `n` строк вверх
    let row: usize = CURSOR_ROW.load(Relaxed);
    CURSOR_ROW.store(row.saturating_sub(lines_count), Relaxed);
    update_cursor();
}


/// Перенос аппаратного курсора CRTC в текущую позицию
fn update_cursor() {
    // Вычисляем новую позицию
    let row: usize = CURSOR_ROW.load(Relaxed);
    let col: usize = CURSOR_COL.load(Relaxed);
    let pos: u16 = (row * VGA_TEXT_WIDTH + col) as u16;

    // Выбираем регистр CRTC и передаём младший, затем старший байт позиции курсора
    unsafe {
        outb(VGA_CRTC_INDEX, VGA_CURSOR_LOW);
        outb(VGA_CRTC_DATA, (pos & 0xFF) as u8);

        outb(VGA_CRTC_INDEX, VGA_CURSOR_HIGH);
        outb(VGA_CRTC_DATA, (pos >> 8) as u8);
    }
}


/// Вывод символа на экран (по принципам TTY)
pub fn text_print_char(char_byte: u8, color: Color) {
    match char_byte {
        b'\n' => {
            CURSOR_COL.store(0, Relaxed);
            CURSOR_ROW.fetch_add(1, Relaxed);
        }
        b'\r' => { CURSOR_COL.store(0, Relaxed); }
        _ => {
            let row: usize = CURSOR_ROW.load(Relaxed);
            let col: usize = CURSOR_COL.load(Relaxed);
            let offset: usize = (row * VGA_TEXT_WIDTH + col) * 2;
            unsafe {
                VGA_TEXT_BUFFER.add(offset).write_volatile(char_byte);
                VGA_TEXT_BUFFER.add(offset + 1).write_volatile(color as u8);
            }
            CURSOR_COL.fetch_add(1, Relaxed);
        }
    }

    // Перенос курсора на след. строку
    if CURSOR_COL.load(Relaxed) >= VGA_TEXT_WIDTH {
        CURSOR_COL.store(0, Relaxed);
        CURSOR_ROW.fetch_add(1, Relaxed);
    }

    // Если курсор выходит за нижнюю границу экрана
    while CURSOR_ROW.load(Relaxed) >= VGA_TEXT_HEIGHT {
        scroll_up(1);
    }

    update_cursor();
}

/// Вывод символа в определённой позиции
pub fn write_char_at(row: usize, col: usize, char_byte: u8, color: Color) {
    // Проверка, что символ находитсья в пределах экрана
    if row >= VGA_TEXT_HEIGHT || col >= VGA_TEXT_WIDTH {
        return;
    }

    // Запись символа
    let offset: usize = (row * VGA_TEXT_WIDTH + col) * 2;
    unsafe {
        VGA_TEXT_BUFFER.add(offset).write_volatile(char_byte);
        VGA_TEXT_BUFFER.add(offset + 1).write_volatile(color as u8);
    }
}


/// Вывод строки на экран (по принципам TTY)
pub fn text_print_line(line: &str, color: Color) {
    for byte in line.bytes() {
        print_char(byte, color);
    }
}


/// Стирание последнего символа с переносом курсора назад
pub fn print_backspace() {
    // > Обновление позиции курсора
    if CURSOR_COL.load(Relaxed) > 0 {
        CURSOR_COL.fetch_sub(1, Relaxed);
    } else if CURSOR_ROW.load(Relaxed) > 0 {
        CURSOR_ROW.fetch_sub(1, Relaxed);
        CURSOR_COL.store(VGA_TEXT_WIDTH - 1, Relaxed);
    }

    // Замена последнего символа на пробел
    let row: usize = CURSOR_ROW.load(Relaxed);
    let col: usize = CURSOR_COL.load(Relaxed);
    let offset: usize = (row * VGA_TEXT_WIDTH + col) * 2;
    unsafe {
        VGA_TEXT_BUFFER.add(offset).write_volatile(b' ');
        VGA_TEXT_BUFFER.add(offset + 1).write_volatile(0x0F);
    }

    update_cursor();
}

/// Перевод строки
pub fn print_new_line() {
    print_char(b'\n', Color::White);
}

/// Перевод строки, если надо
pub fn print_new_line_if_needed() {
    if CURSOR_COL.load(Relaxed) != 0 {
        print_char(b'\n', Color::White);
    }
}

/// Вывод строки дампа регистра
pub fn print_reg_line(reg_label: &str, value: u32) {
    let mut buffer: [u8; 10] = [0u8; 10];

    print_line("> ", Color::LightGray);
    print_line(reg_label, Color::LightGray);
    print_line(" = ", Color::LightGray);
    print_line(utils::u32_to_hex_str(value, &mut buffer), Color::White);
    print_new_line();
}