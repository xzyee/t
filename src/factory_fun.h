#ifndef FACTORY_FUN_H
#define FACTORY_FUN_H


enum{
	Vlim = 2,
	Vcal = 3
};
enum{
	Ilim = 2,
	Ical = 3,
};


extern void show_factory_mode_menu();

extern void do_set_0p5V_or_0p5A_ref();

extern void display_two_zero(u8 c);


extern void do_factory_frame_menu_V();
extern void do_factory_set_V_limit();

extern void do_factory_set_V_ref();
extern void do_factory_cal_V_ref();
extern void do_factory_frame_menu_I();
extern void do_factory_set_I_limit();
extern void do_factory_set_I_ref();
extern void do_factory_cal_I_ref();
extern void do_factory_mode_save();

#endif //#ifndef FACTORY_FUN_H
