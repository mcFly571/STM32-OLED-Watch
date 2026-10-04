#include "GUI.h"
#include "Key.h"
#include "OLED.h"
#include "Clock_Module.h"
#include "Delay.h"
#include "Week_Calculate.h"
#include "ADC_Volt.h"
#include "Timer.h"
//#include "AM2302_DHT22.h"
//#include "Reserve_Flash.h"
//#include "State_Machine.h"
uint8_t battery_percent = 0;	//电池电压百分比

struct Menu mainMenu[] = {
	//主菜单
	{NULL, Main_UI, 0},					//0
	{"Setting", Func_UI, 0},			//1
	//子菜单
	{"Set Date", Func_Set_Date, 0},		//2
	{"Set Time", Func_Set_Time, 0},		//3
	{"Timer", Func_Timer, 0}			//4
};

struct Menu *curMenu = mainMenu;	//当前菜单指针
u8 curSonMenuCount = 3;
u8 menuLevel = 0;	//current menu level
u8 selectedIndex = 0;	// 选项索引
u8 preMenuLevel = 0;	//上一个界面状态
u8 sonMenuConfirmFlag = 0;		// 确认按键标志位	

u8 miniSelectedIndex = 0;
u8 miniSelectNum = 2;
u8 miniCurIndex = 0;	//当前子菜单指针

static u8 lockFlag = 0;              // 0=未锁定(选方向) 1=已锁定(可连发)
static OperationDirection lockedDir = DIR_ADD;

volatile u8 g_time_editing;

void Menu_Display(void) 
{
	curMenu[menuLevel].action();	//调用当前菜单的显示函数
}

static void switch_operate(KeyEvent e)
{
    if(e == KEY_LONG_PRESS)
    {
        menuLevel = !menuLevel;
        OLED_Clear();
    }
}

static void back_operate(KeyEvent e)
{
    if(e == KEY_LONG_PRESS) {
        if (g_time_editing) {
            RTC_SetClock(&cur_clock);   // ★ 提交修改到硬件RTC
        }
        if(menuLevel >= 1 && (preMenuLevel != menuLevel))
        {
            menuLevel = preMenuLevel;
        } else {
            menuLevel = 0;
        }
        OLED_Clear();
        preMenuLevel = 0;
        sonMenuConfirmFlag = 0;
        g_time_editing = 0;      // ★ 结束编辑
    }
}

static void confirm_operate(KeyEvent e)
{
    if (e == KEY_PRESS) {
        sonMenuConfirmFlag = 1;
        preMenuLevel = menuLevel;
        menuLevel = selectedIndex + 2;
        //miniCurIndex = 0;
        lockFlag = 0;
        g_time_editing = 1;      // ★ 开始编辑
        OLED_Clear();
    }
}

static void down_index_operate(KeyEvent e)
{
	if(e == KEY_PRESS) {
		if(sonMenuConfirmFlag == 1)
			return;	//如果已经在子菜单中，则不再处理

		if(selectedIndex >= curSonMenuCount - 1) {
			selectedIndex = 0;
		}
		else {
			selectedIndex++;
		}
	}
}

static void adjust_field(volatile CUR_CLOCK *clk, DateTimeField field, OperationDirection dir)
{
    int8_t delta = (dir == DIR_ADD) ? 1 : -1;

    switch (field) {
        case FIELD_YEAR:
            clk->years += delta;
            if (clk->years < 1970) clk->years = 1970;
            break;
        case FIELD_MONTH:
            clk->monthes += delta;
            if (clk->monthes > 12) { clk->monthes = 1; clk->years++; }
            if (clk->monthes < 1)  { clk->monthes = 12; clk->years--; }
            if (clk->days > Days_Of_Month_Culculate(clk->monthes))
                clk->days = Days_Of_Month_Culculate(clk->monthes);
            break;
        case FIELD_DAY:
            clk->days += delta;
            if (clk->days > Days_Of_Month_Culculate(clk->monthes)) {
                clk->days = 1; clk->monthes++;
                if (clk->monthes > 12) { clk->monthes = 1; clk->years++; }
            }
            if (clk->days < 1) {
                clk->monthes--;
                if (clk->monthes < 1) { clk->monthes = 12; clk->years--; }
                clk->days = Days_Of_Month_Culculate(clk->monthes);
            }
            break;
        case FIELD_HOUR:
            clk->hours += delta;
            if (clk->hours >= 24 || clk->hours > 200) clk->hours = 0; // >200 是 u8 下溢保护
            break;
        case FIELD_MINUTE:
            clk->minutes += delta;
            if (clk->minutes >= 60 || clk->minutes > 200) {
				clk->hours++;
				clk->minutes = 0;
				if(clk->hours >= 24 || clk->hours > 200) {clk->hours = 0;clk->days++;}
			}
			if(clk->minutes <= 0) {
				clk->minutes = 59;
				clk->hours--;
				if(clk->hours <= 0 || clk->hours > 200) {clk->hours = 23;clk->days--;}
			}
            break;
        case FIELD_SECOND:
            clk->seconds += delta;
            if (clk->seconds >= 60 || clk->seconds > 200) clk->seconds = 0;
            break;
    }
}

