#!/usr/bin/env python3
"""Собирает thrfs:/boot.cfg (bootcfg_t, см. include/bootcfg.h) из аргументов
командной строки.

Пример:
    ./make_bootcfg.py --title RealixOS --background bg.tga --timer 20 -o assets/boot.cfg
"""
import argparse
import struct

MAGIC = b"TCFG"
VERSION = 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--title", default="RealixOS", help="Текст над меню (макс. 31 байт)")
    parser.add_argument("--background", default="", help="Имя файла фона в thrfs (макс. 31 байт), пусто = нет фона")
    parser.add_argument("--timer", type=int, default=20, help="Таймаут меню в секундах")
    parser.add_argument("-o", "--output", default="assets/boot.cfg", help="Куда сохранить")
    args = parser.parse_args()

    if len(args.title.encode()) > 31:
        raise SystemExit("--title слишком длинный (макс. 31 байт, включая место под NUL)")
    if len(args.background.encode()) > 31:
        raise SystemExit("--background слишком длинный (макс. 31 байт, включая место под NUL)")
    if not (0 <= args.timer <= 0xFFFF):
        raise SystemExit("--timer вне диапазона uint16_t (0-65535)")

    data = struct.pack(
        "<4sB32s32sH16s",
        MAGIC,
        VERSION,
        args.background.encode(),
        args.title.encode(),
        args.timer,
        b"\x00" * 16,
    )
    assert len(data) == 87, f"неожиданный размер {len(data)} байт — формат разошёлся с bootcfg_t?"

    with open(args.output, "wb") as f:
        f.write(data)

    print(f"[+] {args.output} ({len(data)} байт): "
          f"title={args.title!r} background={args.background!r} timer={args.timer}s")


if __name__ == "__main__":
    main()