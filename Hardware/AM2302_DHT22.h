#ifndef __DHT22_H
#define __DHT22_H

#include "stm32f10x.h"

// 引脚配置 - PA3
#define DHT22_PORT      GPIOA
#define DHT22_PIN       GPIO_Pin_4
#define DHT22_RCC       RCC_APB2Periph_GPIOA

typedef struct {
    float temperature;  // 温度 (°C)
    float humidity;     // 湿度 (%)
    uint8_t valid;      // 数据是否有效
} DHT22_Data;

void DHT22_Init(void);
void delay_us(uint32_t us);
void DHT22_SysTick_Init(void);
uint8_t DHT22_Read(DHT22_Data *data);

extern DHT22_Data sensor;

#endif