static void adjust_operate(KeyEvent e_pa6, KeyEvent e_pb12, volatile CUR_CLOCK *clk, DateTimeField field)
{
    CUR_CLOCK snapshot;
    __disable_irq();
    snapshot = *clk;
	//OLED_ShowNum(1, 10, clk->days, 2);
    __enable_irq();

    if (!lockFlag) {
        // 未锁定：PB12 短按移动指针
        if (e_pb12 == KEY_PRESS) {
            miniCurIndex = (miniCurIndex + 1) % miniSelectNum;
        }
        // PA6 短按：锁定当前方向
        if (e_pa6 == KEY_PRESS || e_pa6 == KEY_LONG_PRESS) {
    		lockFlag  = 1;
    		lockedDir = (miniCurIndex == 0) ? DIR_ADD : DIR_DECREASE;
    		adjust_field(&snapshot, field, lockedDir);
		}
    } else {
        // 已锁定：PA6 单击 = +1一次，长按/连发 = 持续调整
        if (e_pa6 == KEY_PRESS || e_pa6 == KEY_LONG_PRESS || e_pa6 == KEY_AUTO_INC) {
        	adjust_field(&snapshot, field, lockedDir);
    	}
        // PB12 长按：退出锁定
        if (e_pb12 == KEY_LONG_PRESS) {
            lockFlag = 0;
        }
    }

    // 显示
    OLED_ShowString(1, 3, "adjust+");
    OLED_ShowString(2, 3, "adjust-");
    OLED_ShowString(1, 1, (miniCurIndex == 0) ? ">" : " ");
    OLED_ShowString(2, 1, (miniCurIndex == 1) ? ">" : " ");
    OLED_ShowString(4, 3, lockFlag ? "[LOCKED]" : "[UNLOCK]");

    __disable_irq();
    *clk = snapshot;
    __enable_irq();
}

static void count_and_refresh_operate(KeyEvent e_pa6, KeyEvent e_pb12)
{
	if (e_pa6 == KEY_PRESS) {
        timer_run_flag = !timer_run_flag;   // 开始/暂停 切换
    }
    if (e_pb12 == KEY_PRESS && !timer_run_flag) {
        // 只有暂停状态下长按才允许复位，防止计时中误触清零
        __disable_irq();
        temp_timer_clock.hours   = 0;
        temp_timer_clock.minutes = 0;
        temp_timer_clock.seconds = 0;
        temp_timer_temp_second   = 0;
        __enable_irq();
    }
}

u8 wasLocked = 0; // 用于在 adjust_operate 调用前后检查锁定状态
u8 Adjust_IsLocked(void) { return lockFlag; }   // 供 Key_Event_Handler 判断

void Key_Event_Handler(void)
{
	KeyEvent pa_Event = Key_Fetch(&PA6_Key_State);
	KeyEvent pb_Event = Key_Fetch(&PB12_Key_State);
	switch(menuLevel){
		case MENU_CLOCK:	//时钟界面
			switch_operate(pa_Event);
			break;
		case MENU_FUNCTION:	//功能界面
			//stateUI = MENU_FUNCTION;	//保持在功能界面
			switch_operate(pa_Event);

			down_index_operate(pb_Event);

			confirm_operate(pa_Event);

			back_operate(pb_Event);
			break;
		case SON_MENU_SET_DATE:	//功能设置界面
			confirm_operate(pa_Event);

			wasLocked = Adjust_IsLocked();
    		adjust_operate(pa_Event, pb_Event, &cur_clock, FIELD_DAY);
    		if (!wasLocked) {
        		back_operate(pb_Event);
    		}
			break;
		case SON_MENU_SET_TIME:	//功能设置界面
			confirm_operate(pa_Event);

			wasLocked = Adjust_IsLocked();
    		adjust_operate(pa_Event, pb_Event, &cur_clock, FIELD_MINUTE);
    		if (!wasLocked) {
        		back_operate(pb_Event);
    		}
    break;
		case SON_MENU_TIMER:	//功能设置界面
			count_and_refresh_operate(pa_Event, pb_Event);
			back_operate(pb_Event);
			break;
		default:
			break;
	}
}

