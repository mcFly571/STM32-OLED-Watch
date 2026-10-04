#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "GUI.h"
#include "Timer.h"
#include "Clock_Module.h"
#include "ADC_Volt.h"
//#include "Reserve_Flash.h"
#include "AM2302_DHT22.h"

u8 mainCurIndex;
u8 sonCurIndex;
u8 save_status;
u8 save_current_clock = 0;
//DHT22_Data sensor;
int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	Key_Init();
	CurrentClock_Init();
	RTC_Clock_Init(&cur_clock);	//RTC时钟初始化，传入初始时间
	BatDet_GPIO_Init();
  	BatDet_ADC_Init();

	DHT22_SysTick_Init();	//DHT22初始化
	//DHT22_Init();
	//Delay_ms(2000);		//等待温湿度模块稳定
	uint32_t last_ui = 0;
	while (1)
	{
		Key_Event_Handler();                  // 每圈都处理按键，响应最快

        if (g_ms - last_ui >= 50) {           // 每 50ms 刷新一次界面
            last_ui = g_ms;
            Menu_Display();
            battery_percent = BatDet_GetPercentage();
		}

		//if(g_dht22_read_flag){
		//	DHT22_Read(&sensor);
		//	g_dht22_read_flag = 0;  // 清除标志位
		//}
	}
		
}
