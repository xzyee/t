#ifndef DISPLAY_H
#define DISPLAY_H

#define _Dpy_wei_0 (0x0001<<9)
#define _Dpy_wei_1 (0x0001<<12)
#define _Dpy_wei_2 (0x0001<<13)
#define _Dpy_wei_3 (0x0001<<3)
#define _Dpy_wei_4 (0x0001<<0)
#define _Dpy_wei_5 (0x0001<<2)
#define _Dpy_wei_6 (0x0001<<1)
#define _Dpy_duan_A (0x0001<<10)
#define _Dpy_duan_B (0x0001<<11)
#define _Dpy_duan_C (0x0001<<4)
#define _Dpy_duan_D (0x0001<<6)
#define _Dpy_duan_E (0x0001<<7)
#define _Dpy_duan_F (0x0001<<8)
#define _Dpy_duan_G (0x0001<<14)
#define _Dpy_duan_DP (0x0001<<5)

/*
    单个数码管数字的字段数据，多种字段可用逻辑与运算结合显示（示例：Dpy_duan_0 & Dpy_duan_dp 就是显示“0.”）
*/
#define Dpy_duan_0      (~(_Dpy_duan_A|_Dpy_duan_B|_Dpy_duan_C|_Dpy_duan_D|_Dpy_duan_E|_Dpy_duan_F                         ))
#define Dpy_duan_1      (~(            _Dpy_duan_B|_Dpy_duan_C                                                             ))
#define Dpy_duan_2      (~(_Dpy_duan_A|_Dpy_duan_B            |_Dpy_duan_D|_Dpy_duan_E            |_Dpy_duan_G             ))
#define Dpy_duan_3      (~(_Dpy_duan_A|_Dpy_duan_B|_Dpy_duan_C|_Dpy_duan_D                        |_Dpy_duan_G             ))
#define Dpy_duan_4      (~(            _Dpy_duan_B|_Dpy_duan_C                        |_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_5      (~(_Dpy_duan_A            |_Dpy_duan_C|_Dpy_duan_D            |_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_6      (~(_Dpy_duan_A            |_Dpy_duan_C|_Dpy_duan_D|_Dpy_duan_E|_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_7      (~(_Dpy_duan_A|_Dpy_duan_B|_Dpy_duan_C                                                             ))
#define Dpy_duan_8      (~(_Dpy_duan_A|_Dpy_duan_B|_Dpy_duan_C|_Dpy_duan_D|_Dpy_duan_E|_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_9      (~(_Dpy_duan_A|_Dpy_duan_B|_Dpy_duan_C|_Dpy_duan_D            |_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_all    (~(_Dpy_duan_A|_Dpy_duan_B|_Dpy_duan_C|_Dpy_duan_D|_Dpy_duan_E|_Dpy_duan_F|_Dpy_duan_G|_Dpy_duan_DP))
#define Dpy_duan_null   (~(0x0000                                                                                          ))
#define Dpy_duan_negative (~(                                                                      _Dpy_duan_G             ))
#define Dpy_duan_dp     (~(                                                                                    _Dpy_duan_DP))
#define Dpy_duan_o      (~(                        _Dpy_duan_C|_Dpy_duan_D|_Dpy_duan_E            |_Dpy_duan_G             ))
#define Dpy_duan_f      (~(_Dpy_duan_A                                    |_Dpy_duan_E|_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_s      Dpy_duan_5
#define Dpy_duan_e      (~(_Dpy_duan_A                        |_Dpy_duan_D|_Dpy_duan_E|_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_t      (~(                                    _Dpy_duan_D|_Dpy_duan_E|_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_v      (~(                        _Dpy_duan_C|_Dpy_duan_D|_Dpy_duan_E                                     ))
#define Dpy_duan_r      (~(                                                _Dpy_duan_E            |_Dpy_duan_G             ))
#define Dpy_duan_a      (~(_Dpy_duan_A|_Dpy_duan_B|_Dpy_duan_C            |_Dpy_duan_E|_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_c      (~(_Dpy_duan_A                        |_Dpy_duan_D|_Dpy_duan_E|_Dpy_duan_F                         ))
#define Dpy_duan_n      (~(                        _Dpy_duan_C            |_Dpy_duan_E            |_Dpy_duan_G             ))
#define Dpy_duan_l      (~(                                    _Dpy_duan_D|_Dpy_duan_E|_Dpy_duan_F                         ))
#define Dpy_duan_y      (~(            _Dpy_duan_B|_Dpy_duan_C|_Dpy_duan_D            |_Dpy_duan_F|_Dpy_duan_G             ))
#define Dpy_duan_d      (~(            _Dpy_duan_B|_Dpy_duan_C|_Dpy_duan_D|_Dpy_duan_E            |_Dpy_duan_G             ))
#define Dpy_duan_m      (~(_Dpy_duan_A|_Dpy_duan_B|_Dpy_duan_C            |_Dpy_duan_E|_Dpy_duan_F                         ))

/*
    数码管的字位选通数据，Dpy_wei_0 是最左边的数码管，Dpy_wei_6 是最右边的数码管（示例：Dpy_wei_2 & Dpy_duan_0 & Dpy_duan_dp 就是在最左边第3个数码管显示“0.”）
*/
#define Dpy_wei_0 (_Dpy_wei_0|(~Dpy_duan_all))
#define Dpy_wei_1 (_Dpy_wei_1|(~Dpy_duan_all))
#define Dpy_wei_2 (_Dpy_wei_2|(~Dpy_duan_all))
#define Dpy_wei_3 (_Dpy_wei_3|(~Dpy_duan_all))
#define Dpy_wei_4 (_Dpy_wei_4|(~Dpy_duan_all))
#define Dpy_wei_5 (_Dpy_wei_5|(~Dpy_duan_all))
#define Dpy_wei_6 (_Dpy_wei_6|(~Dpy_duan_all))

#endif //#ifndef DISPLAY_H
