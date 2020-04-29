#include "iostm8s103f3.h"
#include "general.h"
#include "driver.h"
#include "it.h"
#include "display.h"
#include "main.h"
#include "parameter.h"
#include "led.h"


#define _DEBUG_PWM 

static void set_display_style_WEAK()
{
  flashing_FSM = 1;//闪烁状态机
  flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
}

static void set_display_style_STRONG()
{
  flashing_FSM = 1;//闪烁状态机
  flashing_style[0] = STRONGBLINK;//灭<->全亮闪烁模式
}

static void shutdown_PWM()
{
  setV = 0;
  setI = 0;
  output_PWM_update = 1;
}

/******************************************************************************/  
//FACTORY_MODE_MENU:
/******************************************************************************/

static void factory_mode_menu_entry()
{
  set_display_style_STRONG();
  clear_by_null();
  pos = 0; // for show_str
}

static void factory_mode_menu_after()
{
  main_u16x = seting_data[0];//菜单数字（1~6）
  main_u8x = flashing_style[flashing_FSM];
  display_left_1_digital();

  pos = show_str(str_list[LOW(seting_data[0]) - 1], pos);//刷新显示
}

static void factory_mode_menu_turnleft()
{
  pos = 0;
  --seting_data[0];
  if(seting_data[0] < 1) seting_data[0] = 1;
  flashing_FSM = 1;
}

static void factory_mode_menu_turnright()
{
  pos = 0;
  ++seting_data[0];
  if(seting_data[0] > 6) seting_data[0] = 6;
  flashing_FSM = 1;
}

static void factory_mode_menu_btndown()
{
  switch(LOW(seting_data[0])){
  case 1:
    factory_mode_setV_or_setI = 0;
    next_state = FACTORY_MODE_SET_0_5V_OR_0_5A_REF; break;
  case 2:
    next_state = STATE_2_STEP0; break;
  case 3:
    factory_mode_setV_or_setI = 5;
    next_state = FACTORY_MODE_SET_0_5V_OR_0_5A_REF; break;
  case 4:
    next_state = STATE_4_STEP0; break;
  case 5:
    next_state = FACTORY_MODE_SAVE_OR_CANSEL; break;
  case 6:
    next_state = START_UP; break;
  default:
    break;
  }
}

void show_factory_mode_menu()
{
  factory_fucn(
    factory_mode_menu_entry,
    factory_mode_menu_after,
    factory_mode_menu_turnleft,
    factory_mode_menu_turnright,
    factory_mode_menu_btndown
  );
}

/******************************************************************************/  
//FACTORY_MODE_SET_0_5V_OR_0_5A_REF:
/******************************************************************************/
static void set_0p5V_or_0p5A_ref_entry()
{
  //flashing_FSM = 0;//闪烁状态机
  //flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  
  set_display_style_STRONG();
  
  showing_data[1] = DARK;
  showing_data[2] = DARK;
  
  fp_display_PWM_action = NULL;
}

static void set_0p5V_or_0p5A_ref_after()
{
  main_u16x = seting_data[1 + factory_mode_setV_or_setI];//传参给下面这个函数
  main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
  display_PWM_value();
}

static void set_0p5V_or_0p5A_ref_callback_btndown()
{
  factory_mode_seting_PWM_data = seting_data[1 + factory_mode_setV_or_setI];//初始值
  
  factory_mode_seting_PWM();
  
  if(factory_mode_setV_or_setI)
  {//电流
    #ifdef _DEBUG_PWM
    seting_data[6] = 99;//factory_mode_seting_PWM_data;
    seting_data[7] = 9*64;//nowI_16bit_ADC_result;
    #else
    seting_data[6] = factory_mode_seting_PWM_data;
    seting_data[7] = nowI_16bit_ADC_result;
    #endif
  }else
  {//电压
    #ifdef _DEBUG_PWM
    seting_data[1] = 55;//factory_mode_seting_PWM_data;
    seting_data[2] = 6*64;//nowI_16bit_ADC_result;
    #else
    seting_data[1] = factory_mode_seting_PWM_data;
    seting_data[2] = nowI_16bit_ADC_result;
    #endif
  }
  next_state = FACTORY_MODE_MENU;
}

