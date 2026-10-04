#ifndef __ADC_VOLT_H
#define __ADC_VOLT_H

#include "stm32f10x.h"

void BatDet_GPIO_Init(void);
void BatDet_ADC_Init(void);
void BatDet_SwitchOn(void);
void BatDet_SwitchOff(void);
uint16_t BatDet_ReadRaw(void);
float BatDet_GetVoltage(void);
uint8_t BatDet_GetPercentage(void);

#endif // __ADC_VOLT_H
