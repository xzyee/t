#include "iostm8s103f3.h"
#include "general.h"
#include "it.h"
#include "driver.h"
#include "display.h"


u16 setV = 0;//main线程专用，最终由TIM4实行
u16 setI = 0;//main线程专用，最终由TIM4实行

u8 is_output_ON = 0;
u8 output_PWM_update = 0;//此变量置1即可更新输出PWM，自动清零

u16 nowV_16bit_ADC_result = 0;
u16 nowV = 0;//比例：537 = 53.7V

u16 nowI_16bit_ADC_result = 0;
u16 nowI = 0;//比例：125 = 12.5A

s16 seting_data[11];

u8 factory_mode_setV_or_setI;//工厂模式专用变量，0 = 设置电压（对应菜单项1和2），5 = 设置电流（对应菜单项3和4）。FLASH空间不足了，逼我做成代码复用....

u8 flashing_style[2] = {FULLBRIGHT,FULLBRIGHT}; //0=灭<->亮循环，1=半亮<->全亮循环,2=常亮

u8 main_u8x;//此变量为临时多用变量，只能在main线程内使用，不允许跨函数使用
u16 main_u16x;//此变量为临时多用变量，只能在main线程内使用，不允许跨函数使用

u8 pos;

static void Init();

#include "led.c"

#pragma optimize = low
/**
  ******************************************************************************
  * main()
  ******************************************************************************  
  */