void do_set_0p5V_or_0p5A_ref()
{
  factory_fucn(
    set_0p5V_or_0p5A_ref_entry,
    set_0p5V_or_0p5A_ref_after,
    NULL,
    NULL,
    set_0p5V_or_0p5A_ref_callback_btndown
  );
}

/******************************************************************************/  
//STATE_2_STEP0
/******************************************************************************/  

static void display_two_zero(u8 which)
{

  u8 bright;

  bright = flashing_FSM ? STRONGBLINK : FULLBRIGHT;
  dark012();
  
  if(which == 2)
  {
    showing_data[2] = Dpy_wei_2 & Dpy_duan_0;
    showing_data[3] = DARK;
    set_brightness(bright, _Dpy_wei_2 );//显示左方框
  }
  else if(which == 3)
  {
    showing_data[2] = DARK;
    showing_data[3] = Dpy_wei_3 & Dpy_duan_0;
    set_brightness(bright, _Dpy_wei_3 );//显示右方框
  }
  else
  {
    showing_data[2] = Dpy_wei_2 & Dpy_duan_0;//显示左方框
    showing_data[3] = Dpy_wei_3 & Dpy_duan_0;//显示右方框
    set_brightness(bright, _Dpy_wei_2 | _Dpy_wei_3 );
  }
}

static void frame_menu_V_entry()
{
  clear_by_null();
  shutdown_PWM();
}

static void frame_menu_V_after()
{
  display_two_zero(main_u8x);
}

static void frame_menu_V_callback_turnleft()
{
  main_u8x = Vlim;
}
static void frame_menu_V_callback_turnright()
{
  main_u8x = Vcal;
}
static void frame_menu_V_callback_btndown()
{
  if(main_u8x == Vlim)
    next_state = STATE_2_STEP1; //设置电压限制
  else if(main_u8x == Vcal && set_V_limit != 0)
    next_state = STATE_2_STEP2; //设置校准电压
  else 
    next_state = STATE_2_STEP0;
}

void do_factory_frame_menu_V()
{
  factory_fucn(
    frame_menu_V_entry,
    frame_menu_V_after,
    frame_menu_V_callback_turnleft,
    frame_menu_V_callback_turnright,
    frame_menu_V_callback_btndown
  );
}


/******************************************************************************/  
//STATE_2_STEP1
/******************************************************************************/  
static void callback_set_V_limit_entry()
{
  set_display_style_WEAK();
  dark0123();
  
  main_u8x = EEPROM_ADDR_START_LIMIT;
  eeprom_read_addrx8();
  set_V_limit = eeprom_buf1;
}

static void callback_set_V_limit_after()
{
  set_V_limit = bmq_turn_mgr_seting_data;
  dark012();
}
  
void do_factory_set_V_limit()
{
  bqm_fucn(
    DATA_SET_V_LIMIT,
    COEFFICIENT_SETING_V_LIMIT,
    MAX_V_LIMIT,
    MIN_V_LIMIT,
    display_left_4_digital,
    callback_set_V_limit_entry,
    NULL,/*before*/
    NULL,/*while*/
    callback_set_V_limit_after,
    TIMEOUT_COUNTS //超时5s
  );
}
/******************************************************************************/  
//STATE_2_STEP2
/******************************************************************************/  
static void callback_set_V_ref_entry()
{
  set_display_style_WEAK();
  dark0123();
  setV = seting_data[3];
}

static void callback_set_V_ref_after()
{
  seting_data[3] = setV;
  output_PWM_update = 1;
}
  
