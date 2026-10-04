/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "Timer.h"
#include "GUI.h"
#include "Clock_Module.h"
#include "Reserve_Flash.h"
#include "AM2302_DHT22.h"
#include "Key.h"
#include "ADC_Volt.h"
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */

// 在文件顶部加
#define BATTERY_DETECTION_INTERVAL 10000 // 电量检测间隔，单位：ms
static uint32_t g_battery_detection_tick = 0; // 电量检测计数
volatile uint32_t g_dht22_tick = 0;      // SysTick 计数
volatile uint8_t  g_dht22_read_flag = 0; // 读取标志位

volatile uint32_t g_ms = 0;
void SysTick_Handler(void)
{
  KeyEvent e1, e2;
  g_ms++;
  
  e1 = Key_State_Machine(GPIOA, GPIO_Pin_6, &PA6_ctx);
  if (e1 != KEY_NONE) 
  {
    PA6_Key_State = e1;
  }

  e2 = Key_State_Machine(GPIOB, GPIO_Pin_12, &PB12_ctx);
  if (e2 != KEY_NONE) 
  {
    PB12_Key_State = e2;
  }

  //读取电量
  
  if(++g_battery_detection_tick >= BATTERY_DETECTION_INTERVAL) {
    g_battery_detection_tick = 0;
    BatDet_SwitchOn();
  }

  // DHT22 读取标志位设置
  g_dht22_tick++;
  // 每 2000ms 设置一次标志位
  if (g_dht22_tick >= 2000) {
    g_dht22_tick = 0;
    g_dht22_read_flag = 1;  // 通知主循环去读 DHT22
  }

  //计时器功能
  if (timer_run_flag) {
    temp_timer_temp_second++;
    
    if (temp_timer_temp_second >= 1000) {
        temp_timer_temp_second -= 1000;
        temp_timer_clock.seconds++;

        if (temp_timer_clock.seconds >= 60) {
            temp_timer_clock.seconds = 0;
            temp_timer_clock.minutes++;

            if (temp_timer_clock.minutes >= 60) {
                temp_timer_clock.minutes = 0;
                temp_timer_clock.hours++;

                if (temp_timer_clock.hours >= 24) {
                    temp_timer_clock.hours = 0;   // 秒表一般到24小时归零即可
                }
            }
        }
    }
  }
}

void RTC_IRQHandler(void) {
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET) {
    RTC_ClearITPendingBit(RTC_IT_SEC);
    RTC_WaitForLastTask();
    //秒中断处理
    //这里可以添加需要在每秒钟执行的代码
    if (!g_time_editing) {              // ★ 编辑期间不覆盖
        RTC_GetClock(&cur_clock);
    }
  }
  
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
