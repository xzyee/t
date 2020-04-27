#include "iostm8s103f3.h"
#include "general.h"
#include "driver.h"
#include "it.h"
#include "display.h"
#include "main.h"
#include "parameter.h"

/******************************************************************************/
//CONTINUOUS_ADJUSTMENT_MODE_I:
/******************************************************************************/

static void comm_save_vi(u8 e)
{
	main_u8x = EEPROM_ADDR_START_DEFAULT;
	eeprom_write_unlock_addrx8();
	
	eep_addr  +=  (u8)e;
	if(e)
	{  //current
		eeprom_buf1 = setI;
	}
	else
	{  //volt
		eeprom_buf1 = setV;
	}
	eeprom_write();
	eeprom_write_lock();
}

static void continuous_adjustment_vi_entry()
{
  showing_data[0] = 0x0000;
  is_output_ON = 1;
  pos = 0;
  
  flashing_FSM = 1;//闪烁状态机
  flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
}

static void continuous_adjustment_i_before()
{
	setI = bmq_turn_mgr_seting_data;
	output_PWM_update = 1;

	main_u8x = FULLBRIGHT;
	main_u16x = nowV;//传参给下面这个函数
	display_left_4_digital();

	fp_bmq_turn_mgr_display = display_right_3_digital;
	if(++UI_time_out >= 8)
	{
		UI_time_out = 8;
		fp_bmq_turn_mgr_display = NULL;

		main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
		main_u16x = nowI;//传参给下面这个函数
		display_right_3_digital();
	}
}

static void continuous_adjustment_i_after()
{
	comm_save_vi(CURRENT);
}

void do_continuous_adjustment_i()
{
	bqm_fucn(
		DATA_SET_I,
		COEFFICIENT_ADJ_I,//coefficient
		set_I_limit,
		0,
		NULL,
		continuous_adjustment_vi_entry,
		continuous_adjustment_i_before,
		NULL,/*while*/
		continuous_adjustment_i_after,
		0
	);
}

/******************************************************************************/
//CONTINUOUS_ADJUSTMENT_MODE_V: 
/******************************************************************************/


static void continuous_adjustment_v_before()
{
	setV = bmq_turn_mgr_seting_data;
	output_PWM_update = 1;

	main_u8x = FULLBRIGHT;
	main_u16x = nowI;//传参给下面这个函数
	display_right_3_digital();

	fp_bmq_turn_mgr_display = display_left_4_digital;
	if(++UI_time_out >= 8)
	{
		UI_time_out = 8;
		fp_bmq_turn_mgr_display = NULL;

		main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
		main_u16x = nowV;//传参给下面这个函数
		display_left_4_digital();
	}
}

static void continuous_adjustment_v_after()
{
	comm_save_vi(VOLT);
}

void do_continuous_adjustment_v()
{
	bqm_fucn(
		DATA_SET_V,
		COEFFICIENT_ADJ_V,//coefficient
		set_V_limit, 
		0,
		NULL,
		continuous_adjustment_vi_entry,
		continuous_adjustment_v_before,
		NULL,/*while*/
		continuous_adjustment_v_after,
		0
	);
}

/******************************************************************************/  
//SET_STORAGE:
/******************************************************************************/
static void set_storage_callback_entry()
{
	main_u8x = FULLBRIGHT;//传参给下面这个函数
	main_u16x = setV;//传参给下面这个函数
	display_left_4_digital();
	main_u16x = setI;//传参给下面这个函数
	display_right_3_digital();

	flashing_FSM = 0;//闪烁状态机
	flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
  
}

static void set_storage_callback_while()
{
	main_u16x = 10;
	main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
	display_left_1_digital();
}

void menu_set_storage()
{
	main_menu(
		SET_I,
		SET_V,
		SETING_STORAGE_READ,
		STATE_NONE,
		STATE_NONE,
		set_storage_callback_entry,
		set_storage_callback_while,
		TIMEOUT_COUNTS //超时5s
	);
}

