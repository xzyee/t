#include "iostm8s103f3.h"
#include "general.h"
#include "it.h"
#include "driver.h"
#include "display.h"
#include "parameter.h"
#include "main_fun.h"
#include "factory_fun.h"
#include "led.h"

#include "main.h"

u8 next_state;

u16 setV = 0;//main线程专用，最终由TIM4实行
u16 setI = 0;//main线程专用，最终由TIM4实行

u8 is_output_ON = 0;
u8 output_PWM_update = 0;//此变量置1即可更新输出PWM，自动清零

u16 nowV_16bit_ADC_result = 0;
u16 nowV = 0;//比例：537 = 53.7V

u16 nowI_16bit_ADC_result = 0;
u16 nowI = 0;//比例：125 = 12.5A

u16 set_V_limit = 1000; //电压设置限制
u16 set_I_limit = 500;  //电流设置限制

s16 seting_data[11];

u8 factory_mode_setV_or_setI;//工厂模式专用变量，0 = 设置电压（对应菜单项1和2），5 = 设置电流（对应菜单项3和4）。FLASH空间不足了，逼我做成代码复用....

u8 flashing_style[2] = {FULLBRIGHT,FULLBRIGHT}; //0=灭<->亮循环，1=半亮<->全亮循环,2=常亮

u8 main_u8x;//此变量为临时多用变量，只能在main线程内使用，不允许跨函数使用
u16 main_u16x;//此变量为临时多用变量，只能在main线程内使用，不允许跨函数使用

u8 pos = 0;

static void Init();


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

	//熄灭全部数码管
	clear_showing_data();

	next_state = WAITING_TO_FACTORY_MODE;
	


