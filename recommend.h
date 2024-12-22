
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include <sys/mman.h>
#include <string.h>
#include "bmp.h"
#include "touch.h"
#include "button.h"
#include "ttf.h"
//snprintf(display_text, sizeof(display_text), "     %s                %d                               %.2f", brand, total_usage_count, avg_usage_count);



char* recommend_brand_based_on_usage(sqlite3 *db) {
    char sql[1024];
    // 查询每个品牌的平均使用次数
    sprintf(sql, "SELECT brand, AVG(usage_count) AS avg_usage_count "
                 "FROM sports_equipment GROUP BY brand ORDER BY avg_usage_count DESC LIMIT 1;");

    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        printf("查询失败: %s\n", sqlite3_errmsg(db));
        return;
    }

    ret = sqlite3_step(stmt);
    if (ret == SQLITE_ROW) {
        const char *brand = (const char *)sqlite3_column_text(stmt, 0);
        double avg_usage_count = sqlite3_column_double(stmt, 1);
        printf("推荐品牌: %s，平均使用次数: %.2f 次\n", brand, avg_usage_count);
        return brand;
    } else {
        printf("未找到任何品牌数据。\n");
        return NULL;
    }

    sqlite3_finalize(stmt);
    
}


// 平均使用时长查询，显示到屏幕

void calculate_average_usage_count(sqlite3 *db, font *f, int *lcd) {
    char sql[1024];
    // 查询每个品牌的总使用次数和平均使用次数
    sprintf(sql, "SELECT brand, SUM(usage_count) AS total_usage_count, AVG(usage_count) AS avg_usage_count "
                 "FROM sports_equipment GROUP BY brand;");

    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        printf("查询失败: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("不同品牌运动装备的使用统计信息:\n");

    int row = 0;
    int y_start = 150;  // 显示结果的起始位置
    int y_offset = 80;  // 每行的垂直间距

    char *recommended_brand = NULL;
    double max_avg_usage_count = 0.0;
    

    // 获取查询结果并显示到屏幕
    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
        char *brand = (char *)sqlite3_column_text(stmt, 0);
        int total_usage_count = sqlite3_column_int(stmt, 1);
        double avg_usage_count = sqlite3_column_double(stmt, 2);


        // 打印到控制台
        printf("品牌: %s, 总使用次数: %d 次, 平均使用次数: %.2f 次\n", brand, total_usage_count, avg_usage_count);

        // 构建显示文本
        char display_text[256];
        snprintf(display_text, sizeof(display_text), "     %s                %d                    %.2f", brand, total_usage_count, avg_usage_count);

        // 将每一行数据显示到屏幕上
        display_text_on_image(f, lcd, 10, y_start + row * y_offset, display_text);
        row++;

        if (row >= 3) // 最多显示10行
            break;
        
        //char recommendation_text_next[2];
        //strncpy(recommendation_text_next,recommendation_text,2);
    }
        int x=0,y=0;

        while (1)
        {
            /* code */
            get_touch(&x, &y);
            printf("Touch location: x=%d, y=%d\n", x, y);
            char *region_name11 = dbn(x, y); // 判断触摸区域
            if (!region_name11)
            {
            printf("未检测到有效区域，请重试。\n");
            continue;
            }
            printf("检测到的区域名称: %s\n", region_name11);
            if (is_region_equal(region_name11, "推荐"))
            {
            char recommendation_text[256];
            char *ptemp=recommend_brand_based_on_usage(db);
            snprintf(recommendation_text, sizeof(recommendation_text), "%s", ptemp);
            display_text_on_image(f, lcd, 450, 420, recommendation_text);
            break;
        
            //display_text_on_image(f, lcd, 450, 420, recommendation_text);
    }
        

        
        //printf("推荐品牌: %s, 平均使用次数: %.2f\n", recommended_brand);
        //}
        
    }

    if (ret != SQLITE_DONE) {
        printf("查询失败: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);


    
}
