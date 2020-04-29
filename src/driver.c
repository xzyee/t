#include "iostm8s103f3.h"
#include "general.h"
#include "driver.h"
#include "it.h"
#include "display.h"
#include "main.h"
#include "parameter.h"
#include "led.h"

#include "led.c"

/*
    【EEPROM空间分配情况】
    0-7         开机默认电压电流         8字节
    8-79        9组存储电压电流          72字节
    80-119      工厂模式的显式参数       40字节
    120-151     工厂模式的隐式参数       32字节
*/

#define BYTES_EEPROM_ADDR_START_DEFAULT   8
#define BYTES_EEPROM_ADDR_START_USER      40
#define BYTES_EEPROM_ADDR_START_LIMIT     8
#define BYTES_EEPROM_ADDR_START_CAL_RAW   40
#define BYTES_EEPROM_ADDR_START_CAL_DATA  32

/*
    【EEPROM空间分配情况】
	开机默认电压电流         8字节
	9组存储电压电流         40字节
	出厂限制电压电流         8字节
	工厂模式的显式参数      40字节
	工厂模式的隐式参数      32字节
*/

  /*
  seting_data[1] = 0;//0.5V对应的PWM值
  seting_data[2] = 0;//0.5V对应的ADC值
  seting_data[3] = 600;//9-60V之间选中的参考电压（比例：600 = 60V）
  seting_data[4] = 0;//9-60V之间选中的参考电压对应的PWM值
  seting_data[5] = 0;//9-60V之间选中的参考电压对应的ADC值
  seting_data[6] = 0;//0.5A对应的PWM值
  seting_data[7] = 0;//0.5A对应的ADC值
  seting_data[8] = 100;//9-40A之间选中的参考电流（比例：400 = 40A）
  seting_data[9] = 0;//9-40A之间选中的参考电流对应的PWM值
  seting_data[10] = 0;//9-40A之间选中的参考电流对应的ADC值
  */


/*
    【选项字节设置】
    需要激活：AFR0
*/

/*
    【74HC595与数码管连接的脚位定义】
    第1级74HC595的Q0对应0x0001<<0
    第1级74HC595的Q7对应0x0001<<7
    第2级74HC595的Q0对应0x0001<<8
    第2级74HC595的Q7对应0x0001<<15  以此类推
*/

u16 showing_data[7] = {0};
u16 HC595_data;
u16 HC595_data_mask[] = {0xFFFF,0xFFFF};

void clear_by_null()
{
	
	showing_data[1] = Dpy_wei_1 & Dpy_duan_null;
	showing_data[2] = Dpy_wei_2 & Dpy_duan_null;
	showing_data[3] = Dpy_wei_3 & Dpy_duan_null;
	showing_data[4] = Dpy_wei_4 & Dpy_duan_null;
	showing_data[5] = Dpy_wei_5 & Dpy_duan_null;
	showing_data[6] = Dpy_wei_6 & Dpy_duan_null;
}

void dark0123()
{
	showing_data[0] = DARK;
	showing_data[1] = DARK;
	showing_data[2] = DARK;
	showing_data[3] = DARK;
}
void dark012()
{
	showing_data[0] = DARK;
	showing_data[1] = DARK;
	showing_data[2] = DARK;
}
void dark23()
{
	showing_data[2] = DARK;
	showing_data[3] = DARK;
}

void hc595_update(){
  PD4O = 0;
  if(HIGH(HC595_data)&0x80) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(HIGH(HC595_data)&0x40) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(HIGH(HC595_data)&0x20) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(HIGH(HC595_data)&0x10) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(HIGH(HC595_data)&0x08) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(HIGH(HC595_data)&0x04) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(HIGH(HC595_data)&0x02) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(HIGH(HC595_data)&0x01) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(LOW(HC595_data)&0x80) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(LOW(HC595_data)&0x40) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(LOW(HC595_data)&0x20) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(LOW(HC595_data)&0x10) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(LOW(HC595_data)&0x08) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(LOW(HC595_data)&0x04) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(LOW(HC595_data)&0x02) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PD4O = 0;
  if(LOW(HC595_data)&0x01) PD4O = 1;
  PA2O = 0;
  PA2O = 1;
  
  PA1O = 1;
  PA1O = 0;
}



