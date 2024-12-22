#ifndef ad
#define ad
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include "bmp.h"
#include "touch.h"
#include "button.h"
#include "ttf.h"
#include "sport.h"
#include "recommend.h"

int display_query_results(sqlite3_stmt *stmt, font *f, int *lcd, int max_rows)
{
    int row = 0;
    int y_start = 150; // 起始y坐标
    int y_step = 34;   // 每行间隔，包括数据高度和间距
    int a;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (row >= max_rows)
            break; // 超出最大显示行数时停止

        char buffer[200];

        // 获取每列数据（确保列数与数据库匹配）
        const char *name = (const char *)sqlite3_column_text(stmt, 0);     // 姓名
        const char *title = (const char *)sqlite3_column_text(stmt, 1);    // 称谓
        const char *phone = (const char *)sqlite3_column_text(stmt, 2);    // 电话
        const char *address = (const char *)sqlite3_column_text(stmt, 3);  // 地址
        const char *mod_time = (const char *)sqlite3_column_text(stmt, 4); // 修改时间

        // 绘制到屏幕指定区域
        snprintf(buffer, sizeof(buffer), "%s", name ? name : "N/A");
        display_text_on_image(f, lcd, 3, y_start + row * y_step, buffer); // 姓名

        snprintf(buffer, sizeof(buffer), "%s", title ? title : "N/A");
        display_text_on_image(f, lcd, 96, y_start + row * y_step, buffer); // 称谓

        snprintf(buffer, sizeof(buffer), "%s", phone ? phone : "N/A");
        display_text_on_image(f, lcd, 198, y_start + row * y_step, buffer); // 电话

        snprintf(buffer, sizeof(buffer), "%s", address ? address : "N/A");
        display_text_on_image(f, lcd, 412, y_start + row * y_step, buffer); // 地址

        snprintf(buffer, sizeof(buffer), "%s", mod_time ? mod_time : "N/A");
        display_text_on_image(f, lcd, 544, y_start + row * y_step, buffer); // 修改时间

        row++; // 下一行
    }

    printf("已显示 %d 行记录，5秒后继续...\n", row);
    sleep(5); // 暂停显示

    return row; // 返回已显示行数
}
//文字显示到屏幕上
void display_contact_data(font *f, int *lcd, sqlite3 *db, const char *sql)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    int a;
    // 定义各字段在屏幕上的 x 坐标
    int x_positions[] = {0, 87, 170, 300, 459,595,710}; // 各字段的 x 坐标
    int y_start = 256;                          // 起始 y 坐标
    int y_offset = 283-229;                          // 每行记录的 y 偏移量

    int row = 0; // 当前行号
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // 获取每列的数据
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        const char *title = (const char *)sqlite3_column_text(stmt, 1);
        const char *phone = (const char *)sqlite3_column_text(stmt, 2);
        const char *address = (const char *)sqlite3_column_text(stmt, 3);
        const char *last_modified = (const char *)sqlite3_column_text(stmt, 4);
        const char *last_modified1 = (const char *)sqlite3_column_text(stmt, 5);
        const char *last_modified2 = (const char *)sqlite3_column_text(stmt, 6);

        // 将列数据存储到数组中
        const char *fields[] = {name, title, phone, address, last_modified,last_modified1,last_modified2};

        // 绘制每列的数据到屏幕
        for (int col = 0; col < 7; col++)
        {
            char text_buffer[256]; // 定义临时缓冲区存储显示文本
            snprintf(text_buffer, sizeof(text_buffer), "%s", fields[col] ? fields[col] : "N/A");
            int x = x_positions[col];
            int y = y_start + row * y_offset;

            display_text_on_image(f, lcd, x, y, text_buffer);
        }

        row++; // 增加行号
        if (row >= 10)
            break; // 最多显示 10 行记录
    }
    printf("已显示 %d 行记录，5秒后继续...\n", row);
    sleep(5);            // 暂停显示
    sqlite3_finalize(stmt); // 释放语句对象
}

// 错误处理回调函数
static int callback(void *data, int argc, char **argv, char **col_name)
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(col_name[i], "first_name") == 0)
        {
            printf("%-15s ", argv[i] ? argv[i] : "无");
        }
        else if (strcmp(col_name[i], "phone") == 0)
        {
            printf("%-15s ", argv[i] ? argv[i] : "无");
        }
        else if (strcmp(col_name[i], "address") == 0)
        {
            printf(" %-15s ", argv[i] ? argv[i] : "无");
        }
        else if (strcmp(col_name[i], "relationship") == 0)
        {
            printf(" %-10s ", argv[i] ? argv[i] : "无");
        }
        else if (strcmp(col_name[i], "updated_at") == 0)
        {
            printf(" %-18s\n", argv[i] ? argv[i] : "无");
        }
    }
    return 0;
}

// 获取当前时间
const char *get_current_time()
{
    time_t t;
    struct tm *tm_info;
    static char buffer[26]; // 用于存储当前时间字符串

    time(&t);
    tm_info = localtime(&t);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info); // 格式化为 YYYY-MM-DD HH:MM:SS

    return buffer;
}


