#ifndef __bmp__
#define __bmp__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define LCD_PATH "/dev/fb0"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

#pragma pack(1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPFileHeader;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;
#pragma pack()

void display_bmp_scaled(const char *filename) {
    int lcd_fd = open(LCD_PATH, O_RDWR);
    if (lcd_fd == -1) {
        perror("Failed to open LCD");
        return;
    }

    struct fb_var_screeninfo vinfo;
    if (ioctl(lcd_fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Failed to get screen info");
        close(lcd_fd);
        return;
    }

    int screen_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    uint32_t *fb = mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if (fb == MAP_FAILED) {
        perror("Failed to mmap framebuffer");
        close(lcd_fd);
        return;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Failed to open BMP file");
        munmap(fb, screen_size);
        close(lcd_fd);
        return;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    fread(&file_header, sizeof(BMPFileHeader), 1, fp);
    fread(&info_header, sizeof(BMPInfoHeader), 1, fp);

    if (file_header.bfType != 0x4D42) { // 检查 BMP 文件标识
        fprintf(stderr, "Not a valid BMP file\n");
        fclose(fp);
        munmap(fb, screen_size);
        close(lcd_fd);
        return;
    }

    int img_width = info_header.biWidth;
    int img_height = abs(info_header.biHeight); // BMP 高度可能为负，表示自下而上存储

    fseek(fp, file_header.bfOffBits, SEEK_SET);

    uint8_t *img_data = malloc(img_width * img_height * 3); // 假设 24 位色
    fread(img_data, img_width * img_height * 3, 1, fp);

    fclose(fp);

    // 计算缩放比例
    float scale_w = (float)SCREEN_WIDTH / img_width;
    float scale_h = (float)SCREEN_HEIGHT / img_height;
    float scale = scale_w < scale_h ? scale_w : scale_h;

    int scaled_width = img_width * scale;
    int scaled_height = img_height * scale;

    // 显示图片，处理镜像
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int src_x = x / scale;
            int src_y = (SCREEN_HEIGHT - 1 - y) / scale; // 倒置行

            if (src_x < img_width && src_y < img_height) {
                uint8_t *px = &img_data[(src_y * img_width + src_x) * 3];
                uint32_t color = (px[2] << 16) | (px[1] << 8) | px[0];
                fb[y * SCREEN_WIDTH + x] = color;
            } else {
                fb[y * SCREEN_WIDTH + x] = 0x000000; // 黑色填充
            }
        }
    }

    free(img_data);
    munmap(fb, screen_size);
    close(lcd_fd);
}

void show_bmp(char *s, int startX, int startY, int lcd)
{ // bmp的文件路径，图像在屏幕上显示的起始x坐标，  y的起始坐标，lcd板子
    int color = 0x00ff0000;
    int width, higth;
    short depth;
    int B, G, R;
    int x, y;
    //控制屏幕的第一步    打开屏幕文件
    int *plcd = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd, 0);
    int bmp = open(s, O_RDWR);
    lseek(bmp, 0x12, SEEK_SET);
    //读取宽度信息
    read(bmp, &width, 4);
    printf("宽度 = %d\n", width);

    lseek(bmp, 0x16, SEEK_SET);
    //读取度信息
    read(bmp, &higth, 4);
    printf("高度 = %d\n", higth);

    lseek(bmp, 0x1c, SEEK_SET);
    //读取深度信息
    read(bmp, &depth, 2);
    printf("深度 = %d\n", depth);

    //像素数据的提取  像素数据的大小 宽度 *高度   每个像素占3个字节
    //像素数据大小  宽度 *高度*深度/8
    char buf[width * higth * depth / 8]; //用于保存像素数据 大小由图片的规格决定
    lseek(bmp, 0x36, SEEK_SET);
    //读取像素数据信息
    read(bmp, buf, width * higth * depth / 8);
    //解析状态
    int n = 0;

    for (y = startY + higth - 1; y >= startY; y--)
    {
        for (x = startX; x < startX + width; x++)
        {
            B = buf[n++];
            G = buf[n++];
            R = buf[n++];
            color = (R << 16) | (G << 8) | B;
            //显示逻辑
            if (x >= 0 && x < 800 && y >= 0 && y < 480)
            {
                *(plcd + y * 800 + x) = color;
            }
        }
    }
}
#endif
//INSERT INTO usr (username, password) VALUES ('1', '1');
