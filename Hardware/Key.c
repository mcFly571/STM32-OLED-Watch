#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Clock_Module.h"
/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOB和GPIOA的时钟RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
	//初始化PA6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//初始化PB12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

volatile KeyEvent PA6_Key_State = KEY_NONE;
volatile KeyEvent PB12_Key_State = KEY_NONE;

// 每个按键各自独立一份上下文
KeyCtx PA6_ctx = {0};
KeyCtx PB12_ctx = {0};

#define KEY_DEBOUNCE_CNT   3     // 消抖次数
#define LONG_PRESS_DELAY   1000  // 长按启动延迟（次），配合调用周期换算，比如1ms调用一次则=1000ms
#define AUTO_INC_SPEED     100   // 长按连续触发间隔（次）

// ★ 需要每隔固定周期（例如1ms）调用一次，放在定时器中断或SysTick里
KeyEvent Key_State_Machine(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, KeyCtx *ctx)
{
    u8 pin_down = (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == 0);

    switch (ctx->key_state) {

    case 0: // 空闲
        if (pin_down) {
            if (++ctx->debounce_cnt >= KEY_DEBOUNCE_CNT) {
                ctx->debounce_cnt = 0;
                ctx->press_cnt = 0;
                ctx->key_state = 1;
            }
        } else {
            ctx->debounce_cnt = 0;
        }
        break;

    case 1: // 已确认按下
        if (!pin_down) {
            if (++ctx->debounce_cnt >= KEY_DEBOUNCE_CNT) {
                ctx->debounce_cnt = 0;
                ctx->key_state = 0;
                return KEY_PRESS;
            }
        } else {
            ctx->debounce_cnt = 0;
            if (++ctx->press_cnt >= LONG_PRESS_DELAY) {
                ctx->press_cnt = 0;
                ctx->key_state = 2;
                return KEY_LONG_PRESS;
            }
        }
        break;

    case 2: // 长按中
        if (!pin_down) {
            if (++ctx->debounce_cnt >= KEY_DEBOUNCE_CNT) {
                ctx->debounce_cnt = 0;
                ctx->key_state = 0;
            }
        } else {
            ctx->debounce_cnt = 0;
            if (++ctx->press_cnt >= AUTO_INC_SPEED) {
                ctx->press_cnt = 0;
                return KEY_AUTO_INC;
            }
        }
        break;
    }
    
    return KEY_NONE;
}

KeyEvent Key_Fetch(volatile KeyEvent *slot)
{
    KeyEvent e;
    __disable_irq();          // 只关几个周期，SysTick 挂起的中断不会丢
    e = *slot;
    *slot = KEY_NONE;
    __enable_irq();
    return e;
}
/* 判断FunctionEvent对应的事件 */
// void adjust_field(CUR_CLOCK *clk, FunctionEvent event, OperationDirection dir) {
//     int delata = (dir == DIR_ADD) ? 1 : -1;

//     switch (event) {
//         case FIELD_DAY:
//             clk->days += delata;
//             //进位错位处理
//             if(clk->days > Days_Of_Month_Culculate(clk->monthes)) {
//                 clk->days = 1;
//                 clk->monthes++;
//                 if(clk->monthes > 12) { clk->monthes = 1;clk->years++; }

//             } 

//             if (clk->days < 1) {
//                 clk->monthes--;
//                 if (clk->monthes < 1) { clk->monthes = 12;clk->years--; }
//                 clk->days = Days_Of_Month_Culculate(clk->monthes);
//             }
//             break;

//         case FIELD_MONTH:
//             clk->monthes += delata;
//             if (clk->monthes > 12) { clk->monthes = 1; clk->years++; }
//             if (clk->monthes < 1) { clk->monthes = 12; clk->years--; }
//             //调整日期，防止日期超过当月最大值
//             if (clk->days > Days_Of_Month_Culculate(clk->monthes)) {
//                 clk->days = Days_Of_Month_Culculate(clk->monthes);
//             }
//             break;

//         case FIELD_YEAR:
//             clk->years += delata;
//             if (clk->years < 1970) { clk->years = 1970; } //防止下溢
//             break;

//         case FIELD_HOUR:
//             clk->hours += delata;
//             if(clk->hours >= 24) { clk->hours = 0; }
//             //if(clk->hours > 23) { clk->hours = 23; } //防止下溢
//             if(clk->hours > 200) { clk->hours = 23; } //u8下溢变成255
//             break;
        
//         case FIELD_MINUTE:
//             clk->minutes += delata;
//             if(clk->minutes >= 60) { 
//                 clk->minutes = 0; 
//                 clk->hours++; 
//                 if(clk->hours >= 24) { clk->hours = 0; }
//             }
//             if(clk->hours > 200) { clk->hours = 59; } //u8下溢变成255 
//             break;

//         case FIELD_SECOND:
//             clk->seconds += delata;
//             if(clk->seconds >= 60) { 
//                 clk->seconds = 0; 
//                 clk->minutes++; 
//                 if(clk->minutes >= 60) { 
//                     clk->minutes = 0; 
//                     clk->hours++; 
//                 } 
//             } 
//             if(clk->seconds > 200) { clk->seconds = 59; } //u8下溢变成255
//             break;
//     }
// }

/* 根据字段刷新显示 */
// static void refresh_display(CUR_CLOCK *clk, Field field) {
//     if(field <= FIELD_YEAR) {
//         //显示年月日
//         OLED_ShowNum(3, 3, clk->years, 4);
//         OLED_ShowString(3, 7, "-");
//         OLED_ShowNum(3, 8, clk->monthes, 2);
//         OLED_ShowString(3, 10, "-");
//         OLED_ShowNum(3, 11, clk->days, 2);
//     }else {
//         //显示时间
//         OLED_ShowNum(3, 4, clk->hours, 2);
//         OLED_ShowString(3, 6, ":");
//         OLED_ShowNum(3, 7, clk->minutes, 2);
//         OLED_ShowString(3, 9, ":");
//         OLED_ShowNum(3, 10, clk->seconds, 2);
//     }

// }

/* 通用状态机
    GPIOx, GPIO_Pin: 触发按键的 GPIO 端口和引脚
    field: 要调整的字段
    dir: 调整方向（增加或减少）
    
    调用前需将对应的 press_flag 置 1
    函数会在按键松开后自动清零 press_flag 并写入RTC
*/

#if 0
/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~2，返回0代表没有按键按下
  * 注意事项：此函数是阻塞式操作，当按键按住不放时，函数会卡住，直到按键松手
  */

uint8_t Key_Pressed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    static uint8_t last_state = 1;  // 记录上一次电平状态，默认松开(高电平)
    uint8_t cur_state = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
    uint8_t triggered = 0;

    if(last_state == 1 && cur_state == 0) {
        // 检测到从高变低的瞬间，判定为一次有效按下
        triggered = 1;
        OLED_Clear();
    }
    last_state = cur_state;

    return triggered;
}
#endif
