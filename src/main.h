#ifndef MAIN_H
#define MAIN_H

extern u8 factory_mode_setV_or_setI;//工厂模式专用变量，0 = 设置电压（对应菜单项1和2），5 = 设置电流（对应菜单项3和4）。FLASH空间不足了，逼我做成代码复用....
extern u16 setV;//main线程专用，最终由TIM4实行
extern u16 setI;//main线程专用，最终由TIM4实行

extern u8 is_output_ON;
extern u8 output_PWM_update;

extern u16 nowV_16bit_ADC_result;
extern u16 nowV;
extern u16 nowI_16bit_ADC_result;
extern u16 nowI;

extern s16 seting_data[];
extern u8  flashing_style[];
extern u8  main_u8x;
extern u16 main_u16x;

extern u16 duanma[];
	

extern u8 pos;
extern void Init();

#endif //#ifndef MAIN_H

