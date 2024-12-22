#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sqlite3.h>
// 回调函数，用于处理查询结果
int deal1(void *msg, int num, char **value, char **name) {
    int i;
    printf("查询结果:\n");
    // 输出字段名和对应的值
    for (i = 0; i < num; i++) {
        printf("[%s]: %s\n", name[i], value[i]);
    }
    printf("\n");
    return 0;
}

void insert_data1(sqlite3 *db) {
    char name[255], brand[255], purchase_date[255], storage_location[255];
    double price;
    int usage_count, maintenance_cycle;
    
    printf("请输入装备名称: ");
    scanf("%s", name);
    printf("请输入品牌: ");
    scanf("%s", brand);
    printf("请输入购买日期(YYYY-MM-DD): ");
    scanf("%s", purchase_date);
    printf("请输入价格: ");
    scanf("%lf", &price);
    printf("请输入使用次数: ");
    scanf("%d", &usage_count);
    printf("请输入保养周期(天): ");
    scanf("%d", &maintenance_cycle);
    printf("请输入存放位置: ");
    scanf("%s", storage_location);
    
    // 插入数据到sports_equipment表中
    char sql[1024];
    sprintf(sql, "INSERT INTO sports_equipment (name, brand, purchase_date, price, usage_count, maintenance_cycle, storage_location) "
                "VALUES ('%s', '%s', '%s', %.2f, %d, %d, '%s');", name, brand, purchase_date, price, usage_count, maintenance_cycle, storage_location);

    char *errmsg;
    int ret = sqlite3_exec(db, sql, 0, 0, &errmsg);
    if (ret != SQLITE_OK) {
        printf("插入数据失败: %s\n", errmsg);
        sqlite3_free(errmsg);
    } else {
        printf("插入数据成功\n");
    }
}

void query_data1(sqlite3 *db, const char *condition) {
    char sql[1024];
    sprintf(sql, "SELECT * FROM sports_equipment WHERE %s;", condition);

    char *errmsg;
    int ret = sqlite3_exec(db, sql, deal1, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        printf("查询失败: %s\n", errmsg);
        sqlite3_free(errmsg);
    }
}

void update_data1(sqlite3 *db) {
    char old_name[255];
    char new_name[255], brand[255], purchase_date[255], storage_location[255];
    double price;
    int usage_count, maintenance_cycle;
    char sql[1024];
    
    // 输入要更新的装备名称
    printf("请输入要更新的装备的名称: ");
    scanf("%s", old_name);

    // 根据装备名称查询当前的装备信息
    char query_sql[1024];
    sprintf(query_sql, "SELECT * FROM sports_equipment WHERE name = '%s';", old_name);

    char *errmsg;
    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        printf("查询失败: %s\n", sqlite3_errmsg(db));
        return;
    }

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW) {
        printf("未找到该装备!\n");
        sqlite3_finalize(stmt);
        return;
    }

    // 获取现有数据
    const char *current_name = (const char *)sqlite3_column_text(stmt, 0);
    const char *current_brand = (const char *)sqlite3_column_text(stmt, 1);
    const char *current_purchase_date = (const char *)sqlite3_column_text(stmt, 2);
    double current_price = sqlite3_column_double(stmt, 3);
    int current_usage_count = sqlite3_column_int(stmt, 4);
    int current_maintenance_cycle = sqlite3_column_int(stmt, 5);
    const char *current_storage_location = (const char *)sqlite3_column_text(stmt, 6);

    printf("当前信息: \n");
    printf("名称: %s\n", current_name);
    printf("品牌: %s\n", current_brand);
    printf("购买日期: %s\n", current_purchase_date);
    printf("价格: %.2f\n", current_price);
    printf("使用次数: %d\n", current_usage_count);
    printf("保养周期: %d\n", current_maintenance_cycle);
    printf("存放位置: %s\n", current_storage_location);
    
    // 用户输入新的信息
    printf("请输入新的装备名称: ");
    scanf("%s", new_name);
    printf("请输入新的品牌: ");
    scanf("%s", brand);
    printf("请输入新的购买日期(YYYY-MM-DD): ");
    scanf("%s", purchase_date);
    printf("请输入新的价格: ");
    scanf("%lf", &price);
    printf("请输入新的使用次数: ");
    scanf("%d", &usage_count);
    printf("请输入新的保养周期(天): ");
    scanf("%d", &maintenance_cycle);
    printf("请输入新的存放位置: ");
    scanf("%s", storage_location);

    // 更新数据到表中
    sprintf(sql, "UPDATE sports_equipment SET name = '%s', brand = '%s', purchase_date = '%s', price = %.2f, "
                "usage_count = %d, maintenance_cycle = %d, storage_location = '%s' WHERE name = '%s';",
            new_name, brand, purchase_date, price, usage_count, maintenance_cycle, storage_location, old_name);

    ret = sqlite3_exec(db, sql, 0, 0, &errmsg);
    if (ret != SQLITE_OK) {
        printf("更新数据失败: %s\n", errmsg);
        sqlite3_free(errmsg);
    } else {
        printf("更新数据成功\n");
    }

    sqlite3_finalize(stmt);
}