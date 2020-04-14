#ifndef DRIVER_H
#define DRIVER_H

extern u16 showing_data[];
extern u16 HC595_data;
extern u16 HC595_data_mask[];
extern u8 smg_scan;
extern u8 smg_scan2;

extern u8 bmq_turning;
extern u8 bmq_frist_down;
extern u8 bmq_last_up;
extern u8 bmq_idel_time;
extern u16 bmq_last_time;
extern u16 bmq_last2_time;

/*
#define coefficient_V_show 62700//数值越小，显示越高
#define offset_V_show 1693//数值越小，显示越高

#define coefficient_I_show 57957//数值越小，显示越高
#define offset_I_show 2471//数值越小，显示越高

#define coefficient_V_set 9880//数值越大，设定越高

#define coefficient_I_set 8270//数值越大，设定越高
//*/


//工厂模式的参数
extern s16 PWM_V_bias;
extern u16 PWM_V_coefficient;
extern s16 PWM_I_bias;
extern u16 PWM_I_coefficient;
extern s16 ADC_V_bias;
extern u16 ADC_V_coefficient;
extern s16 ADC_I_bias;
extern u16 ADC_I_coefficient;
extern u8 UI_time_out;
extern u8 btn_event;//按钮事件，0x01 = 编码器正转，0xFF = 编码器反转，02 = 按钮短按，03 = 按钮长按
extern u16 PWM = 0;
extern u8 *eep_addr;
extern u8 is_fan_need_to_speed_up = 0;

extern void hc595_update();

extern void set_V_PWM();
extern void set_I_PWM();

extern s16 calculation_factory_bias(u16 a,u16 b,u16 c);
extern void calculation_factory_parameters();

extern u8 mod();
extern u8 show_str(const u16 *str,u8 pos);
extern u8 UI_timeout_timer();
extern u8 flashing_FSM = 1;
extern void FSM_Reverse();

extern void display_left_1_digital();
extern void display_left_3_digital();
extern void display_right_3_digital();
extern void (*fp_display_PWM_value)();
extern void display_PWM_value();
extern void set_brightness( u8 brightness,u16 mask);

extern void chkbmq();
extern void bmq_wait_event();
extern s16 bmq_turn_mgr_seting_data;
extern u8 bmq_turn_mgr_speed_coefficient;
extern s16 bmq_turn_mgr_number_upper_limit;
extern s16 bmq_turn_mgr_number_lower_limit;
extern void (*fp_bmq_turn_mgr_display)();
extern void bmq_turn_mgr();

extern s16 factory_mode_seting_PWM_PWMdata;
extern void factory_mode_seting_PWM();

extern s16 eeprom_buf1;
extern s16 eeprom_buf2;
extern void eeprom_read_addrx8();
extern void eeprom_write_unlock_addrx8();
extern void eeprom_write();
#endif //#ifndef DRIVER_H
