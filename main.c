#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include "db2.h"
#include <sqlite3.h> // 包含 SQLite 的头文件
#include "bmp.h"
#include "touch.h"
#include "button.h"
#include "ttf.h"
#define LCD_PATH "/dev/fb0"
#define TOUCH_DEVICE "/dev/input/event0"

//编译指令
//arm-linux-gcc mian.c font.c truetype.c -o sc7 -lm -I /home/china/sqlite3_arm/include/ -L /home/china/sqlite3_arm/lib/ -lsqlite3

void add(char *str, char c)
{
    int len = strlen(str);
    if (len < sizeof(str) - 1) {
        str[len] = c;
        str[len + 1] = '\0';
    }
}

void del(char *str)
{
    int len = strlen(str);
    if (len > 0)
    {
        str[len - 1] = '\0';
    }
}

void safe_add(char *str, char c)
{
    add(str, c);
}

int data_deal(void *arg, int num, char **value, char **name)
{
    // 每次查询到之后会去执行的操作
    int i;
    for (i = 0; i < num; i++)
    {
        printf("%s %s\n", name[i], value[i]);
    }
    return 0; // 返回值为0才会继续操作
}

// 用于验证用户名和密码是否匹配的回调函数
int verify_login(void *arg, int num, char **value, char **name)
{
    char *input_username = (char *)arg;  // 输入的用户名
    char *input_password = (char *)name[1];  // 输入的密码

    if (num > 0) {
        // 检查返回的用户名和密码是否匹配
        if (strcmp(value[0], input_username) == 0 && strcmp(value[1], input_password) == 0) {
            return 1;  // 登录成功
        }
    }
    return 0;  // 登录失败
}


int failLogin() {
    // 打开图片文件
    int width = -1;
    int height = -1;
    short depth = -1;
    int bmp = open("failLogin.bmp", O_RDWR);
    if (-1 == bmp) {
        perror("open bmp error");
        return -1;
    }

    // 获取宽度
    lseek(bmp, 0x12, SEEK_SET);
    read(bmp, &width, 4);

    // 获取高度
    lseek(bmp, 0x16, SEEK_SET);
    read(bmp, &height, 4);

    // 获取深度，0x1c，占用两个字节
    lseek(bmp, 0x1c, SEEK_SET);
    read(bmp, &depth, 2);

    // 图片像素信息占用 宽度 * 高度 * 深度 / 8 字节
    // 获取像素数据偏移地址，0x36 为像素数据的起始位置
    lseek(bmp, 0x36, SEEK_SET);

    int buchong = (4 - (width * depth / 8 % 4)) % 4;
    char buf[(width * depth / 8 + buchong) * height];
    read(bmp, buf, sizeof(buf));

    // 打开帧缓冲设备
    int fd = open("/dev/fb0", O_RDWR);
    if (fd == -1) {
        perror("open framebuffer error");
        close(bmp);
        return -1;
    }

    // 获取帧缓冲映射到内存
    int *plcd = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (plcd == MAP_FAILED) {
        perror("mmap error");
        close(fd);
        close(bmp);
        return -1;
    }

    // 遍历像素数组，绘制到屏幕上
    int n = 0, A, R, G, B;
    int x, y;
    int color;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            B = buf[n++];
            G = buf[n++];
            R = buf[n++];
            if (depth == 32) {
                A = buf[n++];
            } else {
                A = 0;
            }
            color = B | (G << 8) | (R << 16) | (A << 24);
            if (x >= 0 && x < 800 && y >= 0 && y < 480) {
                *(plcd + 800 * (479 - y) + x) = color; // 翻转Y轴适配屏幕
            }
        }
        n += buchong; // 补充每行像素数据的对齐字节
    }

    // 关闭文件描述符，解除映射
    munmap(plcd, 800 * 480 * 4);
    close(fd);
    close(bmp);
    sleep(5);
    main();

    return 0;
}


//arm-linux-gcc s.c font.c truetype.c -o sc5 -lm -I /home/china/sqlite3_arm/include/ -L /home/china/sqlite3_arm/lib/ -lsqlite3