/******************************************************************************/  
//SETING_STORAGE_READ:
/******************************************************************************/  
void do_read_animation()
{
	clear_showing_data_but34();
	u8 cnt = 0;
	while(1)
	{//读取提示动画
		switch(cnt){//动画处理
		case 0:
			showing_data[3] = Dpy_wei_3&(~_Dpy_duan_C);
			showing_data[4] = Dpy_wei_4&(~_Dpy_duan_E);
			cnt = 1;
		break;
		case 1:
			showing_data[3] = Dpy_wei_3&(~_Dpy_duan_B);
			showing_data[4] = Dpy_wei_4&(~_Dpy_duan_F);
			cnt = 2;
		break;
		case 2:
			showing_data[3] = Dpy_wei_3&(~_Dpy_duan_A);
			showing_data[4] = Dpy_wei_4&(~_Dpy_duan_A);
			cnt = 3;
		break;
		case 3:
			showing_data[3] = 0x0000;
			showing_data[4] = 0x0000;
			showing_data[3] = Dpy_wei_2&(~_Dpy_duan_A);
			showing_data[4] = Dpy_wei_5&(~_Dpy_duan_A);
			cnt = 4;
		break;
		case 4:
			showing_data[2] = 0x0000;
			showing_data[5] = 0x0000;
			showing_data[3] = Dpy_wei_1&(~_Dpy_duan_A);
			showing_data[4] = Dpy_wei_6&(~_Dpy_duan_A);
			cnt = 5;
		break;
		default:
			showing_data[1] = 0x0000;
			showing_data[6] = 0x0000;
			next_state = SETING_STORAGE_READ2;
			return;
		}
		btn_event = 0;
		user_timer1 = 5;
		do{
			chkbmq();
			if(btn_event == TURN_NONE )continue;
			switch(btn_event)
			{
				case TURN_RIGHT://编码器正转
				case TURN_LEFT://编码器反转
				//next_state = SETING_STORAGE_READ2; 
				return;
			}
		}while(user_timer1);

	} 
}

/******************************************************************************/
//SETING_STORAGE_READ2 超时返回
/******************************************************************************/
 
static void seting_storage_read2_entry()
{
	flashing_FSM = 0;
	flashing_style[0] = STRONGBLINK;//半亮<->全亮闪烁模式
}

static void ssss()
{
	eeprom_read_addrx8();

	main_u8x = FULLBRIGHT;//传参给下面这个函数
	main_u16x = eeprom_buf1;//传参给下面这个函数
	display_left_4_digital();

	main_u16x = eeprom_buf2;//传参给下面这个函数
	display_right_3_digital();
}



static void seting_storage_read2_before()
{
	main_u8x = EEPROM_ADDR_START_USER;
	ssss();
}

static void seting_storage_read2_while()
{
	main_u8x = EEPROM_ADDR_START_USER + bmq_turn_mgr_seting_data - 1;
	ssss();
}

static void seting_storage_read2_after()
{
	setV = eeprom_buf1;
	setI = eeprom_buf2;
}

void do_seting_storage_read2()
{
	bqm_fucn(
		DATA_MENU,
		COEFFICIENT_READ,//coefficient
		MAX_USER_ITEM,
		MIN_USER_ITEM,
		display_left_1_digital,
		seting_storage_read2_entry,
		seting_storage_read2_before,
		seting_storage_read2_while,
		seting_storage_read2_after,
		TIMEOUT_COUNTS //超时5s
	);
}


/******************************************************************************/  
//SET_V://调整电压
/******************************************************************************/
void set_V_callback_entry()
{
	showing_data[0] = Dpy_wei_0 & Dpy_duan_negative;//横杠

	main_u16x = setI;//传参给下面这个函数
	main_u8x = FULLBRIGHT;//传参给下面这个函数
	display_right_3_digital();

	flashing_FSM = 0;//闪烁状态机
	flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
}

void set_V_callback_while()
{
    main_u16x = setV;//传参给下面这个函数
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    display_left_4_digital();
}

void menu_set_V()
{
	main_menu(
		
		SET_V,
		SET_I,
		SETING_V,
		CONTINUOUS_ADJUSTMENT_MODE_V,
		STATE_NONE,
		
		set_V_callback_entry,
		set_V_callback_while,
		
		TIMEOUT_COUNTS //超时5s
	);
}


/******************************************************************************/  
//SETING_V://调节电压
/******************************************************************************/
static void seting_V_entry()
{
	flashing_FSM = 1;//闪烁状态机
	flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
}

static void seting_V_after()
{
      //setV = bmq_turn_mgr_seting_data;
      output_PWM_update = 1;
      comm_save_vi(VOLT);
}

void do_seting_V()
{
	bqm_fucn(
		DATA_SET_V,
		COEFFICIENT_SETING_V,//coefficient
		set_V_limit,
		0,
		display_left_4_digital,
		seting_V_entry,
		NULL,
		NULL,/*while*/
		seting_V_after,
		TIMEOUT_COUNTS //超时5s
	);
}

/******************************************************************************/  
//SET_I://调整电压
/******************************************************************************/
void set_I_callback_entry()
{
	//owing_data[0] = Dpy_wei_0 & Dpy_duan_negative;//横杠

	main_u16x = setI;//传参给下面这个函数
	main_u8x = FULLBRIGHT;//传参给下面这个函数
	display_right_3_digital();

	flashing_FSM = 0;//闪烁状态机
	flashing_style[0] = STRONGBLINK;//灭<->亮闪烁模式
}