void main()
{
  CLK_CKDIVR_HSIDIV = 0;//主时钟不分频

  Init();

  asm("rim");//开全局中断
  
/**
  ******************************************************************************
  * goto FACTORY_MODE;
  ******************************************************************************  
  */  
  
  {//一秒钟内全亮数码管
    showing_data[0] = Dpy_wei_0 & Dpy_duan_all;
    showing_data[1] = Dpy_wei_1 & Dpy_duan_all;
    showing_data[2] = Dpy_wei_2 & Dpy_duan_all;
    showing_data[3] = Dpy_wei_3 & Dpy_duan_all;
    showing_data[4] = Dpy_wei_4 & Dpy_duan_all;
    showing_data[5] = Dpy_wei_5 & Dpy_duan_all;
    showing_data[6] = Dpy_wei_6 & Dpy_duan_all;
    user_timer1 = 40;
    do{}while(user_timer1);
  }
  
/**
  ******************************************************************************
  * 按住编码器再开机的，跳到工厂模式
  ******************************************************************************  
  */  
  if(PA3I == 0){
    pos = 0;
    showing_data[0] = 0x0000;
    showing_data[1] = Dpy_wei_1 & Dpy_duan_null;
    showing_data[2] = Dpy_wei_2 & Dpy_duan_null;
    showing_data[3] = Dpy_wei_3 & Dpy_duan_null;
    showing_data[4] = Dpy_wei_4 & Dpy_duan_null;
    showing_data[5] = Dpy_wei_5 & Dpy_duan_null;
    showing_data[6] = Dpy_wei_6 & Dpy_duan_null;
    while(1){
      pos = show_str(str_0,pos);//刷新显示
      
      user_timer1 = 10;
      do{
        if(btn_event == 0) continue; //用户无动作
        switch(btn_event){
        case 0x02://短按
          goto FACTORY_MODE;
        case 0x03://长按
          if(PA3I){
            btn_event = 0;
            goto FACTORY_MODE;
          }
          break;
        default:
          btn_event = 0;
        }
      }while(user_timer1);
    }
  }

/******************************************************************************/
START_UP:
/******************************************************************************/
  {//读取EEPROM工厂模式的参数
    main_u8x = 15;
    eeprom_read_addrx8();
    PWM_V_bias = eeprom_buf1;
    PWM_V_coefficient = eeprom_buf2;
    
    main_u8x = 16;
    eeprom_read_addrx8();
    PWM_I_bias = eeprom_buf1;
    PWM_I_coefficient = eeprom_buf2;
    
    main_u8x = 17;
    eeprom_read_addrx8();
    ADC_V_bias = eeprom_buf1;
    ADC_V_coefficient = eeprom_buf2;
    
    main_u8x = 18;
    eeprom_read_addrx8();
    ADC_I_bias = eeprom_buf1;
    ADC_I_coefficient = eeprom_buf2;
  }
  
  {//读取EEPROM默认电压电流
    main_u8x = 0;
    eeprom_read_addrx8();
    setV = eeprom_buf1;
    setI = eeprom_buf2;
    output_PWM_update = 1;
  }
  
  //flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  flashing_FSM = 1;
  UI_time_out = 0;
  showing_data[0] = 0x0000;
  
  main_u8x = FULLBRIGHT;//传参给下面这个函数
  main_u16x = setV;//传参给下面这个函数
  display_left_3_digital();
  
  main_u16x = setI;//传参给下面这个函数
  display_right_3_digital();
  
  while(1){//开机延迟5秒再输出电压
    if(flashing_FSM){//亮
      HC595_data_mask[0] = 0xFFFF;
      HC595_data_mask[1] = 0xFFFF;
    }else{//灭
      HC595_data_mask[0] = 0x0000;
      HC595_data_mask[1] = 0x0000;
    }
    
    bmq_wait_event();
    if(btn_event){
      if(btn_event== 0x02){//按钮短按
        
      }
      goto MAIN_UI;
    }
        
    FSM_Reverse();
    
    if(UI_timeout_timer()){
      //5秒无动作，启动输出，跳到主菜单
      is_output_ON = 1;
      output_PWM_update = 1;
      goto MAIN_UI;
    }
  }
  
/******************************************************************************/  
MAIN_UI:
/******************************************************************************/
  
  flashing_FSM = 0;//闪烁状态机
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  UI_time_out = 0;
  pos = 0;
  while(1){//默认主界面
    showing_data[0] = 0x0000;
    
    main_u8x = FULLBRIGHT;//传参给下面这个函数
    main_u16x = nowV;//传参给下面这个函数
    display_left_3_digital();
    
    if(is_output_ON){
      flashing_FSM = 1;
      
      if(_setI!=setI){//限流保护状态，电流闪烁指示
        pos++;
        if(!(pos&0x05)){
          flashing_FSM = 0;
        }
      }
      
      main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
      main_u16x = nowI;//传参给下面这个函数
      display_right_3_digital();
      
    }else{
      showing_data[4] = Dpy_wei_4&Dpy_duan_o;
      showing_data[5] = Dpy_wei_5&Dpy_duan_f;
      showing_data[6] = Dpy_wei_6&Dpy_duan_f;
    }
    //HC595_data_mask[0] = 0xFFFF;
    //HC595_data_mask[1] = 0xFFFF;
    
    btn_event = 0;
    user_timer1 = 6;
    do{
      chkbmq();
      if(btn_event== 0)continue;
      switch(btn_event){
      case 0x01://编码器正转
        
        goto SET_I;
      case 0x02://按钮短按 = 开启输出/关闭输出
        
        is_output_ON=!is_output_ON;//打开输出/关闭输出
        output_PWM_update = 1;
        goto MAIN_UI;
      case 0xFF://编码器反转
        
        goto set_V;
      case 0x03://按钮长按
        
        goto SETING_STORAGE_WRITE;
      //default:
        
      }
    }while(user_timer1);
  }
  
/******************************************************************************/
CONTINUOUS_ADJUSTMENT_MODE_V:
/******************************************************************************/

  showing_data[0] = 0x0000;
  is_output_ON = 1;
  UI_time_out = 0;
  pos = 0;
  
  flashing_FSM = 1;//闪烁状态机
  flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
  
  bmq_turn_mgr_seting_data = setV;
  bmq_turn_mgr_speed_coefficient = 12;
  bmq_turn_mgr_number_upper_limit = 600;
  bmq_turn_mgr_number_lower_limit = 0;
  
  while(1){
    setV = bmq_turn_mgr_seting_data;
    output_PWM_update = 1;
    
    main_u8x = FULLBRIGHT;
    main_u16x = nowI;//传参给下面这个函数
    display_right_3_digital();
    
    fp_bmq_turn_mgr_display = display_left_3_digital;
    if(++UI_time_out>=8){
      UI_time_out = 8;
      fp_bmq_turn_mgr_display = NULL;
      
      main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
      main_u16x = nowV;//传参给下面这个函数
      display_left_3_digital();
    }
    bmq_turn_mgr();
    switch(btn_event){
    case 0x02://短按
      
      main_u8x = 0;
      eeprom_write_unlock_addrx8();
      eeprom_buf1 = setV;
      eeprom_write();
      eeprom_write_lock();
      
      goto MAIN_UI;
    }
  }

/******************************************************************************/  
CONTINUOUS_ADJUSTMENT_MODE_I:
/******************************************************************************/

  showing_data[0] = 0x0000;
  is_output_ON = 1;
  UI_time_out = 0;
  pos = 0;
  
  flashing_FSM = 1;//闪烁状态机
  flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
  
  bmq_turn_mgr_seting_data = setI;
  bmq_turn_mgr_speed_coefficient = 12;
  bmq_turn_mgr_number_upper_limit = 500;
  bmq_turn_mgr_number_lower_limit = 0;
  
  while(1){
    setI = bmq_turn_mgr_seting_data;
    output_PWM_update = 1;
    
    main_u8x = FULLBRIGHT;
    main_u16x = nowV;//传参给下面这个函数
    display_left_3_digital();
    
    fp_bmq_turn_mgr_display = display_right_3_digital;
    if(++UI_time_out>=8){
      UI_time_out = 8;
      fp_bmq_turn_mgr_display = NULL;
      
      main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
      main_u16x = nowI;//传参给下面这个函数
      display_right_3_digital();
    }
    bmq_turn_mgr();
    switch(btn_event){
    case 0x02://短按
      
      main_u8x = 0;
      eeprom_write_unlock_addrx8();
      eep_addr  +=  4;
      eeprom_buf1 = setI;
      eeprom_write();
      eeprom_write_lock();
      
      goto MAIN_UI;
    }
  }
  
/******************************************************************************/  
SET_STORAGE://选中存取位
/******************************************************************************/

  main_u8x = FULLBRIGHT;//传参给下面这个函数
  main_u16x = setV;//传参给下面这个函数
  display_left_3_digital();
  main_u16x = setI;//传参给下面这个函数
  display_right_3_digital();
  
  //showV(setV,0);
  //showI(setI,0);
  flashing_FSM = 0;//闪烁状态机
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  UI_time_out = 0;
  while(1)
  {
    main_u16x = 10;
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    display_left_1_digital();
    
    bmq_wait_event();
    switch(btn_event){

    case 0x01://编码器正转
      goto set_V;

    case 0xFF://编码器反转
      goto SET_I;

    case 0x02://短按
      goto SETING_STORAGE_READ;
    
    }
  
    FSM_Reverse();
    
    if(UI_timeout_timer())
  {
      goto MAIN_UI;//5秒无动作
    }
  }

/******************************************************************************/  
SETING_STORAGE_READ:
/******************************************************************************/

  showing_data[0] = 
  showing_data[1] = 
  showing_data[2] = 
  showing_data[5] = 
  showing_data[6] = 0x0000;
 // t1 = 0;
  pos = 0;
  while(1){//读取提示动画
    switch(pos){//动画处理
    case 0:
      showing_data[3] = Dpy_wei_3&(~_Dpy_duan_C);
      showing_data[4] = Dpy_wei_4&(~_Dpy_duan_E);
      pos = 1;
      break;
    case 1:
      showing_data[3] = Dpy_wei_3&(~_Dpy_duan_B);
      showing_data[4] = Dpy_wei_4&(~_Dpy_duan_F);
      pos = 2;
      break;
    case 2:
      showing_data[3] = Dpy_wei_3&(~_Dpy_duan_A);
      showing_data[4] = Dpy_wei_4&(~_Dpy_duan_A);
      pos = 3;
      break;
    case 3:
      showing_data[3] = 0x0000;
      showing_data[4] = 0x0000;
      showing_data[3] = Dpy_wei_2&(~_Dpy_duan_A);
      showing_data[4] = Dpy_wei_5&(~_Dpy_duan_A);
      pos = 4;
      break;
    case 4:
      showing_data[2] = 0x0000;
      showing_data[5] = 0x0000;
      showing_data[3] = Dpy_wei_1&(~_Dpy_duan_A);
      showing_data[4] = Dpy_wei_6&(~_Dpy_duan_A);
      pos = 5;
      break;
    default:
      showing_data[1] = 0x0000;
      showing_data[6] = 0x0000;
      goto SETING_STORAGE_READ2;
    }
    btn_event = 0;
    user_timer1 = 5;
    do{
      chkbmq();
      if(btn_event== 0)continue;
      switch(btn_event){
      case 0x01://编码器正转
      case 0xFF://编码器反转
        goto SETING_STORAGE_READ2;
      }
      
    }while(user_timer1);
  }
  
/******************************************************************************/ 
SETING_STORAGE_READ2:
/******************************************************************************/

  flashing_FSM = 0;
  flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
  UI_time_out = 0;
  
  bmq_turn_mgr_seting_data = 1;
  bmq_turn_mgr_speed_coefficient = 255;
  bmq_turn_mgr_number_upper_limit = 9;
  bmq_turn_mgr_number_lower_limit = 1;
  fp_bmq_turn_mgr_display = display_left_1_digital;
  
  while(1){
    main_u8x=(u8)bmq_turn_mgr_seting_data;//传参给下面这个函数
    eeprom_read_addrx8();
    
    main_u8x = FULLBRIGHT;//传参给下面这个函数
    main_u16x = eeprom_buf1;//传参给下面这个函数
    display_left_3_digital();
    main_u16x = eeprom_buf2;//传参给下面这个函数
    display_right_3_digital();
    
    bmq_turn_mgr();
    
    switch(btn_event){
    case 0x02://短按
      
      setV = eeprom_buf1;
      setI = eeprom_buf2;
      output_PWM_update = 1;
      
      main_u8x = 0;
      eeprom_write_unlock_addrx8();
      eeprom_write();
      eeprom_buf1 = eeprom_buf2;
      eeprom_write();
      eeprom_write_lock();
      goto MAIN_UI;
    }
    
      
    if(UI_timeout_timer()){
      //5秒无动作
      goto MAIN_UI;
    }
  }
  
/******************************************************************************/  
set_V://选中电压
/******************************************************************************/

  showing_data[0] = Dpy_wei_0&Dpy_duan_negative;//横杠
  
  main_u16x = setI;//传参给下面这个函数
  main_u8x = FULLBRIGHT;//传参给下面这个函数
  display_right_3_digital();
  
  flashing_FSM = 0;//闪烁状态机
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  UI_time_out = 0;
  while(1){
    main_u16x = setV;//传参给下面这个函数
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    display_left_3_digital();
    
    bmq_wait_event();
    switch(btn_event){
    case 0x01://编码器正转
      
      goto SET_I;
    case 0xFF://编码器反转
      
      goto SET_STORAGE;
    case 0x02://短按
      
      goto SETING_V;
    case 0x03://长按
      
      goto CONTINUOUS_ADJUSTMENT_MODE_V;
    }
    
    FSM_Reverse();
    
    if(UI_timeout_timer()){
      //5秒无动作
      goto MAIN_UI;
    }
  }

/******************************************************************************/    
SETING_V://调节电压
/******************************************************************************/

  flashing_FSM = 1;//闪烁状态机
  flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
  UI_time_out = 0;
  
  bmq_turn_mgr_seting_data = setV;
  bmq_turn_mgr_speed_coefficient = 12;
  bmq_turn_mgr_number_upper_limit = 600;
  bmq_turn_mgr_number_lower_limit = 0;
  fp_bmq_turn_mgr_display = display_left_3_digital;
  
  while(1){
    bmq_turn_mgr();
    switch(btn_event){
    case 0x02://短按
      
      setV = bmq_turn_mgr_seting_data;
      output_PWM_update = 1;
      
      main_u8x = 0;
      eeprom_write_unlock_addrx8();
      eeprom_buf1 = setV;
      eeprom_write();
      eeprom_write_lock();
      
      goto MAIN_UI;
    }
    
    
    if(UI_timeout_timer()){
      //5秒无动作
      goto MAIN_UI;
    }
  }

/******************************************************************************/  
SET_I://选中电流
/******************************************************************************/

  showing_data[0] = Dpy_wei_0&Dpy_duan_negative;//横杠
  
  main_u16x = setV;//传参给下面这个函数
  main_u8x = FULLBRIGHT;//传参给下面这个函数
  display_left_3_digital();
  
  //showV(setV,0);
  flashing_FSM = 0;//闪烁状态机
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  UI_time_out = 0;
  while(1){
    main_u16x = setI;//传参给下面这个函数
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    display_right_3_digital();
    
    bmq_wait_event();
    
    switch(btn_event){
    case 0x01://编码器正转
      
      goto SET_STORAGE;
    case 0xFF://编码器反转
      
      goto set_V;
    case 0x02://短按
      
      goto SETING_I;
    case 0x03://长按
      
      goto CONTINUOUS_ADJUSTMENT_MODE_I;
    }
    
    FSM_Reverse();
      
    if(UI_timeout_timer()){
      //5秒无动作
      goto MAIN_UI;
    }
  }

/******************************************************************************/  
SETING_I://调节电流
/******************************************************************************/

  flashing_FSM = 1;//闪烁状态机
  flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
  UI_time_out = 0;
  
  bmq_turn_mgr_seting_data = setI;
  bmq_turn_mgr_speed_coefficient = 12;
  bmq_turn_mgr_number_upper_limit = 500;
  bmq_turn_mgr_number_lower_limit = 0;
  fp_bmq_turn_mgr_display = display_right_3_digital;
  while(1){
    bmq_turn_mgr();
    switch(btn_event){
    case 0x02://短按
      
      setI = bmq_turn_mgr_seting_data;
      output_PWM_update = 1;
      
      main_u8x = 0;
      eeprom_write_unlock_addrx8();
      eep_addr  +=  4;
      eeprom_buf1 = setI;
      eeprom_write();
      eeprom_write_lock();
      
      goto MAIN_UI;
    }
    
    
    if(UI_timeout_timer()){
      //5秒无动作
      goto MAIN_UI;
    }
  }
  
/******************************************************************************/  
SETING_STORAGE_WRITE:
/******************************************************************************/

  showing_data[0] = 
  showing_data[2] = 
  showing_data[3] = 
  showing_data[4] = 
  showing_data[5] = 0x0000;
  //t1 = 0;
  pos = 0;
  while(1){//存储提示动画
    switch(pos){//动画处理
    case 0:
      showing_data[1] = Dpy_wei_1&(~_Dpy_duan_A);
      showing_data[6] = Dpy_wei_6&(~_Dpy_duan_A);
      pos = 1;
      break;
    case 1:
      showing_data[1] = 0x0000;
      showing_data[6] = 0x0000;
      showing_data[2] = Dpy_wei_2&(~_Dpy_duan_A);
      showing_data[5] = Dpy_wei_5&(~_Dpy_duan_A);
      
      pos = 2;
      break;
    case 2:
      showing_data[2] = 0x0000;
      showing_data[5] = 0x0000;
      showing_data[3] = Dpy_wei_3&(~_Dpy_duan_A);
      showing_data[4] = Dpy_wei_4&(~_Dpy_duan_A);
      pos = 3;
      break;
    case 3:
      showing_data[3] = Dpy_wei_3&(~_Dpy_duan_B);
      showing_data[4] = Dpy_wei_4&(~_Dpy_duan_F);
      pos = 4;
      break;
    case 4:
      showing_data[3] = Dpy_wei_3&(~_Dpy_duan_C);
      showing_data[4] = Dpy_wei_4&(~_Dpy_duan_E);
      pos = 5;
      break;
    default:
      showing_data[3] = 0x0000;
      showing_data[4] = 0x0000;
      goto SETING_STORAGE_WRITE2;
    }
    btn_event = 0;
    user_timer1 = 5;
    do{
      chkbmq();
      if(btn_event== 0) continue;
      switch(btn_event){
      case 0x01://编码器正转
      case 0xFF://编码器反转
      goto SETING_STORAGE_WRITE2;
      }
    }while(user_timer1);
  }

/******************************************************************************/  
SETING_STORAGE_WRITE2:
/******************************************************************************/
  flashing_FSM = 0;
  flashing_style[0] = STRONGBLINK;//半亮<->全亮闪烁模式
  UI_time_out = 0;
  
  main_u8x = FULLBRIGHT;
  main_u16x = setV;
  display_left_3_digital();
  main_u16x = setI;
  display_right_3_digital();
  
  bmq_turn_mgr_seting_data = 1;
  bmq_turn_mgr_speed_coefficient = 255;
  bmq_turn_mgr_number_upper_limit = 9;
  bmq_turn_mgr_number_lower_limit = 1;
  fp_bmq_turn_mgr_display = display_left_1_digital;
  
  while(1){
    bmq_turn_mgr();
    switch(btn_event){
    case 0x02://短按
      
      main_u8x=(u8)bmq_turn_mgr_seting_data;
      eeprom_write_unlock_addrx8();
      eeprom_buf1 = setV;
      eeprom_write();
      eeprom_buf1 = setI;
      eeprom_write();
      eeprom_write_lock();
      
      goto MAIN_UI;
    }
    
    if(UI_timeout_timer()){
      //5秒无动作
      goto MAIN_UI;
    }
  }
  
/******************************************************************************/  
FACTORY_MODE://工厂模式
/******************************************************************************/

  seting_data[0] = 1;//最左边的数码管的数字
  
  main_u8x = 10;
  eeprom_read_addrx8();
  seting_data[1] = eeprom_buf1;
  seting_data[2] = eeprom_buf2;
  
  main_u8x = 11;
  eeprom_read_addrx8();
  seting_data[3] = eeprom_buf1;
  seting_data[4] = eeprom_buf2;
  
  main_u8x = 12;
  eeprom_read_addrx8();
  seting_data[5] = eeprom_buf1;
  seting_data[6] = eeprom_buf2;
  
  main_u8x = 13;
  eeprom_read_addrx8();
  seting_data[7] = eeprom_buf1;
  seting_data[8] = eeprom_buf2;
  
  main_u8x = 14;
  eeprom_read_addrx8();
  seting_data[9] = eeprom_buf1;
  seting_data[10] = eeprom_buf2;

  // trim something
  if(seting_data[3]<100 || seting_data[3]>600){
    seting_data[3] = 600;
  }
  if(seting_data[8]<100 || seting_data[8]>400){
    seting_data[8] = 100;
  }
  
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
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  
/******************************************************************************/  
FACTORY_MODE_PREV_MENU:
/******************************************************************************/
  flashing_FSM = 0;
  
/******************************************************************************/  
FACTORY_MODE_MENU:
/******************************************************************************/

  showing_data[1] = Dpy_wei_1 & Dpy_duan_null;
  showing_data[2] = Dpy_wei_2 & Dpy_duan_null;
  showing_data[3] = Dpy_wei_3 & Dpy_duan_null;
  showing_data[4] = Dpy_wei_4 & Dpy_duan_null;
  showing_data[5] = Dpy_wei_5 & Dpy_duan_null;
  showing_data[6] = Dpy_wei_6 & Dpy_duan_null;
  
  pos = 0;
  while(1){
    main_u16x = seting_data[0];//菜单数字（1~6），传参给下面这个函数
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    display_left_1_digital();
    
    pos = show_str(str_list[LOW(seting_data[0])-1],pos);//刷新显示
    
    bmq_wait_event();
    switch(btn_event){
		
    case 0x01://编码器正转
      
      if(flashing_style[0]){//正在调节最左边的数码管
        pos = 0;
        ++seting_data[0];
        if(seting_data[0] > 6) seting_data[0] = 6;
        flashing_FSM = 1;
        goto FACTORY_MODE_MENU;
      }else{
        switch(LOW(seting_data[0])){
        case 1:
          factory_mode_setV_or_setI = 0;
          goto FACTORY_MODE_SET_0_5V_OR_0_5A_REF;
        case 2:
          factory_mode_setV_or_setI = 0;
          goto FACTORY_MODE_SET_9_60V_OR_9_40A_REF;
        case 3:
          factory_mode_setV_or_setI = 5;
          goto FACTORY_MODE_SET_0_5V_OR_0_5A_REF;
        case 4:
          factory_mode_setV_or_setI = 5;
          goto FACTORY_MODE_SET_9_60V_OR_9_40A_REF;
        case 5:
          factory_mode_setV_or_setI = 5;//代表要存储参数
          goto FACTORY_MODE_SAVE_OR_CANSEL;
        default:
          factory_mode_setV_or_setI = 0;//代表要放弃参数
          goto FACTORY_MODE_SAVE_OR_CANSEL;
        }
      }
      break;
	  
    case 0xFF://编码器反转
      
      if(flashing_style[0]){//正在调节最左边的数码管
        pos = 0;
        --seting_data[0];
        if(seting_data[0] < 1) seting_data[0] = 1;
        flashing_FSM = 1;
        goto FACTORY_MODE_MENU;
      }else{
        switch(LOW(seting_data[0])){
        case 1:
          factory_mode_setV_or_setI = 0;
          goto FACTORY_MODE_SET_0_5V_OR_0_5A_REF;
        case 2:
          factory_mode_setV_or_setI = 0;
          goto FACTORY_MODE_SET_9_60V_OR_9_40A_REF2;
        case 3:
          factory_mode_setV_or_setI = 5;
          goto FACTORY_MODE_SET_0_5V_OR_0_5A_REF;
        case 4:
          factory_mode_setV_or_setI = 5;
          goto FACTORY_MODE_SET_9_60V_OR_9_40A_REF2;
        case 5:
          factory_mode_setV_or_setI = 5;//代表要存储参数
          goto FACTORY_MODE_SAVE_OR_CANSEL;
        default:
          factory_mode_setV_or_setI = 0;//代表要放弃参数
          goto FACTORY_MODE_SAVE_OR_CANSEL;
        }
      }
      break;
	  
    case 0x02://短按
            
      flashing_style[0] = !flashing_style[0];//切换：数值调整状态 <--> 项目选择状态
      
      break;
    default: //0x00,用户没有在规定时间内操作
      
      FSM_Reverse();
    }
  }

/******************************************************************************/  
FACTORY_MODE_SET_0_5V_OR_0_5A_REF:
/******************************************************************************/

  flashing_FSM = 0;//闪烁状态机
  showing_data[1] = 0x0000;
  showing_data[2] = 0x0000;
  
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  
  fp_display_PWM_value = NULL;
  
  while(1){
    main_u16x = seting_data[1+factory_mode_setV_or_setI];//传参给下面这个函数
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    display_PWM_value();
    
    bmq_wait_event();
    switch(btn_event){
		
    case 0x02://短按
      
      factory_mode_seting_PWM_PWMdata = seting_data[1+factory_mode_setV_or_setI];
      factory_mode_seting_PWM();
      if(factory_mode_setV_or_setI){//电流
        seting_data[1+5] = factory_mode_seting_PWM_PWMdata;
        seting_data[2+5] = nowI_16bit_ADC_result;
      }else{//电压
        seting_data[1] = factory_mode_seting_PWM_PWMdata;
        seting_data[2] = nowV_16bit_ADC_result;
      }
      goto FACTORY_MODE_SET_0_5V_OR_0_5A_REF;
	  
    case 0x01://编码器正转
    case 0xFF://编码器反转
      
      goto FACTORY_MODE_PREV_MENU;
	
    default:
      FSM_Reverse();
    }
  }

/******************************************************************************/  
FACTORY_MODE_SET_9_60V_OR_9_40A_REF:
/******************************************************************************/

  flashing_FSM = 0;//闪烁状态机

  if(factory_mode_setV_or_setI)
  {//电流
    showing_data[4] = Dpy_wei_4 & Dpy_duan_a;
  }
  else
  {//电压
    showing_data[4] = Dpy_wei_4 & Dpy_duan_v;
  }
  
  showing_data[5] = 0x0000;
  showing_data[6] = Dpy_wei_6 & (~_Dpy_duan_D);//显示短下划线
  
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  
  while(1){
    main_u16x = seting_data[3+factory_mode_setV_or_setI];//传参给下面这个函数
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    display_left_3_digital();
    
    bmq_wait_event();
    switch(btn_event){
    case 0x02://短按
      goto FACTORY_MODE_SET_9_60V_OR_9_40A_REF_SETING;

    case 0x01://编码器正转
      goto FACTORY_MODE_SET_9_60V_OR_9_40A_REF2;

    case 0xFF://编码器反转
      goto FACTORY_MODE_PREV_MENU;

    default:
      FSM_Reverse();
    }
  }

/******************************************************************************/  
FACTORY_MODE_SET_9_60V_OR_9_40A_REF_SETING:
/******************************************************************************/

  flashing_FSM = 1;//闪烁状态机
  
  flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
  
  bmq_turn_mgr_seting_data = seting_data[3+factory_mode_setV_or_setI];
  bmq_turn_mgr_speed_coefficient = 12;
  bmq_turn_mgr_number_upper_limit = 600;
  if(factory_mode_setV_or_setI){
    bmq_turn_mgr_number_upper_limit = 400;
  }
  bmq_turn_mgr_number_lower_limit = 90;
  fp_bmq_turn_mgr_display = display_left_3_digital;
  
  while(1){
    bmq_turn_mgr();
    
    if(btn_event== 0x02){
      seting_data[3+factory_mode_setV_or_setI] = bmq_turn_mgr_seting_data;
      
      goto FACTORY_MODE_SET_9_60V_OR_9_40A_REF;
    }
    
  }
  
/******************************************************************************/  
FACTORY_MODE_SET_9_60V_OR_9_40A_REF2:
/******************************************************************************/

  flashing_FSM = 0;//闪烁状态机
  showing_data[1] = Dpy_wei_1 & (~_Dpy_duan_D);
  showing_data[2] = 0x0000;
  
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  
  fp_display_PWM_value = NULL;
  
  while(1){
    main_u16x = seting_data[4+factory_mode_setV_or_setI];//传参给下面这个函数
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    display_PWM_value();
    
    bmq_wait_event();
    switch(btn_event){
    case 0x02://短按
      
      factory_mode_seting_PWM_PWMdata = seting_data[4+factory_mode_setV_or_setI];
      factory_mode_seting_PWM();
      if(factory_mode_setV_or_setI){//电流
        seting_data[4+5] = factory_mode_seting_PWM_PWMdata;
        seting_data[5+5] = nowI_16bit_ADC_result;
      }else{//电压
        seting_data[4] = factory_mode_seting_PWM_PWMdata;
        seting_data[5] = nowV_16bit_ADC_result;
      }
      goto FACTORY_MODE_SET_9_60V_OR_9_40A_REF2;
    case 0x01://编码器正转
      
      goto FACTORY_MODE_PREV_MENU;
    case 0xFF://编码器反转
      
      goto FACTORY_MODE_SET_9_60V_OR_9_40A_REF;
    default:
      FSM_Reverse();
    }
    
  }

/******************************************************************************/  
FACTORY_MODE_SAVE_OR_CANSEL:
/******************************************************************************/

  flashing_FSM = 0;
  
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  
  while(1)
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
    
    bmq_wait_event();//最多阻塞250ms
    
    switch(btn_event){
    case 0x02://短按
      
      if(factory_mode_setV_or_setI == 5){//存储参数
        
        main_u8x = 10;
        eeprom_write_unlock_addrx8();
        
        pos = 1;
        do{
          eeprom_buf1 = seting_data[pos];
          eeprom_write();
        }while(++pos <= 10);
        
        
        calculation_factory_parameters();
        
        eeprom_buf1 = PWM_V_bias;
        eeprom_write();
        eeprom_buf1 = PWM_V_coefficient;
        eeprom_write();
        eeprom_buf1 = PWM_I_bias;
        eeprom_write();
        eeprom_buf1 = PWM_I_coefficient;
        eeprom_write();
        eeprom_buf1 = ADC_V_bias;
        eeprom_write();
        eeprom_buf1 = ADC_V_coefficient;
        eeprom_write();
        eeprom_buf1 = ADC_I_bias;
        eeprom_write();
        eeprom_buf1 = ADC_I_coefficient;
        eeprom_write();
        
        eeprom_write_lock();
      }
      goto START_UP;
    case 0x01://编码器正转，放弃保存，cancelled
    case 0xFF://编码器反转，放弃保存，cancelled
      goto FACTORY_MODE_PREV_MENU;
    default:
      FSM_Reverse();
    }
  }
  
}