u8 bmq_status1 = 1;
u8 bmq_status2 = 1;

u8 bmq_turning = 0;
u8 bmq_frist_down;
u8 bmq_last_up;
u8 bmq_idel_time = 0xFF;
u16 bmq_last_time = 0x8000;
u16 bmq_last2_time = 0x8000;

/*
#define coefficient_V_show 62700//数值越小，显示越高
#define offset_V_show 1693//数值越小，显示越高

#define coefficient_I_show 57957//数值越小，显示越高
#define offset_I_show 2471//数值越小，显示越高

#define coefficient_V_set 9880//数值越大，设定越高

#define coefficient_I_set 8270//数值越大，设定越高
*/

//工厂模式的参数
s16 PWM_V_bias;
u16 PWM_V_coefficient;
s16 PWM_I_bias;
u16 PWM_I_coefficient;
s16 ADC_V_bias;
u16 ADC_V_coefficient;
s16 ADC_I_bias;
u16 ADC_I_coefficient;

u8 UI_time_out;

u8 btn_event = 0;//按钮事件，0x01 = 编码器正转，0xFF = 编码器反转，02 = 按钮短按，03 = 按钮长按

u16 PWM = 0;

u8 *eep_addr;

u8 is_fan_need_to_speed_up = 0;

void set_V_PWM()
{
  TIM1_CCR3H = 0;
  TIM1_CCR3L = PWM % 100;
  TIM1_CCR4H = 0;
  TIM1_CCR4L = PWM / 100 % 100;
}

void set_I_PWM()
{
  TIM1_CCR1H = 0;
  TIM1_CCR1L = PWM % 100;
  TIM1_CCR2H = 0;
  TIM1_CCR2L = PWM / 100 % 100;
}

void set_brightness(u8 brightness,u16 mask)
{
	if(brightness == FULLBRIGHT)
    {
	    HC595_data_mask[0] = 0xFFFF;
	    HC595_data_mask[1] = 0xFFFF;
    }
	else if(brightness == WEAKBLINK) 
    {
        HC595_data_mask[0] = 0xFFFF;
		HC595_data_mask[1] = ~mask;
    }
	else
    {
	    HC595_data_mask[0] = ~mask;
	    HC595_data_mask[1] = HC595_data_mask[0];
    }
}


#pragma optimize = low
s16 calculation_factory_bias(u16 a,u16 b,u16 c){
  return 
  a - (s16)(
  (u32)(c-a) /*差值*/
  *5         /*下余缝*/
  /(b-5)     /*电压差*/
  );
}


#pragma optimize = low
u16 calculation_factory_coefficient(u16 a,u16 b,u16 c){
  return 
  (c-a)          /*差值*/
  +
  (u16)(
  (u32)(c-a)     /*差值*/
  *(600-b+5)      /*上下余缝*/
  /(b-5)          /*电压差*/
  );
}

#pragma optimize = low
void calculation_factory_parameters()
{
  PWM_V_bias = calculation_factory_bias(
  seting_data[1]
 ,seting_data[3]
 ,seting_data[4]
  );
  
  PWM_V_coefficient = calculation_factory_coefficient(
  seting_data[1]
 ,seting_data[3]
 ,seting_data[4]
  );
  
  //PWM_V_coefficient += (PWM_V_coefficient/2400);//微调补偿
  PWM_V_bias += (PWM_V_coefficient/2400);//微调补偿
  
  ADC_V_bias = calculation_factory_bias(
  seting_data[2]
 ,seting_data[3]
 ,seting_data[5]
  );
  
  ADC_V_coefficient = calculation_factory_coefficient(
  seting_data[2]
 ,seting_data[3]
 ,seting_data[5]
  );
  
  //ADC_V_coefficient-=(ADC_V_coefficient/2400);//微调补偿
  ADC_V_bias-=(ADC_V_coefficient/2400);//微调补偿
  
  PWM_I_bias = calculation_factory_bias(
  seting_data[1+5]
 ,seting_data[3+5]
 ,seting_data[4+5]
  );
  
  PWM_I_coefficient = calculation_factory_coefficient(
  seting_data[1+5]
 ,seting_data[3+5]
 ,seting_data[4+5]
  );
  
  //PWM_I_coefficient += (PWM_I_coefficient/2400);//微调补偿
  PWM_I_bias += PWM_I_coefficient/2400;//微调补偿
  
  ADC_I_bias = calculation_factory_bias(
  seting_data[2+5]
 ,seting_data[3+5]
 ,seting_data[5+5]
  );
  
  ADC_I_coefficient = calculation_factory_coefficient(
  seting_data[2+5]
 ,seting_data[3+5]
 ,seting_data[5+5]
  );
  
  //ADC_I_coefficient-=(ADC_I_coefficient/2400);//微调补偿
  ADC_I_bias -= ADC_I_coefficient/2400;//微调补偿
}

