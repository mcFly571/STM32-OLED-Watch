#include "AM2302_DHT22.h"

// ===================== SysTick 初始化 =====================
void DHT22_SysTick_Init(void)
{
    // 72MHz / 1000 = 72000，每 1ms 触发一次中断
    SysTick_Config(SystemCoreClock / 1000);
}

// ===================== 微秒延时（不依赖中断，直接查询） =====================
void delay_us(uint32_t us)
{
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    uint32_t t0     = SysTick->VAL;
    uint32_t reload = SysTick->LOAD + 1;
    uint32_t elapsed = 0;

    while (elapsed < ticks) {
        uint32_t now = SysTick->VAL;
        if (now < t0)
            elapsed += t0 - now;
        else
            elapsed += reload - now + t0;
        t0 = now;
    }
}

// ===================== IO 方向控制 =====================
static void DHT22_Set_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = DHT22_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT22_PORT, &GPIO_InitStructure);
}

static void DHT22_Set_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin  = DHT22_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DHT22_PORT, &GPIO_InitStructure);
}

static void DHT22_High(void)        { GPIO_SetBits(DHT22_PORT, DHT22_PIN); }
static void DHT22_Low(void)         { GPIO_ResetBits(DHT22_PORT, DHT22_PIN); }
static uint8_t DHT22_Read_Pin(void) { return GPIO_ReadInputDataBit(DHT22_PORT, DHT22_PIN); }

// ===================== 初始化 =====================
void DHT22_Init(void)
{
    RCC_APB2PeriphClockCmd(DHT22_RCC, ENABLE);
    DHT22_Set_Output();
    DHT22_High();
}

// ===================== 读取数据 =====================
uint8_t DHT22_Read(DHT22_Data *data)
{
    uint8_t  buf[5] = {0};
    uint8_t  i, j;
    uint32_t timeout;

    data->valid = 0;

    DHT22_Set_Output();
    DHT22_Low();
    delay_us(1000);
    DHT22_High();
    delay_us(30);

    DHT22_Set_Input();

    timeout = 10000;
    while (DHT22_Read_Pin() == 1) {
        if (--timeout == 0) return 0;
        delay_us(1);
    }
    timeout = 10000;
    while (DHT22_Read_Pin() == 0) {
        if (--timeout == 0) return 0;
        delay_us(1);
    }
    timeout = 10000;
    while (DHT22_Read_Pin() == 1) {
        if (--timeout == 0) return 0;
        delay_us(1);
    }

    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            timeout = 10000;
            while (DHT22_Read_Pin() == 0) {
                if (--timeout == 0) return 0;
                delay_us(1);
            }
            delay_us(40);
            buf[i] <<= 1;
            if (DHT22_Read_Pin() == 1)
                buf[i] |= 0x01;
            timeout = 10000;
            while (DHT22_Read_Pin() == 1) {
                if (--timeout == 0) return 0;
                delay_us(1);
            }
        }
    }

    if (buf[4] != (uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]))
        return 0;

    uint16_t raw_humi = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t raw_temp = ((uint16_t)(buf[2] & 0x7F) << 8) | buf[3];

    data->humidity    = raw_humi / 10.0f;
    data->temperature = raw_temp / 10.0f;

    if (buf[2] & 0x80)
        data->temperature = -data->temperature;

    data->valid = 1;
    return 1;
}