//display clock number in OLED
void Main_UI(void) {
	CUR_CLOCK *pCur_clock = &cur_clock;
	//DHT22_Data *p_Sensor = &sensor;
	int32_t week = date_to_week(pCur_clock->years, pCur_clock->monthes, pCur_clock->days);
	#if 1
	//显示温湿度
	//OLED_ShowChinese(0, 0, FONT_WEN);
	//OLED_ShowString(1, 3, ":");
	//OLED_ShowNum(1, 4, p_Sensor->temperature, 2);
	//OLED_ShowChinese(0, 2, FONT_SHI);
	//OLED_ShowString(2, 3, ":");
	//OLED_ShowNum(2, 4, p_Sensor->humidity, 2);

	//显示电量
	OLED_ShowString(2, 1, "Bat:");
	OLED_ShowNum(2, 5, battery_percent, 3);
	//显示星期
	//OLED_ShowString(2, 0, "Week:");
	switch (week)
	{
	case 1:
		OLED_ShowString(2, 9, "Mon");
		break;
	case 2:
		OLED_ShowString(2, 9, "Tues");
		break;
	case 3:
		OLED_ShowString(2, 9, "Wednes");
		break;
	case 4:
		OLED_ShowString(2, 9, "Thur");
		break;
	case 5:
		OLED_ShowString(2, 9, "Fri");
		break;
	case 6:
		OLED_ShowString(2, 9, "Satur");
		break;
	case 7:
		OLED_ShowString(2, 9, "Sun");
		break;
	default:
		break;
	}
	//OLED_ShowNum(2, 9, week, 1);
	
	//显示年月日
	OLED_ShowNum(1, 7, pCur_clock->years, 4);
	OLED_ShowString(1, 11, "-");
	OLED_ShowNum(1, 12, pCur_clock->monthes, 2);
	OLED_ShowString(1, 14, "-");
	OLED_ShowNum(1, 15, pCur_clock->days, 2);
	//显示时间
	OLED_ShowBigNum2(1, 4, pCur_clock->hours);
	OLED_ShowString(3, 7, ":");
	OLED_ShowBigNum2(4, 4, pCur_clock->minutes);
	OLED_ShowString(3, 13, ":");
	OLED_ShowNum(3, 14, pCur_clock->seconds, 2);
	#endif
}

//display function options in OLED
void Func_UI(void) {
	for(u8 i = 0; i < curSonMenuCount; i++) {
		if(i == selectedIndex) {
			OLED_ShowString(i + 1, 1, ">");
		} else {
			OLED_ShowString(i + 1, 1, " ");
		}
		OLED_ShowString(i + 1, 3, curMenu[i + 2].name);//curMenu[i].name);
	}
}

u8 save_date;
void Func_Set_Date(void) {
	//OLED_ShowString(1, 1, "Set Time");
	if(selectedIndex == 0 && sonMenuConfirmFlag == 1) {
		CUR_CLOCK *pCur_clock = &cur_clock;

		OLED_ShowNum(3, 3, pCur_clock->years, 4);
		OLED_ShowString(3, 7, "-");
		OLED_ShowNum(3, 8, pCur_clock->monthes, 2);
		OLED_ShowString(3, 10, "-");
		OLED_ShowNum(3, 11, pCur_clock->days, 2);
	} 
}

void Func_Set_Time(void) {
	if(selectedIndex == 1 && sonMenuConfirmFlag == 1) {
		CUR_CLOCK *pCur_clock = &cur_clock;
		
		OLED_ShowNum(3, 4, pCur_clock->hours, 2);
		OLED_ShowString(3, 6, ":");
		OLED_ShowNum(3, 7, pCur_clock->minutes, 2);
		OLED_ShowString(3, 9, ":");
		OLED_ShowNum(3, 10, pCur_clock->seconds, 2);
	}
}

u8 timer_run_flag = 0;
u8 timer_temp_stop_flag = 0;
u32 temp_timer_temp_second = 0;
CUR_CLOCK timer_clock = {0, 0, 0};
CUR_CLOCK temp_timer_clock = {0, 0, 0};
void Func_Timer(void) {
    CUR_CLOCK snapshot;

    // 原子地读一份快照，避免显示过程中被中断改到一半
    __disable_irq();
    snapshot = temp_timer_clock;
    __enable_irq();

    OLED_ShowString(1, 1, timer_run_flag ? "Running" : "Paused ");

    OLED_ShowNum(3, 4, snapshot.hours, 2);
    OLED_ShowString(3, 6, ":");
    OLED_ShowNum(3, 7, snapshot.minutes, 2);
    OLED_ShowString(3, 9, ":");
    OLED_ShowNum(3, 10, snapshot.seconds, 2);

    //OLED_ShowString(4, 1, timer_run_flag ? "PA6:Pause" : "PA6:Start");
}

	