//模余运算，用main_u16x传参
u8 mod(){
  u8 result;
  result = main_u16x % 10;
  main_u16x /= 10;
  return result;
}

void clear_showing_data()
{
    showing_data[0] = DARK;
    showing_data[1] = DARK;
    showing_data[2] = DARK;
    showing_data[3] = DARK;
    showing_data[4] = DARK;
    showing_data[5] = DARK;
    showing_data[6] = DARK;	
}

void clear_showing_data_but34()
{
	showing_data[0] = 
	showing_data[1] = 
	showing_data[2] = 
	showing_data[5] = 
	showing_data[6] = DARK;
}

//str是字符串指针，pos是字符串的当前要显示的字符位置，str[pos+1]，仅走字的情况
u8 show_str(const u16 *str,u8 pos)
{
  main_u8x = str[0];//字符串长度
  if(main_u8x > 6)
  {
    showing_data[1] = showing_data[2] & (~_Dpy_wei_2) | _Dpy_wei_1;
    showing_data[2] = showing_data[3] & (~_Dpy_wei_3) | _Dpy_wei_2;
    showing_data[3] = showing_data[4] & (~_Dpy_wei_4) | _Dpy_wei_3;
    showing_data[4] = showing_data[5] & (~_Dpy_wei_5) | _Dpy_wei_4;
    showing_data[5] = showing_data[6] & (~_Dpy_wei_6) | _Dpy_wei_5;
    showing_data[6] = Dpy_wei_6 & str[pos+1];
    
    if(++pos >= main_u8x)
    {
    pos = 0;
    }
  }
  else
  { //不走字时，pos无用处
    showing_data[1] = Dpy_wei_1 & str[1];
    showing_data[2] = Dpy_wei_2 & str[2];
    showing_data[3] = Dpy_wei_3 & str[3];
    showing_data[4] = Dpy_wei_4 & str[4];
    showing_data[5] = Dpy_wei_5 & str[5];
    showing_data[6] = Dpy_wei_6 & str[6];
    pos = 0;
  }
  return pos;
}


u8 UI_timeout_timer()
{
  return ++UI_time_out >= 20;
}


u8 flashing_FSM = 1;//数码管闪烁专用状态机，0 = 暗，1 = 亮

void FSM_Reverse()
{
  flashing_FSM = !flashing_FSM;
}

//用main_u16x传参要显示的数值，用main_x传参亮度
void display_left_1_digital()
{
  showing_data[0] = Dpy_wei_0 & duanma[ (u8)main_u16x ];
  
  set_brightness(main_u8x,_Dpy_wei_0);
}

//用main_u16x传参要显示的数值，用main_x传参亮度
void display_left_4_digital()
{
/*   if(main_u16x & 0x8000){//是负数
    main_u16x = (~main_u16x) + 1;
    showing_data[1] = Dpy_wei_1 & Dpy_duan_negative;
  }else{
    showing_data[1] = Dpy_wei_1 & Dpy_duan_null;
  } */
  
  if(main_u16x > 9999)
  {
	mod();
	showing_data[3] = Dpy_wei_3 & duanma[mod()];
	showing_data[2] = Dpy_wei_2 & duanma[mod()] & Dpy_duan_dp;
	showing_data[1] = Dpy_wei_1 & duanma[mod()];
	showing_data[0] = Dpy_wei_0 & duanma[mod()];
  }
  else
  {
	showing_data[3] = Dpy_wei_3 & duanma[mod()];
	showing_data[2] = Dpy_wei_2 & duanma[mod()];
	showing_data[1] = Dpy_wei_1 & duanma[mod()] & Dpy_duan_dp;
	if(main_u16x) 
		showing_data[0] = Dpy_wei_0 & duanma[main_u16x];
	else
		showing_data[0] = Dpy_wei_0 & Dpy_duan_null;
	}
  
  set_brightness(main_u8x,_Dpy_wei_0 |_Dpy_wei_1 | _Dpy_wei_2 | _Dpy_wei_3);
}
  

