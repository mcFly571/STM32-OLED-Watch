#include "Clock_Module.h"

CUR_CLOCK cur_clock;

void CurrentClock_Init(void) {
	cur_clock.seconds = 0;
	cur_clock.minutes = 0;
	cur_clock.hours = 0;
	
	cur_clock.days = 5;
	cur_clock.monthes = 5;
	cur_clock.years = 2026;
}

u8 Days_Of_Month_Culculate(u8 monthes){
	switch(monthes) {
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			return 31;
		case 4: case 6: case 9: case 11:
			return 30;
		case 2:
			return 28;
		default:
			return 0;	//无效的月份
	}
}
