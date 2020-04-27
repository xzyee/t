#include "iostm8s103f3.h"
#include "general.h"
#include "driver.h"
#include "it.h"
#include "display.h"
#include "main.h"
#include "parameter.h"
#include "led.h"

/******************************************************************************/  
//FACTORY_MODE_MENU:
/******************************************************************************/

static void factory_mode_menu_entry()
{
	clear_by_null();
	pos = 0;
	flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
}

static void factory_mode_menu_after()
{
	main_u16x = seting_data[0];//菜单数字（1~6），传参给下面这个函数
	main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
	display_left_1_digital();

	pos = show_str(str_list[LOW(seting_data[0])-1],pos);//刷新显示
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
		factory_mode_setV_or_setI = 0;
		next_state = STATE_2_STEP0; break;
	case 3:
		factory_mode_setV_or_setI = 5;
		next_state = FACTORY_MODE_SET_0_5V_OR_0_5A_REF; break;
	case 4:
		factory_mode_setV_or_setI = 5;
		next_state = STATE_4_STEP0; break;
	case 5:
		factory_mode_setV_or_setI = 5;//代表要存储参数
		next_state = FACTORY_MODE_SAVE_OR_CANSEL; break;
	default:
		factory_mode_setV_or_setI = 0;//代表要放弃参数
		next_state = START_UP; break;
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
  flashing_FSM = 0;//闪烁状态机
  showing_data[1] = 0x0000;
  showing_data[2] = 0x0000;
  
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  
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
	factory_mode_seting_PWM_data = seting_data[1 + factory_mode_setV_or_setI];
	factory_mode_seting_PWM();
	if(factory_mode_setV_or_setI)
	{//电流
		seting_data[1+5] = factory_mode_seting_PWM_data;
		seting_data[2+5] = 128;//nowI_16bit_ADC_result;
	}else
	{//电压
		seting_data[1] = factory_mode_seting_PWM_data;
		seting_data[2] = 128;//nowV_16bit_ADC_result;
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


enum{
	Vlim = 2,
	Vcal = 3
};

enum{
	Ilim = 2,
	Ical = 3
};

void display_two_zero(u8 c)
{
	showing_data[2] = Dpy_wei_2 & Dpy_duan_0;//显示方框
	showing_data[3] = Dpy_wei_3 & Dpy_duan_0;//显示方框
	
	if(c == 3)
	{
		set_brightness(STRONGBLINK, _Dpy_wei_2 );
	}
	else if(c == 2)
	{
		set_brightness(STRONGBLINK, _Dpy_wei_3 );
	}
	else
	{
		set_brightness(FULLBRIGHT, _Dpy_wei_2 | _Dpy_wei_3 );
	}
}

#define BLUR_LEFT_FRAME display_two_zero(Vlim);
#define BLUR_RIGHT_FRAME display_two_zero(Vcal);
#define SHINE_ALL_FRAME display_two_zero(0);
static void wwwww()
{
	SHINE_ALL_FRAME
	user_timer1 = 4;
	while(user_timer1);
}


static void frame_menu_V_entry()
{

	clear_by_null();

	setV = 0;
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
		next_state = STATE_2_STEP1; //设置电压极限
	else if(main_u8x == Vcal && set_V_limit != 0)
	{
		next_state = STATE_2_STEP2; //设置校准电压
	}
	else 
	{
do_read_animation(); do_read_animation();do_read_animation(); do_read_animation();
	}
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
	flashing_FSM = 1;//闪烁状态机
	flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
	
	main_u8x = EEPROM_ADDR_START_LIMIT;
	eeprom_read_addrx8();
	set_V_limit = eeprom_buf1;
	set_I_limit = eeprom_buf2;
	
	output_PWM_update = 0;
}

static void callback_set_V_limit_after()
{
      set_V_limit = bmq_turn_mgr_seting_data;
      output_PWM_update = 1;
}
	
void do_factory_set_V_limit()
{
	bqm_fucn(
		DATA_SET_V_LIMIT,
		3,//COEFFICIENT_SETING_V_LIMIT,
		MAX_V_LIMIT,
		MIN_V_LIMIT,
		display_left_4_digital,
		callback_set_V_limit_entry,
		NULL,
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
	flashing_FSM = 1;//闪烁状态机
	flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
	setV = seting_data[3];
}

static void callback_set_V_ref_after()
{
	//setV = bmq_turn_mgr_seting_data;
	seting_data[3+factory_mode_setV_or_setI] = setV;//bmq_turn_mgr_seting_data;
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
		NULL,
		NULL,/*while*/
		callback_set_V_ref_after,
		TIMEOUT_COUNTS //超时5s
	);
}
/******************************************************************************/  
//STATE_2_STEP3
/******************************************************************************/  
static void aaaa()
{
	flashing_FSM = 0;//闪烁状态机
	flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
	fp_display_PWM_action = NULL;
	showing_data[2] = 0x0000;
}
static void do_factory_cal_V_ref_entry()
{
	aaaa();
	showing_data[1] = Dpy_wei_1 & Dpy_duan_v;
}

static void do_factory_cal_V_ref_after()
{
	main_u16x = seting_data[1+factory_mode_setV_or_setI];//传参给下面这个函数
	main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
	display_PWM_value();
}

static void do_factory_cal_V_ref_callback_btndown()
{
	factory_mode_seting_PWM_data = seting_data[1 + factory_mode_setV_or_setI];
	factory_mode_seting_PWM();
	if(factory_mode_setV_or_setI)
	{//电流
	seting_data[1 + 5] = factory_mode_seting_PWM_data;
	seting_data[2 + 5] = 9999;//nowI_16bit_ADC_result;
	}
	else
	{//电压
	seting_data[1] = factory_mode_seting_PWM_data;
	seting_data[2] = 9999;//nowV_16bit_ADC_result;
	}
	
	next_state = STATE_4_STEP0;
}

void do_factory_cal_V_ref()
{
	factory_fucn(
		do_factory_cal_V_ref_entry,
		do_factory_cal_V_ref_after,
		NULL,
		NULL,
		do_factory_cal_V_ref_callback_btndown
		);
}

/******************************************************************************/  
//STATE_4_STEP0
/******************************************************************************/  



static void frame_menu_I_entry()
{

	clear_by_null();

	setI = 0;
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
	{
		next_state = STATE_4_STEP2; //设置校准电流
	}
	else 
	{
do_read_animation(); do_read_animation();do_read_animation(); do_read_animation();
	}
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
	flashing_FSM = 1;//闪烁状态机
	flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
	
	main_u8x = EEPROM_ADDR_START_LIMIT;
	eeprom_read_addrx8();
	set_I_limit = eeprom_buf2;
	
	output_PWM_update = 0;
}

static void callback_set_I_limit_after()
{
      set_I_limit = bmq_turn_mgr_seting_data;
      output_PWM_update = 1;
}
	
void do_factory_set_I_limit()
{
	bqm_fucn(
		DATA_SET_I_LIMIT,
		3,//COEFFICIENT_SETING_I_LIMIT,
		MAX_I_LIMIT,
		MIN_I_LIMIT,
		display_left_4_digital,
		callback_set_I_limit_entry,
		NULL,
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
	flashing_FSM = 1;//闪烁状态机
	flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
	setI = seting_data[8];
}

static void callback_set_I_ref_after()
{
	//setI = bmq_turn_mgr_seting_data;
	seting_data[3+factory_mode_setV_or_setI] = setI;//bmq_turn_mgr_seting_data;
	output_PWM_update = 1;
}
	
void do_factory_set_I_ref()
{
	bqm_fucn(
		DATA_SET_I_REF,
		3,//COEFFICIENT_SETING_I_REF,
		set_I_limit,
		MIN_I_LIMIT,
		display_left_4_digital,
		callback_set_I_ref_entry,
		NULL,
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
	aaaa();
	showing_data[1] = Dpy_wei_1 & Dpy_duan_v;
}

static void do_factory_cal_I_ref_after()
{
	main_u16x = seting_data[1+factory_mode_setV_or_setI];//传参给下面这个函数
	main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
	display_PWM_value();
}

static void do_factory_cal_I_ref_callback_btndown()
{
	factory_mode_seting_PWM_data = seting_data[1 + factory_mode_setV_or_setI];
	factory_mode_seting_PWM();
	if(factory_mode_setV_or_setI)
	{//电流
	seting_data[4 + 5] = factory_mode_seting_PWM_data;
	seting_data[5 + 5] = 9999;//nowI_16bit_ADC_result;
	}
	else
	{//电压
	seting_data[4] = factory_mode_seting_PWM_data;
	seting_data[5] = 9999;//nowV_16bit_ADC_result;
	}
	
	next_state = FACTORY_MODE_PREV_MENU;
}

void do_factory_cal_I_ref()
{
	factory_fucn(
		do_factory_cal_I_ref_entry,
		do_factory_cal_I_ref_after,
		NULL,
		NULL,
		do_factory_cal_I_ref_callback_btndown
		);
}


/******************************************************************************/  
//factory_mode_save_or_cansel:
/******************************************************************************/

static void factory_mode_save_entry()
{
	  flashing_FSM = 0;
  
  flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
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
	if(factory_mode_setV_or_setI == 5)
	{//存储参数
do_read_animation();
		main_u8x = EEPROM_ADDR_START_LIMIT;
		eeprom_write_unlock_addrx8();
		
		eeprom_buf1 = set_V_limit;
		eeprom_write();
		eeprom_buf1 = set_I_limit;
		eeprom_write();

		for(pos = 1; pos < 11; ++pos)
		{
			eeprom_buf1 = seting_data[pos];
			eeprom_write();
		}

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




