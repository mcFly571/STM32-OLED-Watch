#ifndef WEEK_CALCULATE_H
#define WEEK_CALCULATE_H
#include "stm32f10x.h"

uint32_t date_to_days(u16 year, u8 month, u8 day);
int32_t date_to_week(u16 year, u8 month, u8 day);
#endif // WEEK_CALCULATE_H
