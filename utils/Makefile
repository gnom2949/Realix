# © Realix > Utils > Makefile.
# © 2026 Alexander Silaev <thebinaryblob@gmail.com>
# ============================
# Makefile для сборки утилит или запуска.

СС = gcc
SRC = ttf2rtf.c
OUT = ttf2rtf
CFLAGS = $(shell pkg-config --cflags freetype2) -I/usr/include/freetype2
LDFLAGS = $(shell pkg-config --libs freetype2) -lm

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) ttf2rtf.c -o ttf2rtf $(LDFLAGS)

clean:
	rm -f $(OUT)

help:
	@echo "© Realix > Utils."
	@echo "Use make help for this message."
	@echo "Use make for build."
	@echo "Use make clean for clean environment."

.PHONY: all help build