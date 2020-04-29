#ifndef IT_H
#define IT_H

extern u8 btn_status;

extern u16 _setV;//TIM4线程专用
extern u16 _setI;//TIM4线程专用

extern u8 btn_down_time;

extern u8 user_timer1;

extern void setV_update();

#endif //#ifndef IT_H