//用main_u16x传参要显示的数值，用main_x传参亮度
void display_right_3_digital()
{
  if(main_u16x & 0x8000)
  {//是负数
    main_u16x=(~main_u16x)+1;
    showing_data[4] = Dpy_wei_4 & Dpy_duan_negative;
  }else
  {
    showing_data[4] = Dpy_wei_4 & Dpy_duan_null;
  }
  
  showing_data[6] = Dpy_wei_6 & duanma[mod()];
  showing_data[5] = Dpy_wei_5 & duanma[mod()] & Dpy_duan_dp;
  
  if(LOW(main_u16x) = mod())
  {
    showing_data[4] = Dpy_wei_4 & duanma[LOW(main_u16x)];
  }
  
  set_brightness(main_u8x,_Dpy_wei_4 | _Dpy_wei_5 | _Dpy_wei_6);
}


void (*fp_display_PWM_action)();
//用main_u16x传参要显示的数值，用main_x传参亮度
void display_PWM_value()
{
  if(fp_display_PWM_action)
  {
    PWM = main_u16x;
    fp_display_PWM_action();//用此函数设置PWM输出
  }
  showing_data[6] = Dpy_wei_6 & duanma[mod()];
  showing_data[5] = Dpy_wei_5 & duanma[mod()];
  showing_data[4] = Dpy_wei_4 & duanma[mod()];
  showing_data[3] = Dpy_wei_3 & duanma[mod()];
  showing_data[2] = Dpy_wei_2 & duanma[mod()];
  
  set_brightness(main_u8x,_Dpy_wei_2 | _Dpy_wei_3 | _Dpy_wei_4 | _Dpy_wei_5 | _Dpy_wei_6);
}


void chkbmq(){//判断编码器状态
  if(bmq_turning){
    if(PB4I == 0){
      bmq_status1 = 0;
      bmq_last_up = 0;
      bmq_idel_time = 0;
    }else if(PB5I== 0){
      bmq_status2 = 0;
      bmq_last_up = 1;
      bmq_idel_time = 0;
    }
  }else{
    if(PB4I == 0){
      bmq_turning = 1;
      bmq_status1 = 0;
      bmq_frist_down = 0;
      bmq_last_up = 0;
      bmq_idel_time = 0;
    }else if(PB5I== 0){
      bmq_turning = 1;
      bmq_status2 = 0;
      bmq_frist_down = 1;
      bmq_last_up = 1;
      bmq_idel_time = 0;
    }
  }
}

#pragma optimize = low
void bmq_wait_event()
{
  btn_event = 0;
  user_timer1 = 10;
  do{
    chkbmq();
    if(btn_event) 
	{
		if(btn_event == TURN_BUTTONDOWN_LONG)
		{
			while(btn_status);
			btn_event = TURN_BUTTONDOWN_LONG;
		}	
		return;
	} 
  }while(user_timer1);
}

s16 bmq_turn_mgr_seting_data;
u8  bmq_turn_mgr_speed_coefficient;
s16 bmq_turn_mgr_number_upper_limit;
s16 bmq_turn_mgr_number_lower_limit;
void (*fp_bmq_turn_mgr_display)();


#pragma optimize = low
static void limit_and_restet_time_out()
{
	main_u8x = (bmq_last2_time > 252) ? 252 : bmq_last2_time; 
	flashing_FSM = 1;
    UI_time_out = 0;
}