static void Init()
{
/**
  ******************************************************************************
  * 初始化TIM1（用于PWM输出，控制电压电流）
  ******************************************************************************  
  */
  {//初始化TIM1（用于PWM输出，控制电压电流）
    TIM1_PSCRH = 0;//计数器不分频
    TIM1_PSCRL = 0;//计数器不分频
    
    //初始化自动装载寄存器，决定PWM的频率，PWM频率 = 计数器频率/100
    TIM1_ARRH = 0;
    TIM1_ARRL = (100-1);
    
    {//ch1
      TIM1_CCMR1_OC1M = 7;//PWM模式2（比较输出方式）
      TIM1_CCER1_CC1P = 1;//PWM极性（1 = 正相输出，0 = 反相输出）
      TIM1_CCER1_CC1E = 1;//开启输出引脚
      
      //初始化比较寄存器，决定PWM的占空比
      TIM1_CCR1H = 0;
      TIM1_CCR1L = 0;
    }
    
    {//ch2
      TIM1_CCMR2_OC2M = 7;//PWM模式2（比较输出方式）
      TIM1_CCER1_CC2P = 1;//PWM极性（1 = 正相输出，0 = 反相输出）
      TIM1_CCER1_CC2E = 1;//开启输出引脚
      
      //初始化比较寄存器，决定PWM的占空比
      TIM1_CCR2H = 0;
      TIM1_CCR2L = 0;
    }
    
    {//ch3
      TIM1_CCMR3_OC3M = 7;//PWM模式2（比较输出方式）
      TIM1_CCER2_CC3P = 1;//PWM极性（1 = 正相输出，0 = 反相输出）
      TIM1_CCER2_CC3E = 1;//开启输出引脚
      
      //初始化比较寄存器，决定PWM的占空比
      TIM1_CCR3H = 0;
      TIM1_CCR3L = 0;
    }
    
    {//ch4
      TIM1_CCMR4_OC4M = 7;//PWM模式2（比较输出方式）
      TIM1_CCER2_CC4P = 1;//PWM极性（1 = 正相输出，0 = 反相输出）
      TIM1_CCER2_CC4E = 1;//开启输出引脚
      
      //初始化比较寄存器，决定PWM的占空比
      TIM1_CCR4H = 0;
      TIM1_CCR4L = 0;
    }
    TIM1_BKR_MOE = 1;//TIM1主输出使能
    TIM1_CR1_CEN = 1;//启动计数
  }
  
  /**
  ******************************************************************************
  * 初始化TIM2（用于PWM输出，控制风扇速度）
  ******************************************************************************  
  */
  {
    TIM2_PSCR = 0;//计数器不分频
    
    //初始化自动装载寄存器，决定PWM的频率，PWM频率 = 计数器频率/255
    TIM2_ARRH = 0xFF;
    TIM2_ARRL = 0xFF;
    
    {//ch1
      TIM2_CCMR1_OC1M = 7;//PWM模式2（比较输出方式）
      TIM2_CCER1_CC1P = 1;//PWM极性（1 = 正相输出，0 = 反相输出）
      TIM2_CCER1_CC1E = 1;//开启输出引脚
      
      //初始化比较寄存器，决定PWM的占空比
      TIM2_CCR1H = 0;
      TIM2_CCR1L = 0;
    }
    TIM2_CR1_CEN = 1;//启动计数
  }
  
/**
  ******************************************************************************
  * 初始化TIM4（用于时间基准）
  ******************************************************************************  
  */
  {
    TIM4_PSCR_PSC = 3;//8分频
    TIM4_CNTR = 0; // 计数器初值
    TIM4_ARR = 200-1;// 自动装载（每秒溢出1万次）
    TIM4_IER_UIE = 1;// 启用更新中断
    TIM4_CR1_CEN = 1; // 启动计数
    //ITC_VECT23SPR = 0;//TIM4中断优先级2/3
  }

/**
  ******************************************************************************
  * 初始化ADC
  ******************************************************************************  
  */
  {
    ADC_TDRL = HEXTOBIT(0x01111000);//ch3、ch4、ch5、ch6关闭施密特输入
    ADC_CR1_SPSEL = 4;//ADC时钟8分频
    //ADC_CR1_CONT = 1;//连续模式
    ADC_CR2_ALIGN = 1;//数据右对齐
    //ADC_CR3_DBUF = 1;//使用缓存模式
    //ADC_CSR = 0x04;//选定ch4
    ADC_CR1_ADON = 1;//打开ADC电源
    //ADC_CR1_ADON = 1;//启动ADC
  }

/**
  ******************************************************************************
  * 初始化74HC595
  ******************************************************************************  
  */  
  {
    PBinit(A,1,1,1,0);//SCLK
    PBinit(A,2,1,1,0);//CLK
    PBinit(D,4,1,1,1);//DATA
    
    PA1O = 1;
    PA2O = 1;
    PD4O = 1;
    
    HC595_data = 0;
    hc595_update();
  }
  
/**
  ******************************************************************************
  * 初始化编码器、按键
  ******************************************************************************  
  */  
  {
    PBinit(B,4,0,0,0);//编码器1
    PBinit(B,5,0,0,0);//编码器2
    PBinit(A,3,0,1,0);//按键
  }
  
/**
  ******************************************************************************
  * 初始化其他IO
  ******************************************************************************  
  */  
  {
    PBinit(C,5,1,1,0);//风扇控制
    PC5O = 0;
  }
}