void do_factory_set_V_ref()
{
  bqm_fucn(
    DATA_SET_V_REF,
    3,//COEFFICIENT_SETING_V_REF,
    set_V_limit,
    MIN_V_LIMIT,
    display_left_4_digital,
    callback_set_V_ref_entry,
    NULL,/*before*/
    NULL,/*while*/
    callback_set_V_ref_after,
    TIMEOUT_COUNTS //超时5s
  );
}
/******************************************************************************/  
//STATE_2_STEP3 : 校准
/******************************************************************************/  
static void set_cal_basic()
{
  set_display_style_STRONG();
  dark012();
  
  fp_display_PWM_action = NULL;
}
static void do_factory_cal_V_ref_entry()
{
  set_cal_basic();
}

static void do_factory_cal_V_ref_after()
{
  main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
  main_u16x = seting_data[4];//传参给下面这个函数
  display_PWM_value();
}

static void do_factory_cal_V_ref_callback_btndown()
{
  factory_mode_seting_PWM_data = seting_data[4];//初始值
  
  factory_mode_seting_PWM();
  
  //电压
  #ifdef _DEBUG_PWM
  seting_data[4] = 9999;
  seting_data[5] = 1023*32;
  #else
  seting_data[4] = factory_mode_seting_PWM_data;
  seting_data[5] = nowI_16bit_ADC_result;
  #endif
  
  next_state = STATE_4_STEP0;
}

void do_factory_cal_V_ref()
{
/*  factory_fucn(
    do_factory_cal_V_ref_entry,
    do_factory_cal_V_ref_after,
    NULL,
    NULL,
    do_factory_cal_V_ref_callback_btndown
    );
*/
  do_factory_cal_V_ref_entry();
  do_factory_cal_V_ref_after();
  do_factory_cal_V_ref_callback_btndown();
}

/******************************************************************************/  
//STATE_4_STEP0
/******************************************************************************/  

static void frame_menu_I_entry()
{
  clear_by_null();
  shutdown_PWM();
}

static void frame_menu_I_after()
{
  display_two_zero(main_u8x);
}

static void frame_menu_I_callback_turnleft()
{
  main_u8x = Ilim;
}
static void frame_menu_I_callback_turnright()
{
  main_u8x = Ical;
}
static void frame_menu_I_callback_btndown()
{
  if(main_u8x == Ilim)
    next_state = STATE_4_STEP1; //设置电流极限
  else if(main_u8x == Ical && set_I_limit != 0)
    next_state = STATE_4_STEP2; //设置校准电流
  else 
    next_state = STATE_4_STEP0;
}

void do_factory_frame_menu_I()
{

 factory_fucn(
  frame_menu_I_entry,
  frame_menu_I_after,
  frame_menu_I_callback_turnleft,
  frame_menu_I_callback_turnright,
  frame_menu_I_callback_btndown
  );
}


/******************************************************************************/  
//STATE_4_STEP1
/******************************************************************************/  
static void callback_set_I_limit_entry()
{
  set_display_style_WEAK();
  dark0123();
  
  main_u8x = EEPROM_ADDR_START_LIMIT;
  eeprom_read_addrx8();
  set_I_limit = eeprom_buf2;
}

static void callback_set_I_limit_after()
{
  set_I_limit = bmq_turn_mgr_seting_data;
}
  
void do_factory_set_I_limit()
{
  bqm_fucn(
    DATA_SET_I_LIMIT,
    COEFFICIENT_SETING_I_LIMIT,
    MAX_I_LIMIT,
    MIN_I_LIMIT,
    display_right_3_digital,
    callback_set_I_limit_entry,
    NULL,/*before*/
    NULL,/*while*/
    callback_set_I_limit_after,
    TIMEOUT_COUNTS //超时5s
  );
}
/******************************************************************************/  
//STATE_4_STEP2
/******************************************************************************/  
static void callback_set_I_ref_entry()
{
  set_display_style_WEAK();
  setI = seting_data[8];
  dark23();
}

