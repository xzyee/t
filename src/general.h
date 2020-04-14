#ifndef GENARAL_H
#define GENARAL_H

typedef   signed char     s8;
typedef   signed short    s16;
typedef   signed long     s4;
typedef unsigned char     u8;
typedef unsigned short    u16;
typedef unsigned long     u32;
typedef void              v;

#define __JOIN4(A,B,C,D) A##B##C##D
#define PBinit(Px, Bn, dir, up_pull_ON, HISPD_ON_or_INT_ON) __JOIN4(P,Px,_DDR_DDR,Bn)  =  dir; __JOIN4(P,Px,_CR1_C1,Bn)  =  up_pull_ON; __JOIN4(P,Px,_CR2_C2,Bn)  =  HISPD_ON_or_INT_ON

#define PAO PA_ODR
#define PA0O PA_ODR_ODR0
#define PA1O PA_ODR_ODR1
#define PA2O PA_ODR_ODR2
#define PA3O PA_ODR_ODR3
#define PA4O PA_ODR_ODR4
#define PA5O PA_ODR_ODR5
#define PA6O PA_ODR_ODR6
#define PA7O PA_ODR_ODR7

#define PBO PB_ODR
#define PB0O PB_ODR_ODR0
#define PB1O PB_ODR_ODR1
#define PB2O PB_ODR_ODR2
#define PB3O PB_ODR_ODR3
#define PB4O PB_ODR_ODR4
#define PB5O PB_ODR_ODR5
#define PB6O PB_ODR_ODR6
#define PB7O PB_ODR_ODR7

#define PCO PC_ODR
#define PC0O PC_ODR_ODR0
#define PC1O PC_ODR_ODR1
#define PC2O PC_ODR_ODR2
#define PC3O PC_ODR_ODR3
#define PC4O PC_ODR_ODR4
#define PC5O PC_ODR_ODR5
#define PC6O PC_ODR_ODR6
#define PC7O PC_ODR_ODR7

#define PDO PD_ODR
#define PD0O PD_ODR_ODR0
#define PD1O PD_ODR_ODR1
#define PD2O PD_ODR_ODR2
#define PD3O PD_ODR_ODR3
#define PD4O PD_ODR_ODR4
#define PD5O PD_ODR_ODR5
#define PD6O PD_ODR_ODR6
#define PD7O PD_ODR_ODR7

#define PA0I PA_IDR_IDR0
#define PA1I PA_IDR_IDR1
#define PA2I PA_IDR_IDR2
#define PA3I PA_IDR_IDR3
#define PA4I PA_IDR_IDR4
#define PA5I PA_IDR_IDR5
#define PA6I PA_IDR_IDR6
#define PA7I PA_IDR_IDR7
#define PB0I PB_IDR_IDR0
#define PB1I PB_IDR_IDR1
#define PB2I PB_IDR_IDR2
#define PB3I PB_IDR_IDR3
#define PB4I PB_IDR_IDR4
#define PB5I PB_IDR_IDR5
#define PB6I PB_IDR_IDR6
#define PB7I PB_IDR_IDR7
#define PC0I PC_IDR_IDR0
#define PC1I PC_IDR_IDR1
#define PC2I PC_IDR_IDR2
#define PC3I PC_IDR_IDR3
#define PC4I PC_IDR_IDR4
#define PC5I PC_IDR_IDR5
#define PC6I PC_IDR_IDR6
#define PC7I PC_IDR_IDR7
#define PD0I PD_IDR_IDR0
#define PD1I PD_IDR_IDR1
#define PD2I PD_IDR_IDR2
#define PD3I PD_IDR_IDR3
#define PD4I PD_IDR_IDR4
#define PD5I PD_IDR_IDR5
#define PD6I PD_IDR_IDR6
#define PD7I PD_IDR_IDR7

