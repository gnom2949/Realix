// © Realix > Utils > TTF2RTF
// © 2026 Alexander Silaev <thebinaryblob@gmail.com>
// ===========================
// Парсер TTF и генератор RTF.
// RTF - Real Type Font. An SDF-Based Technology.
// ! - зависимости: libfreetype6-dev(debian) freetype-devel(void linux).
// Выполнять строго на хосте!!!
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_IMAGE_H

#define RTF_MAGIC 0x504F4F50 // какашка
#define SPREAD 8

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint16_t atlas_width;
    uint16_t atlas_height;
    uint16_t glyphs_count;
    uint8_t  font_size;
    uint8_t  spread;
    uint8_t  padding[2];
} RTFHeader;

typedef struct {
    uint32_t char_code;
    uint16_t tex_x;
    uint16_t tex_y;
    uint16_t tex_w;
    uint16_t tex_h;
    int16_t  offset_x;
    int16_t  offset_y;
    uint16_t advance_x;
    uint8_t  padding[2];
} RTFGlyphMetrics;
#pragma pack(pop)

uint8_t calculate_sdf_pixel(FT_Bitmap *bitmap, int t_x, int t_y, int width, int height)
{
    bool is_inside = false;
    if (t_x >= 0 && t_x < (int)bitmap->width && t_y >= 0 && t_y < (int)bitmap->rows)
        if (bitmap->buffer[t_y * bitmap->pitch + t_x] > 128) is_inside = true;

    float min_dist = (float)SPREAD;

    for (int ny = t_y - SPREAD; ny <= t_y + SPREAD; ny++)
    {
        for (int nx = t_x - SPREAD; nx <= t_x + SPREAD; nx++)
        {
            bool neighbor_inside = false;
            if (nx >= 0 && nx < (int)bitmap->width && ny >= 0 && ny < (int)bitmap->rows)
                if (bitmap->buffer[ny * bitmap->pitch + nx] > 128) 
                    neighbor_inside = true;
            
            if (is_inside != neighbor_inside)
            {
                float dx = (float)(nx - t_x);
                float dy = (float)(ny - t_y);
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist < min_dist) min_dist = dist;
            }
        }
    }

    float factor = min_dist / (float)SPREAD;
    if (is_inside)
        return (uint8_t)(128.0f + (factor * 127.0f));
    else 
        return (uint8_t)(128.0f - (factor * 128.0f));
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Использование: %s <путь_к_ttf> -o <выходной_rtf>\n", argv[0]);
        return 1;
    }

    FT_Library library;
    FT_Face face;
    char *output;

    if (strcmp(argv[2], "-o") == 0) output = argv[3];

    if (FT_Init_FreeType(&library) || FT_New_Face(library, argv[1], 0, &face))
    {
        fprintf(stderr, "Ошибка инициализации FreeType или загрузки шрифта.");
        return 1;
    }

    uint32_t font_size = 64;
    FT_Set_Pixel_Sizes(face, 0, font_size);


    uint32_t start_char = 32;
    uint32_t end_char = 126;
    uint16_t glyphs_count = (uint16_t)(end_char - start_char + 1);

    RTFGlyphMetrics *metrics_table = malloc(sizeof(RTFGlyphMetrics) * glyphs_count);
    
    uint16_t atlas_w = 0;
    uint16_t atlas_h = 0;

    for (uint32_t i = 0; i < glyphs_count; i++)
    {
        uint32_t code = start_char + i;
        if (!FT_Load_Char(face, code, FT_LOAD_RENDER))
        {
            atlas_w += face->glyph->bitmap.width + (SPREAD * 2);
            if ((int)face->glyph->bitmap.rows + (SPREAD * 2) > atlas_h)
                atlas_h = face->glyph->bitmap.rows + (SPREAD * 2);
        }
    }

    uint8_t *atlas_buffer = calloc(atlas_w * atlas_h, sizeof(uint8_t));

    uint16_t current_x = 0;
    for (uint32_t i = 0; i < glyphs_count; i++)
    {
        uint32_t code = start_char + i;
        FT_Load_Char(face, code, FT_LOAD_RENDER);
        FT_GlyphSlot g = face->glyph;

        uint16_t g_w = g->bitmap.width + (SPREAD * 2);
        uint16_t g_h = g->bitmap.rows + (SPREAD * 2);

        metrics_table[i].char_code = code;
        metrics_table[i].tex_x = current_x;
        metrics_table[i].tex_y = 0;
        metrics_table[i].tex_w = g_w;
        metrics_table[i].tex_h = g_h;
        metrics_table[i].offset_x = (int16_t)(g->bitmap_left - SPREAD);
        metrics_table[i].offset_y = (int16_t)(g->bitmap_top + SPREAD); 
        metrics_table[i].advance_x = (uint16_t)(g->advance.x >> 6);
        metrics_table[i].padding[0] = 0;
        metrics_table[i].padding[1] = 0;

        for (int y = 0; y < g_h; y++)
        {
            for (int x = 0; x < g_w; x++)
            {
                int orig_x = x - SPREAD;
                int orig_y = y - SPREAD;

                uint8_t sdf_pixel = calculate_sdf_pixel(&g->bitmap, orig_x, orig_y, g_w, g_h);

                int atlas_pixel_idx = (y * atlas_w) + (current_x + x);
                atlas_buffer[atlas_pixel_idx] = sdf_pixel;
            }
        }
        current_x += g_w;
    }

    RTFHeader header = {
        .magic = RTF_MAGIC,
        .atlas_width = atlas_w,
        .atlas_height = atlas_h,
        .glyphs_count = glyphs_count,
        .font_size = font_size,
        .spread = SPREAD,
        .padding = {0, 0}
    };

    FILE *out = fopen(output, "wb");
    if (out)
    {
        fwrite(&header, sizeof(RTFHeader), 1, out);
        fwrite(metrics_table, sizeof(RTFGlyphMetrics), glyphs_count, out);
        fwrite(atlas_buffer, sizeof(uint8_t), atlas_w * atlas_h, out);
        fclose(out);
        printf("Успешно сгенерирован шрифт: %s:%s (%dx%d пикселей атлас)\n", argv[1], output, atlas_w, atlas_h);
    } else fprintf(stderr, "Не удалось открыть файл на запись!\n");

    free(metrics_table);
    free(atlas_buffer);
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return 0;
}