void bmq_turn_mgr()
{
  if(fp_bmq_turn_mgr_display)
  {
    main_u16x = bmq_turn_mgr_seting_data;//传参给下面这个函数
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    fp_bmq_turn_mgr_display();
  }
  
  bmq_wait_event();
  
  switch(btn_event)
  {
    
  case TURN_RIGHT://编码器正转
    
    limit_and_restet_time_out();
	
    bmq_turn_mgr_seting_data  +=  ((252-main_u8x)/bmq_turn_mgr_speed_coefficient) + 1;

	if(bmq_turn_mgr_seting_data > bmq_turn_mgr_number_upper_limit)
    {
      bmq_turn_mgr_seting_data = bmq_turn_mgr_number_upper_limit;
    }
    return;
  
  case TURN_LEFT://编码器反转
    
	limit_and_restet_time_out();

    bmq_turn_mgr_seting_data -= ((252-main_u8x)/bmq_turn_mgr_speed_coefficient) + 1;

	if(bmq_turn_mgr_seting_data < bmq_turn_mgr_number_lower_limit)
    {
      bmq_turn_mgr_seting_data = bmq_turn_mgr_number_lower_limit;
    }
    return;
	
  default:
  	
    FSM_Reverse();
    return;
  }
}

s16 factory_mode_seting_PWM_data;
/******************************************************************************/
//factory_mode_seting_PWM 无超时返回
/******************************************************************************/
 
static void  seting_PWM_entry()
{
	flashing_FSM = 1;//闪烁状态机
	flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
        return;
}

static void seting_PWM_befor()
{
	 
  if(factory_mode_setV_or_setI){//电流PWM
    PWM = 800;//输出5V左右
    set_V_PWM();
    fp_display_PWM_action = set_I_PWM;
  }else{//电压PWM
    PWM = 5000;//输出25A左右
    set_I_PWM();
    fp_display_PWM_action = set_V_PWM;
  }
}
static void seting_PWM_after()
{
	PWM = 0;
	set_I_PWM();
	set_V_PWM();
}

void factory_mode_seting_PWM()
{
	bqm_fucn(
		DATA_PWM,
		COEFFICIENT_PWM, //coefficient
		MAX_PWM_DATA,
		MIN_PWM_DATA,
		display_PWM_value,
		seting_PWM_entry,
		seting_PWM_befor,
		NULL,/*while*/
		seting_PWM_after,
		0
	);
}

void bqm_fucn(
	u8 idx,
	u8 coefficient,
	s16 upper_limit,
	s16 lower_limit,
	fp_display callback_disp,
	fp_entry callback_entry,
	fp_before callback_before,
	fp_while callback_while,
	fp_after callback_after,
	u8 time_out
)
{
	if(callback_entry) callback_entry();
	switch(idx)
	{
		case DATA_SET_V_LIMIT:
			bmq_turn_mgr_seting_data = set_V_limit; break;
		case DATA_SET_I_LIMIT:
			bmq_turn_mgr_seting_data = set_I_limit; break;
		case DATA_SET_V:
		case DATA_SET_V_REF:
			bmq_turn_mgr_seting_data = setV; break;
		case DATA_SET_I:
		case DATA_SET_I_REF:
			bmq_turn_mgr_seting_data = setI; break;
		case DATA_PWM:
			bmq_turn_mgr_seting_data = factory_mode_seting_PWM_data; break;
		case DATA_MENU:
			bmq_turn_mgr_seting_data = 1; break;
		default:
			bmq_turn_mgr_seting_data = seting_data[idx]; break;
	}
	
	bmq_turn_mgr_speed_coefficient = coefficient;
	bmq_turn_mgr_number_upper_limit = upper_limit;
	bmq_turn_mgr_number_lower_limit = lower_limit;
	fp_bmq_turn_mgr_display = callback_disp;

	if(callback_before) callback_before();
	
	UI_time_out = TIMEOUT_COUNTS - time_out;
	while(1)
	{
		bmq_turn_mgr();
		
		if(btn_event == TURN_BUTTONDOWN_SHORT) //短按
		{
			switch(idx)
			{
				case DATA_SET_V_LIMIT:
					set_V_limit = bmq_turn_mgr_seting_data; break;
				case DATA_SET_I_LIMIT:
					set_I_limit = bmq_turn_mgr_seting_data; break;
				case DATA_SET_V: //pass through
				case DATA_SET_V_REF:
					setV = bmq_turn_mgr_seting_data; break;
				case DATA_SET_I:
				case DATA_SET_I_REF:
					setI = bmq_turn_mgr_seting_data; break;
				case DATA_PWM:
					factory_mode_seting_PWM_data = bmq_turn_mgr_seting_data; break;
				case DATA_MENU:
					main_u8x = (u8)bmq_turn_mgr_seting_data;
					break;
				default:
					seting_data[idx] = bmq_turn_mgr_seting_data; break;
			}
			
			if(callback_after) callback_after();
			return;
		}
		if(callback_while) callback_while();
		if(time_out == 0) continue;
		if(UI_timeout_timer()) //5秒无动作
		{
			return;;
		}
	}
}