#define PA0D PA_DDR_DDR0
#define PA1D PA_DDR_DDR1
#define PA2D PA_DDR_DDR2
#define PA3D PA_DDR_DDR3
#define PA4D PA_DDR_DDR4
#define PA5D PA_DDR_DDR5
#define PA6D PA_DDR_DDR6
#define PA7D PA_DDR_DDR7
#define PB0D PB_DDR_DDR0
#define PB1D PB_DDR_DDR1
#define PB2D PB_DDR_DDR2
#define PB3D PB_DDR_DDR3
#define PB4D PB_DDR_DDR4
#define PB5D PB_DDR_DDR5
#define PB6D PB_DDR_DDR6
#define PB7D PB_DDR_DDR7
#define PC0D PC_DDR_DDR0
#define PC1D PC_DDR_DDR1
#define PC2D PC_DDR_DDR2
#define PC3D PC_DDR_DDR3
#define PC4D PC_DDR_DDR4
#define PC5D PC_DDR_DDR5
#define PC6D PC_DDR_DDR6
#define PC7D PC_DDR_DDR7
#define PD0D PD_DDR_DDR0
#define PD1D PD_DDR_DDR1
#define PD2D PD_DDR_DDR2
#define PD3D PD_DDR_DDR3
#define PD4D PD_DDR_DDR4
#define PD5D PD_DDR_DDR5
#define PD6D PD_DDR_DDR6
#define PD7D PD_DDR_DDR7

#define EXTI0_PORT_A_vector               0x05
#define EXTI1_PORT_B_vector               0x06
#define EXTI2_PORT_C_vector               0x07
#define EXTI3_PORT_D_vector               0x08
#define EXTI4_PORT_E_vector               0x09
 
#define ITC_VECT0SPR         ITC_SPR1_VECT0SPR
#define ITC_VECT1SPR         ITC_SPR1_VECT1SPR
#define ITC_VECT2SPR         ITC_SPR1_VECT2SPR
#define ITC_VECT3SPR         ITC_SPR1_VECT3SPR
#define ITC_VECT4SPR         ITC_SPR2_VECT4SPR
#define ITC_VECT5SPR         ITC_SPR2_VECT5SPR
#define ITC_VECT6SPR         ITC_SPR2_VECT6SPR
#define ITC_VECT7SPR         ITC_SPR2_VECT7SPR
#define ITC_VECT8SPR         ITC_SPR3_VECT8SPR
#define ITC_VECT9SPR         ITC_SPR3_VECT9SPR
#define ITC_VECT10SPR        ITC_SPR3_VECT10SPR
#define ITC_VECT11SPR        ITC_SPR3_VECT11SPR
#define ITC_VECT12SPR        ITC_SPR4_VECT12SPR
#define ITC_VECT13SPR        ITC_SPR4_VECT13SPR
#define ITC_VECT14SPR        ITC_SPR4_VECT14SPR
#define ITC_VECT15SPR        ITC_SPR4_VECT15SPR
#define ITC_VECT16SPR        ITC_SPR5_VECT16SPR
#define ITC_VECT17SPR        ITC_SPR5_VECT17SPR
#define ITC_VECT18SPR        ITC_SPR5_VECT18SPR
#define ITC_VECT19SPR        ITC_SPR5_VECT19SPR
#define ITC_VECT20SPR        ITC_SPR6_VECT20SPR
#define ITC_VECT21SPR        ITC_SPR6_VECT21SPR
#define ITC_VECT22SPR        ITC_SPR6_VECT22SPR
#define ITC_VECT23SPR        ITC_SPR6_VECT23SPR
#define ITC_VECT24SPR        ITC_SPR7_VECT24SPR
#define ITC_VECT25SPR        ITC_SPR7_VECT25SPR
#define ITC_VECT26SPR        ITC_SPR7_VECT26SPR
#define ITC_VECT27SPR        ITC_SPR7_VECT27SPR
#define ITC_VECT28SPR        ITC_SPR8_VECT28SPR
#define ITC_VECT29SPR        ITC_SPR8_VECT29SPR

#define EEPROM_ADDRESS 0x4000
#define NULL 0

#define ADC_DR (*(u16 *)&ADC_DRH)

#define HIGH(x) (*(0+(u8 *)(&(x))))
#define LOW(x) (*(1+(u8 *)(&(x))))


#define HEXTOBIT(hex)	(\
						((hex%0x10))\
						|((hex/0x10%0x10)<<1)\
						|((hex/0x100%0x10)<<2)\
						|((hex/0x1000%0x10)<<3)\
						|((hex/0x10000%0x10)<<4)\
						|((hex/0x100000%0x10)<<5)\
						|((hex/0x1000000%0x10)<<6)\
						|((hex/0x10000000%0x10)<<7)\
						)



#define eeprom_write_lock() FLASH_IAPSR_DUL = 0

#endif //#ifndef GENARAL_H
