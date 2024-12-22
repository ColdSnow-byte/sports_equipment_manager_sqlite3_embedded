#ifndef touch
#define touch
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define LCD_PATH "/dev/fb0"
#define TOUCH_DEVICE "/dev/input/event0"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480
#define TOUCH_MAX_X 1024  // 原始触摸屏 X 方向的最大值（需根据设备调整）
#define TOUCH_MAX_Y 600   // 原始触摸屏 Y 方向的最大值（需根据设备调整）
void get_touch(int *x, int *y) {
    struct input_event ev;
    int fd = open(TOUCH_DEVICE, O_RDONLY);
    if (fd == -1) {
        perror("open touch device failed");
        exit(1);
    }

    int raw_x = -1, raw_y = -1; // 原始坐标
    while (1) {
        ssize_t bytes = read(fd, &ev, sizeof(struct input_event));
        if (bytes < (ssize_t) sizeof(struct input_event)) {
            perror("read failed");
            exit(1);
        }

        // 捕获触摸事件
        if (ev.type == EV_ABS) {
            if (ev.code == ABS_X) {
                raw_x = ev.value; // 获取原始 X 坐标
            } else if (ev.code == ABS_Y) {
                raw_y = ev.value; // 获取原始 Y 坐标
            }
        } else if (ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0) {
            // 手指抬起时退出
            break;
        }
    }

    close(fd);

    // 坐标缩放
    if (raw_x >= 0 && raw_y >= 0) {
        *x = raw_x * SCREEN_WIDTH / TOUCH_MAX_X;   // 将 X 坐标缩放到屏幕范围
        *y = raw_y * SCREEN_HEIGHT / TOUCH_MAX_Y; // 将 Y 坐标缩放到屏幕范围
    }
}
#endif