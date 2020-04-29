
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
		do_factory_frame_menu_I();//显示方框菜单
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
