#include "ADC_Volt.h"
#include "Delay.h"

/* ================= 初始化部分 ================= */

// PB10 配置为推挽输出（控制Q7开关）
void BatDet_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_10); // 默认关闭开关
}

// PA2 配置为模拟输入 + ADC1初始化
void BatDet_ADC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    // ADC时钟不要超过14MHz，一般系统时钟72MHz要6分频
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    // PA2 -> ADC1_IN2，模拟输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);

    // 校准（标准库套路，必须做）
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

/* ================= 使用部分 ================= */

// 打开电量检测开关
void BatDet_SwitchOn(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
    //Delay_ms(100);   // 换成你项目里的延时函数，比如 Delay_ms / HAL_Delay 都行
}

// 关闭电量检测开关
void BatDet_SwitchOff(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
}

// 读取PA2（ADC1_IN2）原始值
uint16_t BatDet_ReadRaw(void)
{
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_55Cycles5);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    return ADC_GetConversionValue(ADC1);
}

// 获取电池电压 (单位: V)
float BatDet_GetVoltage(void)
{
    //BatDet_SwitchOn();

    uint16_t raw = BatDet_ReadRaw();

    BatDet_SwitchOff();

    float v_pa2 = (raw / 4095.0f) * 3.3f;
    float v_bat = v_pa2 * 2.0f;   // R11=R12=10K，1:1分压，所以乘2

    return v_bat;
}

uint8_t BatDet_GetPercentage(void)
{
    float v_bat = BatDet_GetVoltage();

    float v_empty = 3.1f;  // 电池视为"空"的电压，可按你电池实际情况调整
    float v_full  = 4.1f;  // 电池充满的电压

    if (v_bat >= v_full) return 100;
    if (v_bat <= v_empty) return 0;

    float percent = (v_bat - v_empty) / (v_full - v_empty) * 100.0f;

    return (uint8_t)percent;
}