while(1)
{  
	//while(1){do_read_animation();}
	switch(next_state)
	{
		
		/******************************************************************************/
		case WAITING_TO_FACTORY_MODE:
		/******************************************************************************/
		main_u8x = 0; 
		btn_event = 0;
		main_u16x = 12;//WAIT_TIME_FOR_GOING_TO_FACTORY_MENU_COUNTS;
		next_state = START_UP;
		
		while(main_u16x)
		{
			bmq_wait_event();
			if(btn_event == TURN_BUTTONDOWN_SHORT)
				++main_u8x; 
			if(main_u8x >= 3)
			{//按钮连续短按三次
				btn_event = TURN_NONE;
				pos = show_str(str_0, pos);
				next_state = FACTORY_MODE;
				break;
			}
			--main_u16x;
		}
		break;
			
		/******************************************************************************/
		case START_UP:
		/******************************************************************************/
		{//读取EEPROM工厂模式的参数
			main_u8x = EEPROM_ADDR_START_CAL_DATA;
			eeprom_read_addrx8();
			PWM_V_bias = eeprom_buf1;
			PWM_V_coefficient = eeprom_buf2;

			main_u8x = EEPROM_ADDR_START_CAL_DATA + 1;
			eeprom_read_addrx8();
			PWM_I_bias = eeprom_buf1;
			PWM_I_coefficient = eeprom_buf2;

			main_u8x = EEPROM_ADDR_START_CAL_DATA + 2;
			eeprom_read_addrx8();
			ADC_V_bias = eeprom_buf1;
			ADC_V_coefficient = eeprom_buf2;

			main_u8x = EEPROM_ADDR_START_CAL_DATA + 3;
			eeprom_read_addrx8();
			ADC_I_bias = eeprom_buf1;
			ADC_I_coefficient = eeprom_buf2;
		}

		{//读取EEPROM默认电压电流
			main_u8x = EEPROM_ADDR_START_DEFAULT;
			eeprom_read_addrx8();
			setV = eeprom_buf1;
			setI = eeprom_buf2;
		}

		{//读取EEPROM限制电压电流
			main_u8x = EEPROM_ADDR_START_LIMIT;
			eeprom_read_addrx8();
			set_V_limit = eeprom_buf1;
			//if(set_V_limit<500)  set_V_limit = 500;
			set_I_limit = eeprom_buf2;
			//if(set_I_limit<400)  set_I_limit = 400;
		}

			is_output_ON = 0; //开机要求不输出两路基准
			output_PWM_update = 0; 

			flashing_FSM = 1;
			UI_time_out = 0;
			showing_data[0] = DARK;

			main_u8x = FULLBRIGHT;//传参给下面这个函数
			main_u16x = setV;//传参给下面这个函数
			display_left_4_digital();

			main_u16x = setI;//传参给下面这个函数
			display_right_3_digital();
			next_state = MAIN_UI;
			break;

		/******************************************************************************/  
		case MAIN_UI:
		/******************************************************************************/

			flashing_FSM = 0;//闪烁状态机
			flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
			UI_time_out = 0;
			pos = 0;
			
			//默认主界面
			showing_data[0] = DARK;

			main_u8x = FULLBRIGHT;//传参给下面这个函数
			main_u16x = is_output_ON ? nowV : setV;//传参给下面这个函数
			display_left_4_digital();

			if(is_output_ON)
			{
			flashing_FSM = 1;

			main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
			main_u16x = nowI;//传参给下面这个函数
			display_right_3_digital();

			}
			else
			{
				showing_data[4] = Dpy_wei_4&Dpy_duan_o;
				showing_data[5] = Dpy_wei_5&Dpy_duan_f;
				showing_data[6] = Dpy_wei_6&Dpy_duan_f;
			}

			btn_event = TURN_NONE;
			user_timer1 = 6;
			do
			{
				chkbmq();
				if(btn_event == TURN_NONE) continue;
				switch(btn_event)
				{
					case TURN_LEFT://编码器反转
						next_state = SET_V; break;
					case TURN_RIGHT://编码器正转
						next_state = SET_I; break;
					case TURN_BUTTONDOWN_SHORT://按钮短按 = 开启输出/关闭输出
						btn_event = TURN_NONE;
						
						user_timer1 = WAIT_TIME_FOR_SHORTSHORT_PUSH_COUNTS; //判断连续按两下
						while(user_timer1)
						{				
							if(btn_event == TURN_BUTTONDOWN_SHORT) //again
							{
								next_state = SETING_STORAGE_READ;
								break;
							}
						}
						if(next_state != SETING_STORAGE_READ)
						{
							is_output_ON = is_output_ON ? 0 : 1;//切换：打开输出/关闭输出
							output_PWM_update = 1;
							next_state = MAIN_UI; 
						}
						break;
					case TURN_BUTTONDOWN_LONG://按钮长按
						user_timer1 = WAIT_TIME_FOR_LONG_PUSH_COUNTS;
						while(user_timer1);
						if(btn_down_time > WAIT_TIME_FOR_LONG_PUSH_COUNTS) 
						{
							next_state = MAIN_UI; btn_event = TURN_NONE; //不理会本次长按
						}
						else
						{
							next_state = SETING_STORAGE_WRITE; break;
						}
                                                break;
					case TURN_BUTTONDOWN_LONGLONG://按钮长按3秒
						user_timer1 = 64; /*1.6s*/
						main_u8x = user_timer1; main_u16x = PWM >> 6;
						while(1)
						{
							if(user_timer1 == main_u8x) continue;//25ms坎未到
							main_u8x = user_timer1;
							PWM = ( PWM > main_u16x ) ? PWM - main_u16x : 0; //逐渐减小至0
							set_V_PWM();
							if(PWM == 0) is_output_ON = 0; clear_by_null();
						};//永远循环，只能关机
						break;
					default:
						break;
				}
			}while(user_timer1);

		
			break;
		/******************************************************************************/  
		case CONTINUOUS_ADJUSTMENT_MODE_I:
		/******************************************************************************/
		do_continuous_adjustment_i();
		next_state = MAIN_UI; break;

		/******************************************************************************/  
		case CONTINUOUS_ADJUSTMENT_MODE_V:
		/******************************************************************************/
		do_continuous_adjustment_v();
		next_state = MAIN_UI; break;

		/******************************************************************************/  
		case SET_STORAGE://选中存取位
		/******************************************************************************/
		menu_set_storage(); break;
		
		/******************************************************************************/  
		case SETING_STORAGE_READ:
		/******************************************************************************/
		do_read_animation(); 
		next_state = SETING_STORAGE_READ2; break;
		
		/******************************************************************************/ 
		case SETING_STORAGE_READ2:
		/******************************************************************************/
		do_seting_storage_read2();
		next_state = MAIN_UI; break;

		/******************************************************************************/  
		case SET_V://选中电压
		/******************************************************************************/
		menu_set_V(); break;
		/******************************************************************************/    
		case SETING_V://调节电压
		/******************************************************************************/
		do_seting_V(); 
		next_state = MAIN_UI; break;
		/******************************************************************************/  
		case SET_I://选中电流
		/******************************************************************************/
		menu_set_I(); break;
		/******************************************************************************/  
		case SETING_I://调节电流
		/******************************************************************************/
		do_seting_I(); 
		next_state = MAIN_UI; break;

		/******************************************************************************/  
		case SETING_STORAGE_WRITE:
		/******************************************************************************/
		do_write_animation();
		next_state = SETING_STORAGE_WRITE2; break;
		/******************************************************************************/  
		case SETING_STORAGE_WRITE2:
		/******************************************************************************/
		do_seting_storage_write2();
		next_state = MAIN_UI; break;



		/******************************************************************************/  
		case FACTORY_MODE://工厂模式
		/******************************************************************************/

		seting_data[0] = 1;//最左边的数码管的数字

		main_u8x = EEPROM_ADDR_START_CAL_RAW;
		eeprom_read_addrx8();
		seting_data[1] = eeprom_buf1;
		seting_data[2] = eeprom_buf2;

		main_u8x = EEPROM_ADDR_START_CAL_RAW + 1;
		eeprom_read_addrx8();
		seting_data[3] = eeprom_buf1; //Vref
		seting_data[4] = eeprom_buf2;

		main_u8x = EEPROM_ADDR_START_CAL_RAW + 2;
		eeprom_read_addrx8();
		seting_data[5] = eeprom_buf1;
		seting_data[6] = eeprom_buf2;

		main_u8x = EEPROM_ADDR_START_CAL_RAW + 3;
		eeprom_read_addrx8();
		seting_data[7] = eeprom_buf1;
		seting_data[8] = eeprom_buf2;//Iref

		main_u8x = EEPROM_ADDR_START_CAL_RAW + 4;
		eeprom_read_addrx8();
		seting_data[9]  = eeprom_buf1;
		seting_data[10] = eeprom_buf2;

		// trim something ???
		if(seting_data[3]<100 || seting_data[3]>12000){
		seting_data[3] = 12000;
		}
		if(seting_data[8]<100 || seting_data[8]>900){
		seting_data[8] = 900;
		}

		flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
		next_state = FACTORY_MODE_PREV_MENU;
		break;

		/******************************************************************************/  
		case FACTORY_MODE_PREV_MENU:
		/******************************************************************************/
		flashing_FSM = 0;
		next_state = FACTORY_MODE_MENU;

		/******************************************************************************/  
		case FACTORY_MODE_MENU:
		/******************************************************************************/
			show_factory_mode_menu();
			break;

		case FACTORY_MODE_SET_0_5V_OR_0_5A_REF:
			do_set_0p5V_or_0p5A_ref();
			break;

		case STATE_2_STEP0:
		do_factory_frame_menu_V();//显示方框菜单
		break;
		case STATE_2_STEP1: //设置极限电压
		do_factory_set_V_limit();
		next_state = STATE_2_STEP0;
		break;
		case STATE_2_STEP2: //设置校准电压
		//do_read_animation(); do_read_animation();
		do_factory_set_V_ref();
		next_state = STATE_2_STEP3;
		break;
		case STATE_2_STEP3: //校准此校准电压
		do_factory_cal_V_ref();
		next_state = FACTORY_MODE_MENU;
		break;
		case STATE_4_STEP0: //设置极限电流
		do_factory_frame_menu_V();//显示方框菜单
		break;
		case STATE_4_STEP1: //设置极限电流
		do_factory_set_I_limit();
		next_state = STATE_4_STEP0;
		break;
		case STATE_4_STEP2: //设置校准电流
		do_factory_set_I_ref();
		next_state = STATE_4_STEP3;
		break;
		case STATE_4_STEP3: //校准此校准电流
		do_factory_cal_I_ref();
		next_state = FACTORY_MODE_MENU;
		break;

		case FACTORY_MODE_SAVE_OR_CANSEL:
		do_factory_mode_save();
		break;
		
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
	PBinit(D,2,1,1,0);//X控制
	PD2O = 1;//开机要求输出高电平
  }
}
