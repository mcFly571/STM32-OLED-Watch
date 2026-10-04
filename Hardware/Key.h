#ifndef __KEY_H
#define __KEY_H

//按键事件
typedef enum {
    KEY_NONE = 0,
    KEY_PRESS,
    KEY_LONG_PRESS,
    KEY_AUTO_INC,
} KeyEvent;

//操作方向
typedef enum {
    DIR_NONE = 0,
    DIR_ADD,
    DIR_DECREASE,
} OperationDirection;

typedef struct {
    u8  key_state;     // 0:空闲 1:已确认按下 2:长按中
    u16 press_cnt;
    u8  debounce_cnt;
} KeyCtx;

extern volatile KeyEvent PA6_Key_State;
extern volatile KeyEvent PB12_Key_State;

extern KeyCtx PA6_ctx;
extern KeyCtx PB12_ctx;

extern volatile uint32_t g_ms;
//#define GetKey1Press()	Key_Pressed(GPIOA, GPIO_Pin_1)  //switch key
//#define GetKey9Press()	Key_Pressed(GPIOA, GPIO_Pin_9)  //index ke

#define GetKeyPA6Pressed()	Key_State_Machine(GPIOA, GPIO_Pin_6, &PA6_ctx)

#define GetKeyPB12Pressed()	Key_State_Machine(GPIOB, GPIO_Pin_12, &PB12_ctx)

KeyEvent Key_State_Machine(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, KeyCtx *ctx);

void Key_Init(void);

uint8_t Key_Pressed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

KeyEvent Key_Fetch(volatile KeyEvent *slot);

#endif
