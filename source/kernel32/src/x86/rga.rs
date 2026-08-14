/* RGA the Realix Graphics Access
    Copyright(C) 2026 Alexander Silaev <thebinaryblob@gmail.com>
*/
use core::arch::asm;
use core::mem::size_of;

const IA32_SYSENTER_CS: u32 = 0x174;
const IA32_SYSENTER_ESP: u32 = 0x175;
const IA32_SYSENTER_EIP: u32 = 0x176;

const MAX_PROCS: usize = 64;