static void callback_set_I_ref_after()
{
  seting_data[8] = setI;
  output_PWM_update = 1;
}
  
void do_factory_set_I_ref()
{
  bqm_fucn(
    DATA_SET_I_REF,
    COEFFICIENT_SETING_I_REF,
    set_I_limit,
    MIN_I_LIMIT,
    display_right_3_digital,
    callback_set_I_ref_entry,
    NULL,/*before*/
    NULL,/*while*/
    callback_set_I_ref_after,
    TIMEOUT_COUNTS //超时5s
  );
}
/******************************************************************************/  
//STATE_4_STEP3
/******************************************************************************/  

static void do_factory_cal_I_ref_entry()
{
  set_cal_basic();
}

static void do_factory_cal_I_ref_after()
{
  main_u8x = flashing_style[flashing_FSM];
  main_u16x = seting_data[9];
  display_PWM_value();
}

static void do_factory_cal_I_ref_callback_btndown()
{
  factory_mode_seting_PWM_data = seting_data[9];//初始值
  
  factory_mode_seting_PWM();
  
  //电流
  #ifdef _DEBUG_PWM
  seting_data[9] = 9999;
  seting_data[10] = 1023*32;
  #else
  seting_data[9] = factory_mode_seting_PWM_data;
  seting_data[10] = nowI_16bit_ADC_result;
  #endif
  next_state = FACTORY_MODE_PREV_MENU;
}

void do_factory_cal_I_ref()
{
/*  factory_fucn(
    do_factory_cal_I_ref_entry,
    do_factory_cal_I_ref_after,
    NULL,
    NULL,
    do_factory_cal_I_ref_callback_btndown
    );
*/
  do_factory_cal_I_ref_entry();
  do_factory_cal_I_ref_after();
  do_factory_cal_I_ref_callback_btndown();
}


/******************************************************************************/  
//factory_mode_save_or_cansel:
/******************************************************************************/

static void factory_mode_save_entry()
{
//flashing_FSM = 0;
//flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
set_display_style_STRONG();
}

static void factory_mode_save_after()
{
    //改变显示特征
    if(flashing_FSM) //编码器空闲期间会在0和1之间来回翻
  {
      HC595_data_mask[0] = 0xFFFF;//所有数码管工作
      HC595_data_mask[1] = 0xFFFF;//所有数码管工作
    }else{
      HC595_data_mask[0] = Dpy_wei_0;//只有最左边数码管工作
      HC595_data_mask[1] = Dpy_wei_0;//只有最左边数码管工作
    }
}
static void factory_mode_save_btndown()
{
  main_u8x = EEPROM_ADDR_START_LIMIT;
  eeprom_write_unlock_addrx8();

  eeprom_buf1 = set_V_limit;  eeprom_write();
  eeprom_buf1 = set_I_limit;  eeprom_write();

  for(pos = 1; pos < 11; ++pos)
  {
    eeprom_buf1 = seting_data[pos];  eeprom_write();
  }

  calculation_factory_parameters();

  eeprom_buf1 = PWM_V_bias;          eeprom_write();
  eeprom_buf1 = PWM_V_coefficient;   eeprom_write();
  eeprom_buf1 = PWM_I_bias;          eeprom_write();
  eeprom_buf1 = PWM_I_coefficient;   eeprom_write();
  eeprom_buf1 = ADC_V_bias;          eeprom_write();
  eeprom_buf1 = ADC_V_coefficient;   eeprom_write();
  eeprom_buf1 = ADC_I_bias;          eeprom_write();
  eeprom_buf1 = ADC_I_coefficient;   eeprom_write();

  eeprom_write_lock();

  next_state = START_UP;
}

void do_factory_mode_save()
{
  factory_fucn(
    factory_mode_save_entry,
    factory_mode_save_after,
    NULL,
    NULL,
    factory_mode_save_btndown
  );
}

