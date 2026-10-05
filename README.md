https://github.com/user-attachments/assets/ed302dfb-9cce-4e27-a586-5144f3911453

<img width="1263" height="894" alt="Image" src="https://github.com/user-attachments/assets/5d6543fe-b8a0-44f0-85cd-c63d7b9dcde1" />
<img width="1401" height="534" alt="Image" src="https://github.com/user-attachments/assets/1325ff89-e5d2-4d2e-94d0-382a95ea53b0" />
<img width="1021" height="864" alt="Image" src="https://github.com/user-attachments/assets/f8f8415c-cb27-4a81-acb4-47e13bf6c715" />

硬件部分：STM32F1C8T6、ST-Link、4PIN-OLED搭建的，其中用了两个笑脸按键开关，按键采用点击/长按状态机，长按PA6切换菜单、长按PB12返回、点击PA6为确认操作、点击PB12为下移指针操作。

软件部分：使用I2C驱动OLED，RTC-LSE控制主菜单时钟，菜单模式为结构体数组函数指针struct Menu mainMenu[]。