void main_menu(
	u8 state_turn_left,
	u8 state_turn_righ,
	u8 state_turn_btdn,
	u8 state_turn_long,
	u8 state_turn_longlong,
	fp_menu_entry callback_entry,
	fp_menu_while callback_while,
	u8 time_out
)
{
	if(callback_entry) callback_entry();
	UI_time_out = 0;
	while(1)
	{
		if(callback_while) callback_while();
		bmq_wait_event();
		switch(btn_event)
		{
			case TURN_LEFT://编码器反转
				next_state = state_turn_left; return;
			case TURN_RIGHT://编码器正转
				next_state = state_turn_righ; return;
			case TURN_BUTTONDOWN_SHORT://短按
				next_state = state_turn_btdn; return;
			case TURN_BUTTONDOWN_LONG://长按
				next_state = state_turn_long; return;
			case TURN_BUTTONDOWN_LONGLONG://长按
				next_state = state_turn_longlong; return;
			default: break;
		}
		FSM_Reverse();
		if(time_out)
		{
			if(UI_timeout_timer())
			{
			  next_state = MAIN_UI; return; //5秒无动作
			}
		}
	}
}


void factory_fucn(
	
	fp_factory_entry factory_entry,
	fp_factory_after factory_after,
	
	fp_factory_turnleft callback_turnleft,
	fp_factory_turnright callback_turnright,
	fp_factory_btndown factory_btndown
)
{
	if (factory_entry) factory_entry();
	while(1)
	{
		if (factory_after) factory_after();
		bmq_wait_event();
		switch(btn_event)
		{
			case TURN_LEFT://编码器反转
				//next_state = STATE_NONE;
				if (callback_turnleft) callback_turnleft();
				break;
			case TURN_RIGHT://编码器正转
				//next_state = STATE_NONE;
				if (callback_turnright) callback_turnright();
				break;
			case TURN_BUTTONDOWN_SHORT://短按
				//next_state = state_btndown;
				if (factory_btndown) factory_btndown();
				return;
			default:
			FSM_Reverse();
		}
	}
}  

/**
  ******************************************************************************
  * EEPROM fuction
  ******************************************************************************  
  */  

s16 eeprom_buf1;
s16 eeprom_buf2;
void eeprom_read_addrx8()
{
  eep_addr=(u8 *)(EEPROM_ADDRESS+(8*main_u8x));//用main_x传参确定地址
  
  eeprom_buf1=*(s16 *)eep_addr;
  if(eeprom_buf1 != ((*(s16 *)(eep_addr+2))^0xADB5)){
    eeprom_buf1 = 0;
  }
  
  eeprom_buf2=*(s16 *)(eep_addr+4);
  if(eeprom_buf2 != ((*(s16 *)(eep_addr+6))^0xADB5)){
    eeprom_buf2 = 0;
  }
}

void eeprom_write_unlock_addrx8(){
  FLASH_DUKR = 0xAE;//EEPROM解锁密码
  FLASH_DUKR = 0x56;//EEPROM解锁密码
  while(FLASH_IAPSR_DUL == 0);//等待解密就绪

  eep_addr=(u8 *)(EEPROM_ADDRESS+(8*main_u8x));//用main_x传参确定地址
}

void eeprom_write(){
  FLASH_CR2_WPRG = 1;//启用字（4字节）写入模式
  FLASH_NCR2_NWPRG = 0;//启用字（4字节）写入模式
  *eep_addr++ = HIGH(eeprom_buf1);
  *eep_addr++ = LOW(eeprom_buf1);
  *eep_addr++ = HIGH(eeprom_buf1)^0xAD;
  *eep_addr++ = LOW(eeprom_buf1)^0xB5;
}

