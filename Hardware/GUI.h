#ifndef	__GUI_H
#define __GUI_H

#include "stm32f10x.h"
#include "Clock_Module.h"
#define NULL ((void *)0)

struct Menu{
    char *name;
    void (*action)(void);
    //struct Menu *subMenus;	//子菜单
    uint8_t subMenuCount;
};

enum {
	MENU_CLOCK = 0,
	MENU_FUNCTION,
    SON_MENU_SET_DATE,
    SON_MENU_SET_TIME,
    SON_MENU_TIMER
};

typedef enum {
    FIELD_YEAR = 0,
    FIELD_MONTH,
    FIELD_DAY,
    FIELD_HOUR,
    FIELD_MINUTE,
    FIELD_SECOND
} DateTimeField;

void Main_UI(void);
void Func_UI(void);
// void Others_UI(u8 mainIndex, u8 sonIndex);
void Menu_Display(void);
void Func_Set_Date(void);
void Func_Set_Time(void);
void Func_Timer(void);
void Key_Handler(void);

void Key_Event_Handler(void);

//void Func_KeyHandler(void);
extern struct Menu menu_UI[];
extern u8 mainCurIndex;
extern u8 sonCurIndex;
extern u8 stateUI;
extern u8 timer_run_flag;
extern u32 temp_timer_temp_second;
extern CUR_CLOCK temp_timer_clock;

extern uint8_t battery_percent;	//电池电压百分比

extern volatile u8 g_time_editing;
//extern GUI main_GUI;
//extern Menu menu_UI[];

#endif

