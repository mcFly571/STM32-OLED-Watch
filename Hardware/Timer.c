#include "Timer.h"

volatile u32 g_RTC_Count = 0;
volatile u8 save_time_flag = 0;
//初始化RTC
void RTC_LSE_init(void) {
  #if 0
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    BKP_DeInit();

    // 直接启动 LSI（内部低速时钟，无需外部晶振）
    RCC_LSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);  // 等待LSI稳定（很快）

    // 选择 LSI 作为 RTC 时钟源
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    RCC_RTCCLKCmd(ENABLE);

    RTC_WaitForSynchro();
    RTC_WaitForLastTask();

    // LSI 约 40kHz，预分频 39999 → 约 1Hz
    RTC_SetPrescaler(39999);
    RTC_WaitForLastTask();
  #endif

  #if 1
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    BKP_DeInit();

    RCC_LSEConfig(RCC_LSE_ON);

    // 加超时，最多等 500ms
    uint32_t timeout = 500000;
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
        if (timeout-- == 0) {
            // LSE 启动失败，改用 LSI
            RCC_LSEConfig(RCC_LSE_OFF);
            RCC_LSICmd(ENABLE);
            while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
            RCC_RTCCLKCmd(ENABLE);
            RTC_WaitForSynchro();
            RTC_WaitForLastTask();
            RTC_SetPrescaler(39999);  // LSI 约40kHz
            RTC_WaitForLastTask();
            return;  // 直接返回，不走后面LSE的流程
        }
    }

    // LSE 正常启动
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();
    RTC_WaitForLastTask();
    RTC_SetPrescaler(32767);
    RTC_WaitForLastTask();
    #endif

  #if 0
  //使能电源和备份时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  //允许访问备份寄存器和RTC（解除写保护）
  PWR_BackupAccessCmd(ENABLE);
  //复位备份域
  BKP_DeInit();
  
  //启动LSE
  RCC_LSEConfig(RCC_LSE_ON);
  //等待LSE稳定
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
  
  //选择RTC时钟源为LSE
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  //使能RTC时钟
  RCC_RTCCLKCmd(ENABLE);
  
  //等待APB1与RTC时钟同步
  RTC_WaitForSynchro();
  //等待上一次写操作完成
  RTC_WaitForLastTask();

  //设置预分频值，使RTC计数频率为1Hz
  RTC_SetPrescaler(32767); // LSE频率为32.768kHz，预分频值为32767
  RTC_WaitForLastTask();
  #endif
}

void RTC_NVIC_Init(void) {
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  // 使能RTC秒中断
  RTC_ITConfig(RTC_IT_SEC, ENABLE); 
  RTC_WaitForLastTask();
}

//判断闰年
u8 is_leap_year(u16 year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
static const u8 days_in_month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

//将时间结构体转换为时间戳
u32 clock_to_timestamp(CUR_CLOCK *pCur_clock) {
  // 将 CUR_CLOCK 结构体转换为时间戳（秒数）
  u32 timestamp = 0;
  u16 y;
  u8 m;

  for(y = 1970; y < pCur_clock->years; y++) {
    timestamp += is_leap_year(y) ? 366 : 365;
  }

  for(m = 1; m < pCur_clock->monthes; m++) {
    timestamp += days_in_month[m];
    if(m == 2 && is_leap_year(pCur_clock->years)) {
      timestamp += 1; // 闰年2月多一天
    }
  }

  timestamp += (pCur_clock->days - 1); // 天数从1开始
  timestamp *= 86400UL; // 转换为秒
  timestamp += pCur_clock->hours * 3600;
  timestamp += pCur_clock->minutes * 60;
  timestamp += pCur_clock->seconds;
  
  return timestamp;
}

//将时间戳转换为时间结构体
void timestamp_to_clock(u32 timestamp, CUR_CLOCK *pCur_clock) {
  u32 days_total;
  u8 dim;

  pCur_clock->seconds = timestamp % 60;
  timestamp /= 60;

  pCur_clock->minutes = timestamp % 60;
  timestamp /= 60;

  pCur_clock->hours = timestamp % 24;
  timestamp /= 24;

  days_total = timestamp;

  pCur_clock->years = 1970;
  while (1) {
    u16 days_in_year = is_leap_year(pCur_clock->years) ? 366 : 365;
    if (days_total < days_in_year) {break;}
    days_total -= days_in_year;
    pCur_clock->years++;
  }
  
  pCur_clock->monthes = 1;
  while (1) {
    dim = days_in_month[pCur_clock->monthes];
    if (pCur_clock->monthes == 2 && is_leap_year(pCur_clock->years)) {
      dim = 29; // 闰年2月多一天
    }
    if (days_total < dim) {break;}
    days_total -= dim;
    pCur_clock->monthes++;
  }
  pCur_clock->days = (u8)(days_total + 1); // 天数从1开始
}

void RTC_Clock_Init(CUR_CLOCK *init_clock) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    if(BKP_ReadBackupRegister(BKP_DR1) != RTC_INIT_FLAG) {
        /* 首次初始化：完整配置 RTC */
        RTC_LSE_init();
        RTC_SetClock(init_clock);
        BKP_WriteBackupRegister(BKP_DR1, RTC_INIT_FLAG);
    } else {
        /* 非首次：LSI 需要重新启动，但不能 BKP_DeInit */
        //RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
        //PWR_BackupAccessCmd(ENABLE);
        /* 重新启动 LSI */
        RCC_LSICmd(ENABLE);
        while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); // 重新选择时钟源（LSI)
        /* 重新使能 RTC 时钟 */
        RCC_RTCCLKCmd(ENABLE);
        RTC_WaitForSynchro();       // 现在有时钟源了，不会卡死
        RTC_WaitForLastTask();

        RTC_SetPrescaler(39999);    // 重新设置预分频值（LSI 约40kHz）
        RTC_WaitForLastTask();
    }

    /* 无论是否首次，配置中断并读取时间 */
    RTC_NVIC_Init();
    RTC_GetClock(&cur_clock);
}

void RTC_GetClock(CUR_CLOCK *pCur_clock) {
  timestamp_to_clock(RTC_GetCounter(), pCur_clock);
}

void RTC_SetClock(CUR_CLOCK *pCur_clock) {
  RTC_WaitForLastTask();
  RTC_SetCounter(clock_to_timestamp(pCur_clock));
  RTC_WaitForLastTask();
}

#if 0
void Timer_init(void) {
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    
    
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;
    TIM_TimeBaseStructure.TIM_Period = 10000-1;    
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    
    
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    
    TIM_Cmd(TIM2, ENABLE);
}


// 1 second per times
void TIM2_IRQHandler(void) {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		// interupt
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    
		CUR_CLOCK *pCur_clock = &cur_clock;
		pCur_clock->seconds += 1;
	
		//calculate time
		if(Advance_Sixty(pCur_clock->seconds)) {
			pCur_clock->seconds = 0;
		
			pCur_clock->minutes += 1;
		}
	
		if(Advance_Sixty(pCur_clock->minutes)) {
			pCur_clock->minutes = 0;
			
			pCur_clock->hours += 1;
		}
		
		if(pCur_clock->hours % 24 == 0)
			pCur_clock->hours = 0;
	}
}
#endif

