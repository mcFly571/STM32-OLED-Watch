#ifndef __CLOCK_MODULE_H
#define __CLOCK_MODULE_H

#include "stm32f10x.h"

#define Advance_Sixty(time) ( (time == 60) ? (1) : (0) )

typedef struct {
	u8 seconds;
	u8 minutes;
	u8 hours;
	
	u8 days;
	u8 monthes;
	u16 years;
}CUR_CLOCK;

extern CUR_CLOCK cur_clock;
void CurrentClock_Init(void);
u8 Days_Of_Month_Culculate(u8 monthes);

#endif

