#ifndef ttf
#define ttf

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include "font.h"
// 在 LCD 上叠加显示文字（透明背景，白色字体）
void display_text_on_image(font *f, int *lcd, int x, int y,  char *text) {
    bitmap *text_bitmap = createBitmapWithInit(SCREEN_WIDTH, 28, 4, 0x00000000); // 透明背景
    fontPrint(f, text_bitmap, 0, 0, text, 0xff000000, 0); // 白色字体

    // 将文字叠加到屏幕
    int text_width = text_bitmap->width;
    int text_height = text_bitmap->height;
    uint8_t *src = (uint8_t *)text_bitmap->map;

    for (int j = 0; j < text_height; j++) {
        for (int i = 0; i < text_width; i++) {
            uint32_t pixel = ((uint32_t *)src)[j * text_width + i];
            if (pixel != 0x00000000) { // 仅绘制文字
                *(lcd + (y + j) * SCREEN_WIDTH + (x + i)) = pixel;
            }
        }
    }
    destroyBitmap(text_bitmap); // 释放 bitmap
}
void de(font *f, int *lcd, int x, int y,  char *text) {
    bitmap *text_bitmap = createBitmapWithInit(SCREEN_WIDTH, 28, 4, 0x00000000); // 透明背景
    fontPrint(f, text_bitmap, 0, 0, text, 0xFFFFFFFF, 0); // 白色字体

    // 将文字叠加到屏幕
    int text_width = text_bitmap->width;
    int text_height = text_bitmap->height;
    uint8_t *src = (uint8_t *)text_bitmap->map;
    for (int j = 0; j < text_height; j++) {
        for (int i = 0; i < text_width; i++) {
            // 清空原来区域（用透明背景颜色填充）
            *(lcd + (y + j) * SCREEN_WIDTH + (x + i)) = 0x00000000;  // 透明背景
        }
    }
    for (int j = 0; j < text_height; j++) {
        for (int i = 0; i < text_width; i++) {
            uint32_t pixel = ((uint32_t *)src)[j * text_width + i];
            if (pixel != 0x00000000) { // 仅绘制文字
                *(lcd + (y + j) * SCREEN_WIDTH + (x + i)) = pixel;
            }
        }
    }
    destroyBitmap(text_bitmap); // 释放 bitmap
}
#endif