void delete_data(sqlite3 *db) {
    char name[255];
    printf("请输入要删除的装备名称: ");
    scanf("%s", name);

    // 开始事务
    char *errmsg;
    int ret = sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, &errmsg);
    if (ret != SQLITE_OK) {
        printf("开始事务失败: %s\n", errmsg);
        return;
    }

    // 删除对应名称的装备
    char sql[1024];
    sprintf(sql, "DELETE FROM sports_equipment WHERE name = ?;");

    sqlite3_stmt *stmt;
    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        printf("SQL准备失败: %s\n", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", 0, 0, &errmsg);  // 回滚事务
        return;
    }

    // 绑定参数
    ret = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK) {
        printf("绑定失败: %s\n", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", 0, 0, &errmsg);  // 回滚事务
        sqlite3_finalize(stmt);
        return;
    }

    // 执行删除操作
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        printf("删除数据失败: %s\n", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", 0, 0, &errmsg);  // 回滚事务
        sqlite3_finalize(stmt);
        return;
    }

    // 提交事务
    ret = sqlite3_exec(db, "COMMIT;", 0, 0, &errmsg);
    if (ret != SQLITE_OK) {
        printf("提交事务失败: %s\n", errmsg);
        sqlite3_free(errmsg);
    } else {
        printf("删除数据成功\n");
    }

    sqlite3_finalize(stmt);
}



void query_contact(sqlite3 *db, font *f, int *lcd)
{
    char sql[500];
    int rc;
    int x = 0, y = 0;
    display_bmp_scaled("search.bmp"); // 显示主界面背景

    while (1)
    {
        get_touch(&x, &y); // 获取触摸坐标
        printf("Touch location: x=%d, y=%d\n", x, y);
        char *region_name = dbn(x, y); // 判断触摸区域
        if (!region_name)
        {
            printf("未检测到有效区域，请重试。\n");
            continue;
        }

        printf("检测到的区域名称: %s\n", region_name);

        // if (is_region_equal(region_name, "all"))
        // {
        //     snprintf(sql, sizeof(sql), "SELECT * FROM sports_equipment;");
        //     break;
        // }
        // else 
        if (is_region_equal(region_name, "按名称查询"))
        {
            char name[200];
            printf("请输入名称：");

            scanf("%s",name);
            snprintf(sql, sizeof(sql), "SELECT * FROM sports_equipment WHERE name ='%s';", name);
            break;
        }
        else if(is_region_equal(region_name, "按使用次数查询")){
            char usage_count[20];
            printf("请输入使用次数：");
            scanf("%s", usage_count);
            snprintf(sql, sizeof(sql), "SELECT * FROM sports_equipment WHERE usage_count = '%s';", usage_count);
            break;
        }
        else if (is_region_equal(region_name, "按品牌查询"))
        {
            char brand[20];
            printf("请输入品牌：");
            scanf("%s", brand);
            snprintf(sql, sizeof(sql), "SELECT * FROM sports_equipment WHERE brand = '%s';", brand);
            break;
        }
        else if(is_region_equal(region_name, "所有装备")){
            snprintf(sql, sizeof(sql), "SELECT * FROM sports_equipment;");
            break;
        }
        // else if(is_region_equal(region_name, "平均")){
        //     calculate_average_usage_count(db);
        //snprintf(sql, sizeof(sql), "SELECT * FROM sports_equipment;");
        //     break;
        // }
        else if(is_region_equal(region_name, "平均")){
        display_bmp_scaled("b3.bmp");
        calculate_average_usage_count(db, f, lcd); // 显示到屏幕上
        break;
        }
        else
        {
            printf("无效的区域，请重试。\n");
        }
    }
    rc = sqlite3_exec(db, sql, callback, 0, NULL);
    if (rc != SQLITE_OK)
    {
        printf("查询失败: %s\n", sqlite3_errmsg(db));
    }
    else
    {
        // 将查询结果显示到触摸屏
        display_contact_data(f, lcd, db, sql);
    }
}



void cc(font *f, int *lcd)
{
    sqlite3 *db;
    char *errMsg = 0;
    int rc;

    // 打开数据库（如果文件不存在则会创建）
    rc = sqlite3_open("sports_equipment.db", &db);
    if (rc)
    {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
    }
    else
    {
        printf("数据库成功打开\n");
    }

 

    int choice, x = 0, y = 0;
    while (1)
    {
        display_bmp_scaled("maintain.bmp"); // 主进程显示图片
        get_touch(&x, &y);
        printf("Touch location: x=%d, y=%d\n", x, y);
        char *region_name = dbn(x, y);
        if (region_name)
        {
            printf("Touched region: %s\n", region_name);
        }
        else
        {
            printf("Touched region: None\n");
        }

        printf("检测到的区域名称: %s\n", region_name);
        // 判断是否与某个按键相等
        if (is_region_equal(region_name, "录入"))
        {

            insert_data1(db);
        }
        else if (is_region_equal(region_name, "查询"))
        {
            query_contact(db, f, lcd);
        }
        else if (is_region_equal(region_name, "更新"))
        {
            update_data1(db);
        }
        else if(is_region_equal(region_name, "删除")){
            delete_data(db);
        }
        else
        {
            printf("未检测到有效区域\n");
        }
    }

    sqlite3_close(db);
}

#endif