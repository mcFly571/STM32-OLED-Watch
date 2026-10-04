#include "stm32f10x.h"
#include "Reserve_Flash.h"


void FLASH_Unlock_Reg(void){
	// 检查是否锁定，如果锁定则解锁
	if(FLASH->CR & FLASH_CR_LOCK){
		FLASH->KEYR = 0x45670123;	// 写入第一个密钥
		FLASH->KEYR = 0xCDEF89AB;	// 写入第二个密钥
	}
}

// 锁定 FLASH 寄存器
void FLASH_Lock_Reg(void){
	FLASH->CR |= FLASH_CR_LOCK;
}

u8 FLASH_Erase_Sector(u32 SectorAddr){
	u32 timeout = FLASH_TIMEOUT;
	
	// 1.等待 FLASH 空闲（BSY位为0）
	while((FLASH->SR & FLASH_SR_BSY) && (timeout-- > 0));
	//time out
	if(timeout == 0)
		return 1;
	
	// 2.擦除扇区，设置 PER 位
	FLASH->CR |= FLASH_CR_PER;
	// 3.设置要擦除的扇区地址
	FLASH->AR = SectorAddr;
	// 4.擦除扇区，设置 STRT 位
	FLASH->CR |= FLASH_CR_STRT;
	
	// 5.等待擦除完成
	timeout = FLASH_TIMEOUT;
	while((FLASH->SR & FLASH_SR_BSY) && (timeout-- > 0));
	if(timeout == 0)
		return 1;
	
	// 6.清除扇区擦除位(PER)，设置完成位(EOP)
	FLASH->CR &= ~FLASH_CR_PER;
	FLASH->SR |= FLASH_SR_EOP;	// 清除完成位(EOP)
	
	return 0;
}

u16 FLASH_Read_HalfWord(u32 addr){
	return *(volatile u16*)addr;
}

u8 FLASH_Save_Clock(CUR_CLOCK clock_data){
	FLASH_Status status;
	u32 addr = FLASH_ADDR;

	CUR_CLOCK check_clock;

	// 1.解锁 FLASH
	FLASH_Unlock_Reg();
	// 2.等待 FLASH 空闲（BSY位为0）
	while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) != RESET);
	// 3.擦除整个页
	status = FLASH_ErasePage(FLASH_ADDR);
	if(status != FLASH_COMPLETE){
		FLASH_Lock();
		return 1;
	}
	// 4.写入数据（秒、分、时、日、月、年）到 FLASH
	while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) != RESET);	
	status = FLASH_ProgramHalfWord(addr, clock_data.seconds);
	if(status != FLASH_COMPLETE){
		FLASH_Lock();
		return 2;
	}

	addr += 2;
	while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) != RESET);
	status = FLASH_ProgramHalfWord(addr, clock_data.minutes);
	if(status != FLASH_COMPLETE){
		FLASH_Lock();
		return 3;
	}

	addr += 2;
	while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) != RESET);
	status = FLASH_ProgramHalfWord(addr, clock_data.hours);
	if(status != FLASH_COMPLETE){
		FLASH_Lock();
		return 4;
	}

	addr += 2;
	while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) != RESET);
	status = FLASH_ProgramHalfWord(addr, clock_data.days);
	if(status != FLASH_COMPLETE){
		FLASH_Lock();
		return 5;
	}

	addr += 2;
	while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) != RESET);
	status = FLASH_ProgramHalfWord(addr, clock_data.monthes);
	if(status != FLASH_COMPLETE){
		FLASH_Lock();
		return 6;
	}

	addr += 2;
	while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) != RESET);
	status = FLASH_ProgramHalfWord(addr, clock_data.years);
	if(status != FLASH_COMPLETE){
		FLASH_Lock();
		return 7;
	}

	// 5.清除标志位并锁定和验证 FLASH
	FLASH_ClearFlag(FLASH_FLAG_EOP);
	FLASH_Lock();

	check_clock = FLASH_Load_Data();
	if(check_clock.seconds != clock_data.seconds ||
	   check_clock.minutes != clock_data.minutes ||
	   check_clock.hours != clock_data.hours ||
	   check_clock.days != clock_data.days ||
	   check_clock.monthes != clock_data.monthes ||
	   check_clock.years != clock_data.years)
	    {
			return 8; // 验证失败
		}

	return 0;
}

CUR_CLOCK FLASH_Load_Data(void){
	CUR_CLOCK clock_tmp;
	u32 addr = FLASH_ADDR;

	// 从 FLASH 中读取数据到 clock_tmp 结构体
	clock_tmp.seconds = *(volatile u8*)(addr);
	clock_tmp.minutes = *(volatile u8*)(addr + 2);
	clock_tmp.hours = *(volatile u8*)(addr + 4);
	clock_tmp.days = *(volatile u8*)(addr + 6);
	clock_tmp.monthes = *(volatile u8*)(addr + 8);
	clock_tmp.years = *(volatile u16*)(addr + 10);

	return clock_tmp;
}

u8 FLASH_Write_HalfWord(u32 addr, u16 data){
	u32 timeout = FLASH_TIMEOUT;
	
	//1. wait FLASH free(bit BSY is 0)
	while((FLASH->SR & FLASH_SR_BSY) && (timeout-- > 0));
	//time out
	if(timeout == 0)
		return 1;
	
	//2. set programming bit(PSIZE=00,PG)
	FLASH->CR &= ~FLASH_CR_PG;	//PSIZE=00, half word
	FLASH->CR |= FLASH_CR_PG;		//set PG bit
	
	//3. write data to target address
	*(volatile u16*)addr = data;
	
	//4. wait for programming complete
	timeout = FLASH_TIMEOUT;
	while((FLASH->SR & FLASH_SR_BSY) && (timeout-- > 0));
	if(timeout == 0)
		return 1;
	
	//5. clear programming bit, set complete bit
	FLASH->CR &= ~FLASH_CR_PG;	//clear PG bit
	FLASH->SR |= FLASH_SR_EOP;	//clear EOP bit
	
	return 0;
}
