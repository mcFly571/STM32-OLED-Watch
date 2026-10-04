#include "stm32f10x.h"
#include "AM2302_DHT22.h"
// SysTick 每 1ms 中断一次，这个变量在中断里累加
// 注意：这个变量要和 stm32f10x_it.c 里的 g_dht22_tick 共用
// 所以 Delay_ms 直接用循环实现即可，不再依赖 SysTick 配置

void Delay_us(uint32_t xus)
{
    // 直接调用 AM2302 里的 delay_us，保持一致
    extern void delay_us(uint32_t us);
    delay_us(xus);
}

void Delay_ms(uint32_t xms)
{
    while (xms--) {
        delay_us(1000);
    }
}

void Delay_s(uint32_t xs)
{
    while (xs--) {
        Delay_ms(1000);
    }
}
