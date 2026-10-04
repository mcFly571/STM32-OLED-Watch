#ifndef __RESERVE_FLASH_H
#define __RESERVE_FLASH_H

#include "Clock_Module.h"

#define FLASH_ADDR 0x0800C000
#define FLASH_TIMEOUT 0xFFFF

void FLASH_Unlock_Reg(void);
void FLASH_Lock_Reg(void);
u8 FLASH_Erase_Sector(uint32_t SectorAddr);
u8 FLASH_Write_HalfWord(u32 addr, u16 data);
u16 FLASH_Read_HalfWord(u32 addr);

u8 FLASH_Save_Clock(CUR_CLOCK clock_data);
CUR_CLOCK FLASH_Load_Data(void);

#endif
