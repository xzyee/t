#include "iostm8s103f3.h"
#include "my.h"
#include "it.h"
#include "main.h"


/*
    【EEPROM空间分配情况】
    0-7         开机默认电压电流         8字节
    8-79        9组存储电压电流          72字节
    80-119      工厂模式的显式参数       40字节
    120-151     工厂模式的隐式参数       32字节
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

u8 smg_scan = 0;
u8 smg_scan2 = 0;

static u8 bmq_status1 = 1;
static u8 bmq_status2 = 1;

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
//*/

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
  if(brightness <= 1){
    HC595_data_mask[1] = ~mask;
  }else{
    HC595_data_mask[0] = 0xFFFF;
    HC595_data_mask[1] = 0xFFFF;
    return;
  }
  if(brightness == 0){
    HC595_data_mask[0] = HC595_data_mask[1];
    return;
  }
  HC595_data_mask[0] = 0xFFFF;
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

//求模运算，用main_u16x传参
u8 mod(){
  u8 result;
  result = main_u16x % 10;
  main_u16x /= 10;
  return result;
}

//str是字符串指针，pos是字符串的当前要显示的字符位置
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
  {
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

void FSM_Reverse(){
  flashing_FSM = !flashing_FSM;
}

//用main_u16x传参要显示的数值，用main_x传参亮度
void display_left_1_digital()
{
  showing_data[0] = Dpy_wei_0 & duanma[ (u8)main_u16x ];
  
  set_brightness(main_u8x,_Dpy_wei_0);
}

//用main_u16x传参要显示的数值，用main_x传参亮度
void display_left_3_digital()
{
  if(main_u16x & 0x8000){//是负数
    main_u16x = (~main_u16x) + 1;
    showing_data[1] = Dpy_wei_1 & Dpy_duan_negative;
  }else{
    showing_data[1] = Dpy_wei_1 & Dpy_duan_null;
  }
  showing_data[3] = Dpy_wei_3 & duanma[mod()];
  showing_data[2] = Dpy_wei_2 & duanma[mod()] & Dpy_duan_dp;
  
  if(LOW(main_u16x) = mod()){
    showing_data[1] = Dpy_wei_1 & duanma[LOW(main_u16x)];
  }
  
  set_brightness(main_u8x,_Dpy_wei_1 | _Dpy_wei_2 | _Dpy_wei_3);
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


void (*fp_display_PWM_value)();
//用main_u16x传参要显示的数值，用main_x传参亮度
void display_PWM_value()
{
  if(fp_display_PWM_value)
  {
    PWM = main_u16x;
    fp_display_PWM_value();//用此函数设置PWM输出
  }
  showing_data[6] = Dpy_wei_6 & duanma[mod()];
  showing_data[5] = Dpy_wei_5 & duanma[mod()];
  showing_data[4] = Dpy_wei_4 & duanma[mod()];
  showing_data[3] = Dpy_wei_3 & duanma[mod()];
  
  set_brightness(main_u8x,_Dpy_wei_3 | _Dpy_wei_4 | _Dpy_wei_5 | _Dpy_wei_6);
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
    if(btn_event) return;
  }while(user_timer1);
}

s16 bmq_turn_mgr_seting_data;
u8 bmq_turn_mgr_speed_coefficient;
s16 bmq_turn_mgr_number_upper_limit;
s16 bmq_turn_mgr_number_lower_limit;
void (*fp_bmq_turn_mgr_display)();

#pragma optimize = low
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
    
  case 0x01://编码器正转
    
    if(bmq_last2_time > 252)
    {
      main_u8x = 252;
    }
    else
    {
      main_u8x = bmq_last2_time;
    }
    
    bmq_turn_mgr_seting_data  +=  ((252-main_u8x)/bmq_turn_mgr_speed_coefficient) + 1;
    if(bmq_turn_mgr_seting_data > bmq_turn_mgr_number_upper_limit)
    {
      bmq_turn_mgr_seting_data = bmq_turn_mgr_number_upper_limit;
    }
    
    flashing_FSM = 1;
    UI_time_out = 0;
    return;
  
  case 0xFF://编码器反转
    
    if(bmq_last2_time>252)
    {
      main_u8x = 252;
    }
    else
    {
      main_u8x = bmq_last2_time;
    }

    bmq_turn_mgr_seting_data -= ((252-main_u8x)/bmq_turn_mgr_speed_coefficient) + 1;
    if(bmq_turn_mgr_seting_data < bmq_turn_mgr_number_lower_limit)
    {
      bmq_turn_mgr_seting_data = bmq_turn_mgr_number_lower_limit;
    }
    
    flashing_FSM = 1;
    UI_time_out = 0;
    return;
	
  default:
    FSM_Reverse();
    return;
  }
}

s16 factory_mode_seting_PWM_PWMdata;
void factory_mode_seting_PWM()
{
  flashing_FSM = 1;//闪烁状态机
  
  flashing_style[0] = 1;//半亮<->全亮闪烁模式
  
  bmq_turn_mgr_seting_data = factory_mode_seting_PWM_PWMdata;
  bmq_turn_mgr_speed_coefficient = 1;
  bmq_turn_mgr_number_upper_limit = 9999;
  bmq_turn_mgr_number_lower_limit = 0;
  fp_bmq_turn_mgr_display = display_PWM_value;
  
  if(factory_mode_setV_or_setI){//电流PWM
    PWM = 800;//输出5V左右
    set_V_PWM();
    fp_display_PWM_value = set_I_PWM;
  }else{//电压PWM
    PWM = 5000;//输出25A左右
    set_I_PWM();
    fp_display_PWM_value = set_V_PWM;
  }
  
  while(1){
    bmq_turn_mgr();
    
    if(btn_event == 0x02)
    {
      factory_mode_seting_PWM_PWMdata = bmq_turn_mgr_seting_data;
          
      PWM = 0;
      set_I_PWM();
      set_V_PWM();
      
      return;
    }
    
  }
}

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