void set_I_callback_while()
{
    main_u16x = setI;//传参给下面这个函数
    main_u8x = flashing_style[flashing_FSM];//传参给下面这个函数
    display_right_3_digital();
}

void menu_set_I()
{
	main_menu(
		
		SET_V,
		SET_I,
		SETING_I,
		CONTINUOUS_ADJUSTMENT_MODE_I,
		STATE_NONE,
		
		set_I_callback_entry,
		set_I_callback_while,
		
		TIMEOUT_COUNTS //超时5s
	);
}

/******************************************************************************/  
//SETING_I://调节电流
/******************************************************************************/
static void seting_I_entry()
{
	flashing_FSM = 1;//闪烁状态机
	flashing_style[0] = WEAKBLINK;//半亮<->全亮闪烁模式
}

static void seting_I_after()
{
      //setI = bmq_turn_mgr_seting_data;
      output_PWM_update = 1;
      comm_save_vi(CURRENT);
}

void do_seting_I()
{
	bqm_fucn(
		DATA_SET_I,
		COEFFICIENT_SETING_I,//coefficient
		set_I_limit,
		0,
		display_right_3_digital,
		seting_I_entry,
		NULL,
		NULL,/*while*/
		seting_I_after,
		TIMEOUT_COUNTS //超时5s
	);
}

/******************************************************************************/  
//SETING_STORAGE_WRITE:
/******************************************************************************/
void do_write_animation()
{
	/* clear_showing_data_but34();
	u8 cnt = 0;
	while(1)
	{//读取提示动画
		switch(cnt){//动画处理
		case 0:

			showing_data[3] = Dpy_wei_3&(~_Dpy_duan_C);
			showing_data[4] = Dpy_wei_4&(~_Dpy_duan_E);
			cnt = 1;
		break;
		case 1:
			showing_data[3] = 0x0000;
			showing_data[4] = 0x0000;
			showing_data[3] = Dpy_wei_2&(~_Dpy_duan_A);
			showing_data[4] = Dpy_wei_5&(~_Dpy_duan_A);
			cnt = 2;
		break;
		case 2:
			showing_data[3] = Dpy_wei_3&(~_Dpy_duan_A);
			showing_data[4] = Dpy_wei_4&(~_Dpy_duan_A);
			cnt = 3;
		break;
		case 3:
			showing_data[3] = Dpy_wei_3&(~_Dpy_duan_B);
			showing_data[4] = Dpy_wei_4&(~_Dpy_duan_F);
			cnt = 4;
		break;
		case 4:
			showing_data[3] = Dpy_wei_3&(~_Dpy_duan_C);
			showing_data[4] = Dpy_wei_4&(~_Dpy_duan_E);
			cnt = 5;
		break;
		default:
			showing_data[1] = 0x0000;
			showing_data[6] = 0x0000;
			next_state = SETING_STORAGE_WRITE2;
			return;
		}
		btn_event = 0;
		user_timer1 = 5;
		do{
			chkbmq();
			if(btn_event == TURN_NONE )continue;
			switch(btn_event)
			{
				case TURN_RIGHT://编码器正转
				case TURN_LEFT://编码器反转
				//next_state = SETING_STORAGE_WRITE2; 
				return;
			}
		}while(user_timer1);
	}  */
}


/******************************************************************************/
//SETING_STORAGE_WRITE2 超时返回
/******************************************************************************/
 
static void seting_storage_write2_entry()
{
	flashing_FSM = 0;
	flashing_style[0] = STRONGBLINK;//半亮<->全亮闪烁模式
}

static void seting_storage_write2_before()
{
	main_u8x = FULLBRIGHT;
	
	main_u16x = setV;
	display_left_4_digital();
	main_u16x = setI;
	display_right_3_digital();
}

static void seting_storage_write2_after()
{
	main_u8x += EEPROM_ADDR_START_USER - 1;
	eeprom_write_unlock_addrx8();
	eeprom_buf1 = setV;
	eeprom_write();
	eeprom_buf1 = setI;
	eeprom_write();
	eeprom_write_lock();
}

void do_seting_storage_write2()
{
	bqm_fucn(
		DATA_MENU,
		COEFFICIENT_WRITE,//coefficient
		MAX_USER_ITEM,
		MIN_USER_ITEM,
		display_left_1_digital,
		seting_storage_write2_entry,
		seting_storage_write2_before,
		NULL,/*while*/
		seting_storage_write2_after,
		TIMEOUT_COUNTS //超时5s
	);
}