int main()
{
    // 打开 LCD 设备
    int lcd_fd = open(LCD_PATH, O_RDWR);
    if (lcd_fd == -1) {
        perror("Failed to open LCD device");
        return -1;
    }

    int *lcd = mmap(NULL, SCREEN_WIDTH * SCREEN_HEIGHT * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if (lcd == MAP_FAILED) {
        perror("Failed to mmap LCD");
        close(lcd_fd);
        return -1;
    }

    // 初始化字体
    font *f = fontLoad("123.ttf");
    if (f == NULL) {
        printf("Failed to load font.\n");
        munmap(lcd, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
        close(lcd_fd);
        return -1;
    }

    fontSetSize(f, 24);
    char str[100] = "";
    char st[100] = "";

    display_bmp_scaled("login.bmp");

    int x = 0, y = 0;

    while (1)
    {
        get_touch(&x, &y);
        printf("Touch location: x=%d, y=%d\n", x, y);
        char *region_name = detect_region(x, y);
        if (region_name)
        {
            printf("Touched region: %s\n", region_name);
        }
        else
        {
            printf("Touched region: None\n");
        }

        if (region_name)
        {
            printf("Detected region: %s\n", region_name);

            if (is_region_equal(region_name, "Account"))
            {
                printf("Touched Account region\n");
                while (1)
                {
                    get_touch(&x, &y);
                    region_name = detect_region(x, y);
                    if (region_name) {
                        printf("Touched region: %s\n", region_name);
                    } else {
                        printf("Touched region: None\n");
                    }

                    display_text_on_image(f, lcd, 50, 0, str);

                    if (is_region_equal(region_name, "0")) {
                        safe_add(str, '0');
                    } else if (is_region_equal(region_name, "1")) {
                        safe_add(str, '1');
                    } else if (is_region_equal(region_name, "2")) {
                        safe_add(str, '2');
                    } else if (is_region_equal(region_name, "3")) {
                        safe_add(str, '3');
                    } else if (is_region_equal(region_name, "4")) {
                        safe_add(str, '4');
                    } else if (is_region_equal(region_name, "5")) {
                        safe_add(str, '5');
                    } else if (is_region_equal(region_name, "6")) {
                        safe_add(str, '6');
                    } else if (is_region_equal(region_name, "7")) {
                        safe_add(str, '7');
                    } else if (is_region_equal(region_name, "8")) {
                        safe_add(str, '8');
                    } else if (is_region_equal(region_name, "9")) {
                        safe_add(str, '9');
                    }
                    else if (is_region_equal(region_name, "con")) {
                        break;
                    }
                    else if (is_region_equal(region_name, "delete")) {
                        del(str);
                        de(f, lcd, 50, 0, str);
                        display_bmp_scaled("login.bmp");
                    }

                    display_text_on_image(f, lcd, 50, 0, str);
                    display_text_on_image(f, lcd, 459, 0, st);
                }
            }
            else if (is_region_equal(region_name, "Password"))
            {
                printf("Touched Password region\n");
                while (1)
                {
                    get_touch(&x, &y);
                    region_name = detect_region(x, y);
                    if (region_name) {
                        printf("Touched region: %s\n", region_name);
                    } else {
                        printf("Touched region: None\n");
                    }

                    display_text_on_image(f, lcd, 459, 0, st);

                    if (is_region_equal(region_name, "0")) {
                        safe_add(st, '0');
                    } else if (is_region_equal(region_name, "1")) {
                        safe_add(st, '1');
                    } else if (is_region_equal(region_name, "2")) {
                        safe_add(st, '2');
                    } else if (is_region_equal(region_name, "3")) {
                        safe_add(st, '3');
                    } else if (is_region_equal(region_name, "4")) {
                        safe_add(st, '4');
                    } else if (is_region_equal(region_name, "5")) {
                        safe_add(st, '5');
                    } else if (is_region_equal(region_name, "6")) {
                        safe_add(st, '6');
                    } else if (is_region_equal(region_name, "7")) {
                        safe_add(st, '7');
                    } else if (is_region_equal(region_name, "8")) {
                        safe_add(st, '8');
                    } else if (is_region_equal(region_name, "9")) {
                        safe_add(st, '9');
                    }
                    else if (is_region_equal(region_name, "con")) {
                        break;
                    }
                    else if (is_region_equal(region_name, "delete")) {
                        del(st);
                        de(f, lcd, 459, 0, st);
                        display_bmp_scaled("login.bmp");
                    }

                    display_text_on_image(f, lcd, 50, 0, str);
                    display_text_on_image(f, lcd, 459, 0, st);
                }
            }
            else if (is_region_equal(region_name, "Login"))
            {
                printf("Touched Login region\n");

                sqlite3 *sqlite3_db = NULL;
                int ret = sqlite3_open("contacts.db", &sqlite3_db);
                if (ret != SQLITE_OK)
                {
                    printf("Failed to open database: %s\n", sqlite3_errmsg(sqlite3_db));
                    return -1;
                }

                char *input_username = str;
                char *input_password = st;

                // 查询数据库验证用户名和密码是否匹配
                char *sql = "SELECT * FROM usr WHERE username = ? AND password = ?";
                sqlite3_stmt *stmt;
                ret = sqlite3_prepare_v2(sqlite3_db, sql, -1, &stmt, 0);
                if (ret != SQLITE_OK) {
                    printf("Failed to prepare statement: %s\n", sqlite3_errmsg(sqlite3_db));
                    sqlite3_close(sqlite3_db);
                    return -1;
                }

                sqlite3_bind_text(stmt, 1, input_username, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, input_password, -1, SQLITE_STATIC);

                ret = sqlite3_step(stmt);
                if (ret == SQLITE_ROW) {
                    printf("Login successful\n");
                } else {
                    printf("Login failed\n");
                    
                    failLogin();
                    return 0;
                }

                sqlite3_finalize(stmt);
                sqlite3_close(sqlite3_db);

                cc(f, lcd);
            }
            //else if (is_region_equal(region_name, "Register"))
            //{
                // sqlite3 *db;
                // char *errMsg = 0;
                // int ret;
                // const char *sql;

                // ret = sqlite3_open("contacts.db", &db);
                // if (ret != SQLITE_OK)
                // {
                //     printf("Failed to open database: %s\n", sqlite3_errmsg(db));
                //     return -1;
                // }

                // sql = "INSERT INTO user(username, password) VALUES(?, ?);";
                // sqlite3_stmt *stmt;
                // ret = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
                // if (ret != SQLITE_OK)
                // {
                //     printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
                //     sqlite3_close(db);
                //     return -1;
                // }

                // sqlite3_bind_text(stmt, 1, str, -1, SQLITE_STATIC);
                // sqlite3_bind_text(stmt, 2, st, -1, SQLITE_STATIC);

                // ret = sqlite3_step(stmt);
                // if (ret != SQLITE_DONE)
                // {
                //     printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
                // }
                // else
                // {
                //     printf("Register successful\n");
                // }

                // sqlite3_finalize(stmt);
                // sqlite3_close(db);

                // cc(f, lcd);
           // }
        }
        else
        {
            printf("No valid region detected\n");
        }
    }

    munmap(lcd, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    close(lcd_fd);
    fontUnload(f);
    return 0;
}
