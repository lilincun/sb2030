/*****************************************************
**																									**
**	File:					Version:															**
**		sb2010.h				0.0.0.1														**
**																									**
**	Author:				Last date modified:										**
**		Li.Lincun				2010-02-03											**
**																									**
**	Functional description:															**
**		Main system function															**
**																									**
**																									**
******************************************************
**																									**
**	This software may only be used or reproduced under	the	**
**	valid permission from the author. Any full or partial usage	**
**	or reproduction of this software shall retain this notice.		**
**																									**
******************************************************/

#define f_12MHz

#ifndef sb2010_h
#define sb2010_h

#ifdef  SB2010
	#define EXT
#else
	#define EXT  extern 
#endif

#include "defines.h"
#ifdef STC5616
#include "stc12c5620ad.h"
#else
#include "stc11f04.h"
#endif
#include "intrins.h"

/*-----------port ---------*/

sbit P_stdby = P1^0;
sbit P_signal = P1^1;
sbit P_k1 = P1^2;
sbit P_disp_data = P1^3;
sbit P_disp_scl = P1^4;
sbit P_disp_stb = P1^5;
sbit P_Limter = P1^6;
sbit P_LED2 = P1^7;

sbit P_k2 = P3^2;
sbit P_remote = P3^3;
sbit P_I2C_scl = P3^4;
sbit P_I2C_sda = P3^5;
sbit P_mute = P3^7;

#define power(act_) {\
	P_stdby = act_;\
}
#define tran(state_) {\
	me->sig = SIG_exit;\
	me->target = state_;\
}
#define exit() {\
	me->st = me->target;\
	me->sig = SIG_entry;\
}
#define timing(timer_, time_) {\
	timer_ = time_;\
}
#define time_up(timer_) (timer_ == 0)
#define to_idle() {\
	me->sig = SIG_idle;\
}
#define signal() P_signal == HIGH
#define disp(disp_, time_) {\
	me->disp = disp_;\
	disp_time = time_;\
}
#define sleep() {\
	PCON = 0x02;\
	_nop_();_nop_();_nop_();_nop_();\
}
#define NOP() {\
	_nop_();\
}

#define T_always 0
#define T_100ms 5
#define T_120ms 6
#define T_200ms 10
#define T_400ms 20
#define T_500ms 25
#define T_1s 50
#define T_2s 100
#define T_3s 150
#define T_5s 250
#define T_10s 500
#define T_5m 15000
#define T_10m 30000

#define MVOL_MIN 0
#define MVOL_MAX 100
#define BVOL_MIN 0
#define BVOL_MAX 12

#define CMD_DISPMOD     0x02
#define CMD_ADDRMOD    0x40
#define CMD_ADDRRST     0xC0
#define CMD_DISPDARK    0x80
#define CMD_DISPNORM   0x8D
#ifdef STC5616
#define ADDR_ROM 0x2E
#endif
#define CMD_READ 1
#define CMD_WRITE 2
#define CMD_EARSE 3


struct state {
	uchar st, sig, key, target, disp;
};

enum OUT_STATE {
   ST_STDBY, ST_NORM, ST_MUTE, ST_PLAY
};

enum DISP_STATE {
   DISP_DARK, DISP_INPUT, DISP_MUTE, DISP_MVOL, DISP_BVOL, DISP_SLEEP, DISP_OK
};

enum SYS_SIGNAL {
   SIG_idle, SIG_entry, SIG_exit, SIG_user
};

enum USER_SIGNAL {
	SIG_K_stdby = SIG_user,
	SIG_K_mute, SIG_K_Mvol, SIG_K_Bvol, SIG_K_input, SIG_K_sound, SIG_K_dim,SIG_K_sleep, SIG_sleep
};

enum SOUND {
   STANDARD, MOVIE, MUSIC, NEWS, SOUND_MAX
};

enum DIMMER {
   DIM_DARK, DIM_MIN, DIM_MID, DIM_MAX
};

typedef enum {
	IN_TV, IN_AUX, IN_MP3, IN_MAX
} input_Option;

enum REM_KEY {
	NOKEY,				// 0
	STDBY	,				//	1
	DIM,					//	2
	MUTE,					//	3
	R_STANDARD,	//	4
	R_MOVIE,			//	5
	R_MUSIC,			//	6
	R_NEWS,			//	7
	MVOL_inc,			//	8
	MVOL_dec,			//	9
	BVOL_inc,			//	10
	BVOL_dec,			//	11
	TV,						//	12
	MP3,					//	13
	AUX,					//	14
	SLEEP,				//	15
	R_MAX				//	16
};
enum PANNEL_KEY {
	K_INPUT = R_MAX,				  // 16
	K_STDBY,							     // 17
	K_SOUND,						        // 18					
	K_MVOL_INC,						 // 19
	K_MVOL_DEC							 // 20					
};

EXT uint time, t_rem;
EXT input_Option input;
EXT uchar Mvol, Bvol, sound, dim;
EXT uchar disp_time,key_time, mute_time, t_sleep;
EXT Bool f_T20ms, f_rem, f_keep, f_mute, f_sleep, f_save;
EXT uchar Rem_data, preset;
EXT uchar disp_cmd;
EXT uchar disp_ram[12];

EXT uchar wr_I2C(uchar ,uchar ,uchar ) ;
EXT void refresh( void );
EXT void disp_dispatch(struct state *);


#endif

