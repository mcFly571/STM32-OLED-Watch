#include "Week_Calculate.h"
#include "Timer.h"
//将日期转换为总天数
uint32_t date_to_days(u16 year, u8 month, u8 day) {
    u16 days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if(is_leap_year(year)) {
        days_in_month[2] = 29; // 闰年2月有29天
    }
    uint32_t y = (uint32_t)(year - 1); // 从1年开始计算

    /* 
    假设每年都是365天，先算个基础值。
    每4年有一个闰年（多1天），所以加上 y/4 个闰年。+ y / 4
    但是，整百年（100、200、300……）不是闰年，上一步多加了，要减掉。- y / 100
    但是，整400年（400、800、1200……）又是闰年，上一步减多了，要加回来。+ y / 400
    */
    uint32_t total = y * 365 + (y / 4) - (y / 100) + (y / 400); 

    // 加上当前年已经过的天数
    for (u8 i = 1; i < month; i++) {
        total += days_in_month[i];
    }
    
    total += day;
    return total;
}

int32_t date_to_week(u16 year, u8 month, u8 day) {
    /* 以 2000-01-01（星期六）为基准，
    计算目标日期与基准日期的天数差，然后取模7得到星期几 */
    int32_t base_day = (int32_t)date_to_days(2000, 1, 1);
    int32_t target_day = (int32_t)date_to_days(year, month, day);
    int32_t diff = target_day - base_day;

    /* +6 是因为 2000-01-01 是星期六（对应数字6），调整为以星期一为1的系统 */
    int32_t w = ((diff + 6) % 7 + 7) % 7;  
    return (w == 0) ? 7 : w;               // 变成 1=周一...7=周日
}   
