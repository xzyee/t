#include "iostm8s103f3.h"
#include "general.h"
#include "driver.h"
#include "main.h"
#include "parameter.h"

static u8 btn_status = 1;
u8 btn_down_time = 0;

u16 _setV = 0;//TIM4线程专用
u16 _setI = 0;//TIM4线程专用

static u16 output_I_limit; 
static u16 _adc_buf[4];

static u16 adc_buf = 0;
static u8  adc_now_ch = 0;//0是主温度，1是电压，2是电流，3是整流温度
static u8  adc_step = 0;
static u8  adc_count;

static u8 tim4_timer1 = 250;
static u8 tim4_timer2 = 0;//此处放入非0值即可每25mS自动递减一次（TIM4专用）
static u8 tim4_timer3 = 0;//此处放入非0值即可每25mS自动递减一次（TIM4专用）

static u8 smg_scan = 0;
static u8 smg_scan2 = 0;

u8 user_timer1 = 0;//此处放入非0值即可每25mS自动递减一次


#pragma optimize = low
void setV_update()
{
  PWM = 0;
  if(is_output_ON && _setV){
    PWM = (u32)_setV * PWM_V_coefficient / 600;
    PWM += PWM_V_bias;
    if( (s16)PWM < 0 )
    {
      PWM = 0;
    }
  }
  set_V_PWM();
}


#pragma optimize = low
void setI_update()
{
  PWM = 0;
  if(_setI){
    PWM = (u32)_setI * PWM_I_coefficient / 600;
    PWM += PWM_I_bias;
    if( (s16)PWM < 0 )
    {
      PWM = 0;
    }
  }
  set_I_PWM();
}

