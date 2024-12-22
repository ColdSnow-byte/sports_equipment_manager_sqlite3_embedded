#ifndef button
#define button
// 定义区域的边界结构体
typedef struct
{
    int x;
    int y;
    int w;
    int h;
    char *name;
} Region;

// 定义各区域
Region regions[] = {
    {50, 0, 350, 37, "Account"},
    {459, 0, 341, 37, "Password"},
    {49, 73, 115, 72, "Login"},
    //{239, 73, 116, 72, "Register"},
    {597, 80, 709, 143, "delete"},
    {239, 73, 531, 147, "con"},
    
    {22, 183, 131, 103, "1"},
    {166, 183, 126, 103, "2"},
    {310, 183, 125, 103, "3"},
    {446, 183, 127, 103, "4"},
    {590, 183, 131, 103, "5"},
    {22, 319, 131, 103, "6"},
    {166, 319, 126, 103, "7"},
    {310, 319, 125, 103, "8"},
    {446, 319, 127, 103, "9"},
    {590, 319, 131, 103, "0"} };
Region db[] = {

    {0,115,245,170,"按名称查询"},
    
    {532,115,800,170,"按使用次数查询"},
    {250,115,530,170,"按品牌查询"},
    //我们的点
    {0,69,238,112,"录入"},
    {530,68,799,112,"更新"},
    {240,67,525,112,"查询"},
    {190,355,387,480,"推荐"},
    {32,9,158,56,"删除"},
    {0,443,190,480,"所有装备"},
    {388,443,800,480,"平均"}

    };

#define REGION_COUNT (sizeof(regions) / sizeof(Region))
// 判断触摸点是否在区域内
char *detect_region(int x, int y)
{
    for (int i = 0; i < REGION_COUNT; i++)
    {
        Region region = regions[i];
        if (x >= region.x && x <= region.x + region.w && y >= region.y && y <= region.y + region.h)
        {
            return region.name;
        }
    }
    return NULL;
}

// 判断触摸点是否在区域内
char *dbn(int x, int y)
{
    for (int i = 0; i < REGION_COUNT; i++)
    {
        Region region = db[i];
        if (x >= region.x && x <= region.x + region.w && y >= region.y && y <= region.y + region.h)
        {
            return region.name;
        }
    }
    return NULL;
}
// 检查区域名称是否匹配
int is_region_equal(const char *region_name, const char *target_name)
{
    if (region_name == NULL || target_name == NULL)
    {
        return 0; // NULL 检查
    }
    return strcmp(region_name, target_name) == 0;
}

#endif
// int main() {
//     int x = 0, y = 0;

//     while (1) {
//         get_touch(&x, &y);

//         printf("Touch location: x=%d, y=%d\n", x, y);

//         char *region_name = detect_region(x, y);
//         if (region_name) {
//             printf("Touched region: %s\n", region_name);
//         } else {
//             printf("Touched region: None\n");
//         }
//     }

//     return 0;
// }
