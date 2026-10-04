#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"
#include "Clock_Module.h"

#define RTC_INIT_FLAG  0xA5A5

void RTC_LSE_init(void);
void RTC_NVIC_Init(void);
void RTC_Clock_Init(CUR_CLOCK *init_clock);

u32 clock_to_timestamp(CUR_CLOCK *pCur_clock);
void timestamp_to_clock(u32 timestamp, CUR_CLOCK *pCur_clock);
void RTC_GetClock(CUR_CLOCK *pCur_clock);
void RTC_SetClock(CUR_CLOCK *pCur_clock);
u8 is_leap_year(u16 year);

extern volatile u32 g_RTC_Count;
//extern volatile u8 save_time_flag;
extern volatile uint8_t g_dht22_read_flag;  // 需要读取 DHT
#endif

