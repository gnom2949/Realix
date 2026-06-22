# ℹ️ Realix `v0.06`
![Status](https://img.shields.io/badge/status-legacy-yellow)
![License](https://img.shields.io/github/license/NightFox-YT/Realix)
![Architecture](https://img.shields.io/badge/architecture-x86-blue)

✅ This version is officially supported and frequently updated by the author.

## 📌 About
Realix is a **hybrid OS** designed for x86 architecture, written in NASM.
It supports a built-in boot switcher that lets users choose between a 16-bit Real Mode kernel for legacy compatibility and a high-performance 32-bit Protected Mode kernel.

- **OS size:** `≈3.8 KB`
- **Initial release:** `June 14, 2026`

## ✨ Key Features
- BIOS-based bootloader
- VGA text mode (80×25)
- VGA video mode (320×200, 256 colors)
- Reads raw sectors from disk (INT 13h)
- Read-only FAT12 filesystem support
  - Loads second-stage bootloader
  - Loads kernel files by filename
  - Loads 32-bit kernel
- Detects available system memory (INT 12h, 15h)
- TTY bell character support
- 🆕 Interactive CPU mode selector (`switcher.asm`)
- 🆕 Kernel16: Basic command-line interface
- Basic Raster Graphics
- No int 0x80, only sysenter
- Realix libc 
- Full FAT32 read/write FS
- Virtual File System
- ATA, PCI and serial drivers

### ⏳ Upcoming Features (v0.07-v0.08)
- Kernel32: Unix time, kernel panic and vash

### ❌ Current Limitations 
- No standart executable support
- No write operations FAT12 support
- No networking stack

## 📸 Preview
![Realix Experience](screencast.gif)

## 📦 Hardware Requirements
- **CPU:** x86 compatible (any Pentium II+(FOR KERNEL32)
- **RAM:** 256 KB or more
- **Motherboard:** BIOS-supported

## 📂 Project Structure
```
.
├─ source/
│  ├─ bios-api/
│  │  ├─ disk/
│  │  │  ├─ init.asm
│  │  │  └─ read.asm
│  │  ├─ drivers/
│  │  │  ├─ sounds.asm
│  │  │  └─ vga.asm
│  │  ├─ fat12/
│  │  │  ├─ file_open.asm
│  │  │  └─ init.asm
│  │  └─ memory/
│  │     ├─ get_free.asm
│  │     ├─ get_lower.asm
│  │     └─ get_map.asm
│  ├─ bootloader/
│  │  ├─ bootix.asm
│  │  ├─ initrix.asm
│  │  └─ switcher.asm
│  └─ kernel16/
│     ├─ io/
│     │  ├─ print_nl.asm
│     │  ├─ print_reg.asm
│     │  └─ print.asm
│     ├─ shell/
│     │  ├─ cli.asm
│     │  ├─ cmd_cls.asm
│     │  └─ commands.asm
│     └─ kernel.asm
├─ build/
│     # Output directory for compiled binaries and .img (gitignored)
├─ Makefile
├─ LICENSE
├─ README.md
└─ screen.png
```

## 🛠️ Quick Start & Build

**Prerequisites**
* Compiler: `nasm` (Assembly)
* Compiler: `gcc` (C) or gcc-i386-elf
* Disk Tools: `mtools` (FAT12 image formatting), `coreutils` (dd/image creation)
* (Optional) Emulator: `qemu-system-i386`

### Linux & macOS
1. Install prerequisites
  - Ubuntu/Debian example: `sudo apt update && sudo apt install nasm mtools qemu-system-i386 gcc make`
  - macOS example (via Homebrew): `brew install nasm mtools qemu i686-elf-gcc`
2. Build & Run via the `Makefile`
  - Only build OS image: `make`
  - Full cycle (build & run in QEMU): `make run`

### Windows
> ⚠️ Native Windows builds are no longer supported as of `v0.03`.
> Please use Linux, macOS, or WSL2 (Recommended) instead.

## 🔗 Links & 🙌 Contributing
- **TikTok:** [tiktok.com/@mainfox.tt](https://www.tiktok.com/@mainfox.tt)
- **Discord:** [discord.gg/Realix](https://discord.gg/D7cATZzSAxp)

Contributions of any kind are welcome:

- 🐞 **Report bugs.**
- 💡 **Suggest new features** or improvements.
- 🔧 **Help optimize or refactor code.**

Feel free to open an issue or reach out via TikTok & Discord.