#pragma vector = TIM4_OVR_UIF_vector
__interrupt void TIM4_Init()
{
   //数码管管理
  {
    hc595_update();
    
    smg_scan2 = !smg_scan2;
    HC595_data = showing_data[smg_scan] & HC595_data_mask[smg_scan2];
    
    smg_scan++;
    if(smg_scan >= 7) {smg_scan = 0;}
  }
  
  //编码器管理
  {
    if(bmq_idel_time != 0xFF) bmq_idel_time++;
    if(bmq_idel_time == 6){
      //完成了一次转动
      bmq_turning = 0;
      btn_event = bmq_last_up - bmq_frist_down;
      bmq_last2_time = bmq_last_time;
      bmq_last_time = 0;
    }
    if((HIGH(bmq_last_time)&0x80)==0) bmq_last_time++;
  }
  
  //按键去抖动
  {
	if(PA3I)
	{//没按
		if(btn_status)
		{
			if(btn_status < TIME_DITHER) btn_status = 1;
			btn_status--;
		}
	}
	else
	{//正在按
		if(btn_status != 0xFF)
		{
			if(btn_status >= TIME_DITHER) btn_status = 0xFE;
			btn_status++;
		}
	}
  }
  
  //通用定时器
  {
	if(--tim4_timer1 == 0)
	{
		//每25mS进入一次
		if(user_timer1) user_timer1--;
		if(tim4_timer2) tim4_timer2--;
		if(tim4_timer3) tim4_timer3--;
		tim4_timer1 = 250;

		{//按键判断
			if(btn_status >= TIME_DITHER)
			{//正在按
				if(btn_down_time != 0xFF) btn_down_time++;
				if(btn_down_time == TIME_BUTTONDOWN_LONG)
				{//发现是长按
					btn_event = TURN_BUTTONDOWN_LONG;
				}
				if(btn_down_time == TIME_BUTTONDOWN_LONGLONG)
				{//发现是长长按
					btn_event = TURN_BUTTONDOWN_LONGLONG;
				}
			}
			else
			{//没按
				if(btn_down_time)
				{
					if(btn_down_time < TIME_BUTTONDOWN_LONG)
					{//是短按
						btn_event = TURN_BUTTONDOWN_SHORT;
					}
					btn_down_time = 0;
				}
			}
		}
	}
  }
  
  //ADC
  {
    switch(adc_step){
    
    case 0://要切换通道
  
      switch(adc_now_ch)
    {
    
      case 0:
        //之前是主温度ADC
        //adc_buf数值 17316 = 4.1V，2111 = 0.5V
        if(adc_buf >= 17316 || adc_buf < 2111)
        {
          is_fan_need_to_speed_up |= 1;
        }
        adc_now_ch = 1;//切换到电压ADC
        ADC_CSR = 0x06;//选定ch6
        break;
    
      case 1:
        //之前是电压ADC
        nowV_16bit_ADC_result = adc_buf;
        if(adc_buf >= ADC_V_bias){
          nowV = (((u32)(adc_buf - ADC_V_bias)) * 600) / ADC_V_coefficient;
        }else{
          nowV =  1 + ~((((u32)(ADC_V_bias - adc_buf)) * 600) / ADC_V_coefficient);
        }
        adc_now_ch = 2;//切换到电流ADC
        ADC_CSR = 0x05;//选定ch5
        break;
    
      case 2:
        //之前是电流ADC
        nowI_16bit_ADC_result = adc_buf;
        if(adc_buf >= ADC_I_bias){
          nowI = (((u32)adc_buf - ADC_I_bias) * 600) / ADC_I_coefficient;
        }else{
          nowI = 1 + ~(((ADC_I_bias - (u32)adc_buf) * 600) / ADC_I_coefficient); 
        }
        adc_now_ch = 3;//切换到整流温度ADC
        ADC_CSR = 0x03;//选定ch3
        break;
    
      default:
        //之前是整流温度ADC
        if(adc_buf > 63000){//整流测温二极管开路
          if(nowI > 50 && nowI > nowV && nowV < 400){
            is_fan_need_to_speed_up |= 1;
            
            if(tim4_timer2 == 0)
            {
              output_I_limit = 400;//限流40A
              tim4_timer3 = 200;
            }
          }
          else
          {
            if(tim4_timer3 == 0)
            {
              output_I_limit = set_I_limit;//限流50A
              tim4_timer2 = 200;
            }
          }
        }
    else
    { //整流测温二极管正常

        }//end else
        
        if(is_fan_need_to_speed_up)
        {
          if(TIM2_CCR1H != 0xFF) TIM2_CCR1H++;
          TIM2_CCR1L = 0xFF;
        }
        else
        {
          if(TIM2_CCR1H) TIM2_CCR1H--;
          TIM2_CCR1L = 0;
        }
    
        is_fan_need_to_speed_up = 0;
                
        adc_now_ch = 0;//切换到主温度ADC
        ADC_CSR = 0x04;//选定ch4
        break;
    
      }//switch(adc_now_ch)
      adc_step = 1;
      break;
    
    
    case 1://预启动ADC
  
  
      ADC_CR1_ADON = 1;//启动ADC
      _adc_buf[0] =0;
      _adc_buf[1] =0;
      _adc_buf[2] =0;
      _adc_buf[3] =0;
      adc_count = 64;
      adc_step = 2;
      
      _setV = setV;
      
      if(output_I_limit < setI) //由于温度原因限流了
      {
        if(_setI != output_I_limit)
        {
          _setI = output_I_limit; //降低电流
          output_PWM_update = 1;
        }
      }
      else
      {
        if(_setI != setI)
        {
          _setI = setI; //正常传递
          output_PWM_update = 1;
        }
      }
      
      if(output_PWM_update)
      {//把PWM控制代码放在这里
        setV_update();
        setI_update();
        output_PWM_update = 0;
      }
      
      break;
       
    default:
  
  //继续启动ADC
      _adc_buf[adc_step-2] += ADC_DR;
      //adc_buf += ADC_DR;
      ADC_CR1_ADON = 1;//启动ADC
      adc_count--;
      if(adc_count == 0){
        adc_count = 64;
        adc_step++;
        if(adc_step >= 6){
          adc_buf=((u32)_adc_buf[0]+(u32)_adc_buf[1]+(u32)_adc_buf[2]+(u32)_adc_buf[3])/4;
          adc_step = 0;
        }
      }
      break;
    } //case end
  } // ADC end
  
  
  TIM4_SR_UIF = 0;//取消本次中断请求
}
