#ifndef __OLED_H
#define __OLED_H


void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

void OLED_ShowChar16x32(uint8_t x, uint8_t y, const uint8_t *font);
void OLED_ShowBigNum(uint8_t x, uint8_t y, uint8_t num);
void OLED_ShowBigNum2(uint8_t x, uint8_t y, uint8_t num);

void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t index);

#define FONT_WEN   0   /* 温 */
#define FONT_SHI   1   /* 湿 */
#define FONT_DU    2   /* 度 */
#define FONT_XING  3   /* 星 */
#define FONT_QI    4   /* 期 */
#define FONT_YI    5   /* 一 */
#define FONT_ER    6   /* 二 */
#define FONT_SAN   7   /* 三 */
#define FONT_SI    8   /* 四 */
#define FONT_WU    9   /* 五 */
#define FONT_LIU   10  /* 六 */
#define FONT_RI    11  /* 日 */

#endif
