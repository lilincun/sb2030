/*****************************************************
**																									**
**	File:					Version:															**
**		Main.c				0.0.0.1															**
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

#define SB2010
#include "sb2010.h"

/*====================================================================================================*/
/*	System (&=12MHz)  					                        	  */
/*	mathine clock:12/&=1.0*(10e-6)S=1uS			                */
/*	T0 overtime:(12/&)*(65536-65286)S=250us			        	  */
/*	T1 overtime:(12/&)*(65536-64536)=1mS 					  */
/*====================================================================================================*/

void sys_init (void) {
#ifdef STC5616
	P3M0 = 0x00;
	P3M1 = 0x82;
#else
	P3M0 = 0x80;
	P3M1 = 0x00;
#endif
	P_mute = HIGH;
	
#ifdef STC5616
	P1M0 = 0x02;
	P1M1 = 0x41;		//P1.1为高阻状态
#else
	P1M0 = 0x41;
	P1M1 = 0x02;		//P1.1为高阻状态
#endif
	power(OFF);
	#ifdef subwoofer
	P_Limter = 1;
	#else
	P_Limter = 0;
	#endif
	
	IT0 = 1;				//外部中断0 下降沿触发
	EX0 = 0;

	IT1 = 1;				//外部中断1 下降沿触发
	EX1 = 1;	
	
	TMOD = 0x01;		//定时器0 方式2 定时250微妙，4MHz晶振
#ifdef f_12MHz
	TL0 = 0x06;
#else
	TL0 = 0xAD;
#endif
	TH0 = 0xFF;
	TR0 = 0;
	ET0 = 1;		

	TMOD += 0x20;		//定时器1 方式2 定时113微妙，4MHz晶振
#ifdef f_12MHz
	TL1 = 0x06;
	TH1 = 0x06;
#else
	TL1 = 0x36;
	TH1 = 0x36;
#endif
	TR1 = 1;
	ET1 =1;
	
	SCON = 0x00;
	PCON = 0x00;

	IP = 0x02;

	EA = 1;		
}

/*
code uchar tab_boost[7][2]={	
	{0x98,	0xA0},	//	+3
	{0x90,	0xA0},	//	+2
	{0x88,	0xA0},	//	+1
	{0x98,	0x90},	//	0
	{0x90,	0x90},	//	-1
	{0x98,	0x90},	//	-2	
	{0x88,	0x90},	//	-3
};	
*/

#ifdef STC5616
uchar read_flash_byte(uchar addr)
{
	ISP_ADDRH = ADDR_ROM;
	ISP_ADDRL = addr;
	ISP_CONTR = 0x83;
	ISP_CMD = CMD_READ;
	ISP_TRIG = 0x46;
	ISP_TRIG = 0xb9;
	_nop_();
	return(ISP_DATA);
}

void earse(void)
{
	ISP_ADDRH = ADDR_ROM;
	ISP_ADDRL = 0x00;
	ISP_CONTR = 0x83;
	ISP_CMD = CMD_EARSE;
	ISP_TRIG = 0x46;
	ISP_TRIG = 0xb9;
	_nop_();
}

void write_flash_byte(uchar addr, uchar dat)
{
	ISP_DATA = dat;
	ISP_ADDRH = ADDR_ROM;
	ISP_ADDRL = addr;
	ISP_CONTR = 0x83;
	ISP_CMD = CMD_WRITE;
	ISP_TRIG = 0x46;
	ISP_TRIG = 0xb9;
	_nop_();
}

void protect_flash(void)
{
	ISP_CONTR = 0;
	ISP_CMD = 0;
	ISP_TRIG = 0;
	ISP_ADDRH = 0xff;
	ISP_ADDRL = 0xff;
}

#else
uchar read_flash_byte(uchar addr) {
	IAP_ADDRH = 0x00;
	IAP_ADDRL = addr;
	IAP_CONTR = 0x85;
	IAP_CMD = CMD_READ;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
	return(IAP_DATA);
}

void earse(void) {
	IAP_ADDRH = 0x00;
	IAP_ADDRL = 0x00;
	IAP_CONTR = 0x85;
	IAP_CMD = CMD_EARSE;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
}

void write_flash_byte(uchar addr, uchar dat) {
	IAP_DATA = dat;
	IAP_ADDRH = 0x00;
	IAP_ADDRL = addr;
	IAP_CONTR = 0x85;
	IAP_CMD = CMD_WRITE;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
}

void protect_flash(void) {
	IAP_CONTR = 0;
	IAP_CMD = 0;
	IAP_TRIG = 0;
	IAP_ADDRH = 0x80;
	IAP_ADDRL = 0x00;
}
#endif
void read_var(void) {
	input = read_flash_byte(0x00);
	Mvol = read_flash_byte(0x01);
	Bvol = read_flash_byte(0x02);
	dim = read_flash_byte(0x03);
	sound = read_flash_byte(0x04);
	if(input >= IN_MAX) input = IN_TV;
	if(Mvol > MVOL_MAX) Mvol = 30;
	if(Bvol > BVOL_MAX) Bvol = 6;
	if((dim > DIM_MAX) ||(dim < DIM_MIN)) dim = DIM_MIN;
	if(sound >= SOUND_MAX) sound = STANDARD;
}

void write_var(void) {
	earse();
	write_flash_byte(0x00, input);
	write_flash_byte(0x01, Mvol);
	write_flash_byte(0x02, Bvol);
	write_flash_byte(0x03, dim);
	write_flash_byte(0x04, sound);
}
#ifdef NJW1180
#ifdef subwoofer
	code uchar tab_tone[4][15] =
	{
		{0x38,0x30,0x28,0x20,0x18,0x10,0x08,0x80,0x88,0x90,0x98,0xa0,0xa8,0x04,0xb6}, // STANDARD
		{0x20,0x18,0x10,0x08,0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,0xc0,0x04,0xba}, // MOVIE/GAME
		{0x38,0x30,0x28,0x20,0x18,0x10,0x08,0x80,0x88,0x90,0x98,0xa0,0xa8,0x04,0xb8}, // MUSIC
		{0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x30,0x28,0x20,0x18,0x10,0x88,0x34,0xb6} // NEWS
	};
#else
	code uchar tab_tone[4][15] =
	{
		{0x08,0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,0xc0,0xc8,0xd0,0xd8,0x04,0xb6}, // STANDARD
		{0x90,0x98,0xa0,0xa8,0xb0,0xb8,0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0x04,0xba}, // MOVIE/GAME
		{0x08,0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,0xc0,0xc8,0xd0,0xd8,0x04,0xb8}, // MUSIC
		{0x38,0x30,0x28,0x20,0x18,0x10,0x08,0x80,0x88,0x90,0x98,0xa0,0xa8,0x34,0xb6} // NEWS
	};
#endif
#else
#ifdef subwoofer
	code uchar tab_tone[4][15] =
	{
		{0x48,0x40,0x38,0x30,0x28,0x20,0x18,0x10,0x08,0x80,0x88,0x90,0x98,0x04,0xa6}, // STANDARD
		{0x30,0x28,0x20,0x18,0x10,0x08,0x80,0x88,0x90,0x98,0xA0,0xA8,0xB0,0x04,0xaa}, // MOVIE/GAME
		{0x48,0x40,0x38,0x30,0x28,0x20,0x18,0x10,0x08,0x80,0x88,0x90,0x98,0x04,0xa8}, // MUSIC
		{0x78,0x70,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x30,0x28,0x20,0x18,0x34,0xa6} // NEWS
	};
#else
	code uchar tab_tone[4][15] =
	{
		{0x18,0x10,0x08,0x80,0x88,0x90,0x98,0xA0,0xA8,0xB0,0xB8,0xC0,0xC8,0x04,0xa6}, // STANDARD
		{0x80,0x88,0x90,0x98,0xA0,0xA8,0xB0,0xB8,0xC0,0xC8,0xD0,0xD8,0xE0,0x04,0xaa}, // MOVIE/GAME
		{0x18,0x10,0x08,0x80,0x88,0x90,0x98,0xA0,0xA8,0xB0,0xB8,0xC0,0xC8,0x04,0xa8}, // MUSIC
		{0x48,0x40,0x38,0x30,0x28,0x20,0x18,0x10,0x08,0x80,0x88,0x90,0x98,0x34,0xa6} // NEWS
	};
#endif
#endif
code uchar VOL_table[ ] = {
	0x00,  //  0
	0x87,  //  1
	0x90,  //  2
	0x99,  //  3
	0xA0,  //  4
	0xA8,  //  5
	0xAF,  //  6
	0xB2,  //  7
	0xB5,  //  8
	0xB8,  //  9
	0xBB,  //  10
	0xBE,  //  11
	0xC0,  //  12
	0xC2,  //  13
	0xC4,  //  14
	0xC7,  //  15
	0xC9,  //  16
	0xCB,  //  17
	0xCD,  //  18
	0xD0,  //  19
	0xD2,  //  20
	0xD3,  //  21
	0xD5,  //  22
	0xD6,  //  23
	0xD8,  //  24
	0xD9,  //  25
	0xDB,  //  26
	0xDC,  //  27
	0xDE,  //  28
	0xDF,  //  29
	0xE1,  //  30
	0xE2,  //  31
	0xE3,  //  32
	0xE4,  //  33
	0xE5,  //  34
	0xE6,  //  35
	0xE8,  //  36
	0xE9,  //  37
	0xEA,  //  38
	0xEB,  //  39
	0xEC,  //  40
	0xED,  //  41
	0xEE,  //  42
	0xEE,  //  43
	0xEF,  //  44
	0xF0,  //  45
	0xF1,  //  46
	0xF1,  //  47
	0xF2,  //  48
	0xF3,  //  49
	0xF4,  //  50
	0xF4,  //  51
	0xF4,  //  52
	0xF5,  //  53
	0xF5,  //  54
	0xF5,  //  55
	0xF6,  //  56
	0xF6,  //  57
	0xF7,  //  58
	0xF7,  //  59
	0xF7,  //  60
	0xF8,  //  61
	0xF8,  //  62
	0xF8,  //  63
	0xF9,  //  64
	0xF9,  //  65
	0xFA,  //  66
	0xFA,  //  67
	0xFA,  //  68
	0xFB,  //  69
	0xFB,  //  70
	0xFB,  //  71
	0xFB,  //  72
	0xFC,  //  73
	0xFC,  //  74
	0xFC,  //  75
	0xFC,  //  76
	0xFD,  //  77
	0xFD,  //  78
	0xFD,  //  79
	0xFD,  //  80
	0xFD,  //  81
	0xFE,  //  82
	0xFE,  //  83
	0xFE,  //  84
	0xFE,  //  85
	0xFE,  //  86
	0xFE,  //  87
	0xFE,  //  88
	0xFF,  //  89
	0xFF,  //  90
	0xFF,  //  91
	0xFF,  //  92
	0xFF,  //  93
	0xFF,  //  94
	0xFF,  //  95
	0xFF,  //  96
	0xFF,  //  97
	0xFF,  //  98
	0xFF,  //  99
	0xFF  //  100
};

void mute(uchar f_mute) {
	P_mute = f_mute;
	timing(mute_time, T_200ms);
	while(!time_up(mute_time));
}
/*
void set_out(void) {
	uchar vol, agc;
	if(Mvol > 72) {
		vol = 2;
		agc = 0x07;
	}
	else {
		vol = 0;
		agc = 0;
	}
	if(Mvol == 0) {
		wr_I2C(0x82, 0x00, 0);
	}
	else {
		wr_I2C(0x82, 0x00, VOL_table[Mvol] - vol - 0x12);
	}
	wr_I2C(0x82, 0x01, input);
	wr_I2C(0x82, 0x02, tab_tone[sound][Bvol] + agc);
	wr_I2C(0x82, 0x03, tab_tone[sound][13]);
	wr_I2C(0x82, 0x04, tab_tone[sound][14]);
}
*/
void set_out(void)
{
	if(Mvol == 0)	wr_I2C(0x82, 0x00, 0);
	else wr_I2C(0x82, 0x00, VOL_table[Mvol] - 0x1C);
	wr_I2C(0x82, 0x01, input);
	wr_I2C(0x82, 0x02, tab_tone[sound][Bvol]);
	wr_I2C(0x82, 0x03, tab_tone[sound][13]);
	wr_I2C(0x82, 0x04, tab_tone[sound][14]);
}
uchar pro_input(uchar key) {
	switch(key) {
		case K_INPUT:
			if(++ input >= IN_MAX) {
				input = IN_TV;
			}
			break;
		case TV:
			if(input == IN_TV)return(0);
			input = IN_TV;
			break;
		case MP3:
			if(input == IN_MP3)return(0);
			input = IN_MP3;
			break;
		case AUX:
			if(input == IN_AUX)return(0);
			input = IN_AUX;
			break;
	}
	f_save = 1;
	return(1);
}

void pro_Mvol(uchar key) {
	switch(key) {
		case K_MVOL_INC:
		case MVOL_inc:
			f_mute = OFF;
			if(Mvol < MVOL_MAX) {
				Mvol ++;
				f_save = 1;
			}
			break;
		case K_MVOL_DEC:
		case MVOL_dec:
			if(Mvol > MVOL_MIN) {
				Mvol --;
				f_save = 1;
			}
			break;
	}
}
void pro_Bvol(uchar key) {
	switch(key) {
		case BVOL_inc:
			if(Bvol < BVOL_MAX) {
				Bvol ++;
				f_save = 1;
			}
			break;
		case BVOL_dec:
			if(Bvol > BVOL_MIN) {
				Bvol --;
				f_save = 1;
			}
			break;
	}
}
void pro_sound(uchar key) {
	switch(key) {
		case K_SOUND:
			if(++ sound >= SOUND_MAX) {
				sound = STANDARD;
			}
			break;
		case R_STANDARD:
			if(sound == STANDARD) return;
			sound = STANDARD;
			break;
		case R_MUSIC:
			if(sound == MUSIC) return;
			sound = MUSIC;
			break;
		case R_MOVIE:
			if(sound == MOVIE) return;
			sound = MOVIE;
			break;
		case R_NEWS:
			if(sound == NEWS) return;
			sound = NEWS;
			break;
	}
	f_save = 1;
}

code uchar tab_sleep[] = {0, 5, 10, 15, 30, 45, 60, 90};

void pro_sleep(void) {
	static uchar i;
	if(f_sleep == 0) {
		f_sleep = 1;
		i = 1;
	}
	else if(i < 7) {
		i ++;
	}
	else {
		f_sleep = 0;
		i = 0;
	}
	t_sleep = tab_sleep[i];
}

void st_stdby(struct state * me) {
	switch(me->sig) {
		case SIG_entry:
			timing(time, T_500ms);
			while(!time_up(time));
			power(OFF);
			timing(time, T_200ms);
			while(!time_up(time));
			disp(DISP_DARK, T_always);
			to_idle();
			break;
		case SIG_idle:
			break;
		case SIG_exit:
			power(ON);
			f_mute = OFF;
			timing(time, T_200ms);
			while(!time_up(time));
			disp(DISP_INPUT, T_always);
			exit();
			break;
		case SIG_K_stdby:	//	待机键
			tran(ST_NORM);  
			break;
		case SIG_K_sound:
			if((me->key == R_STANDARD) && (preset == 1)) preset = 2;
			if((me->key == R_NEWS) && (preset == 2)) preset = 3;
			to_idle();
			break;
		case SIG_K_mute:
			if(preset == 3)
			{
				preset = 0;
				input = IN_TV;
				Mvol = 30;
				Bvol = 6;
				dim = DIM_MIN;
				sound = STANDARD;
				write_var();
				protect_flash();
				disp(DISP_OK, T_3s);
			}
			to_idle();
			break;
		default:
			to_idle();
			break;
	}
}
void st_normal(struct state * me) {
	switch(me->sig) {
		case SIG_entry:
			timing(time, T_10m);
			to_idle();
			break;
		case SIG_idle:	//	空闲时检测是否有信号，如果10 分钟都无信号则进入待机
			if((signal()) && (f_mute == OFF)) {
				tran(ST_PLAY);
			}
			else if(time_up(time)) {
				tran(ST_STDBY);
			}
			break;
		case SIG_exit:
			exit();
			break;
		case SIG_K_stdby:	//	待机键
			preset = 1;
		case SIG_sleep:
			f_mute = OFF;
			f_sleep = 0;
			t_sleep = 0;
			tran(ST_STDBY);
			break;
		case SIG_K_mute:	//	静音键
			if(f_mute == ON){
				f_mute = OFF;
				disp(DISP_INPUT,T_always);
			}
			else {
				f_mute = ON;
				disp(DISP_MUTE,T_always);
			}
			tran(ST_NORM);
			break;
		case SIG_K_input:	//	输入键
			if(pro_input(me->key)){
				set_out();
			}
			if(f_mute == ON){disp(DISP_INPUT,T_1s);}
			else {disp(DISP_INPUT,T_always);}
			tran(ST_NORM);
			break;
		case SIG_K_Mvol:	//	音量键
			pro_Mvol(me->key);
			set_out();
			disp(DISP_MVOL, T_1s);
			tran(ST_NORM);
			break;
		case SIG_K_Bvol:	//	音量键
			pro_Bvol(me->key);
			set_out();
			disp(DISP_BVOL, T_1s);
			tran(ST_NORM);
			break;
		case SIG_K_dim:	//	调光键
			if(-- dim < DIM_MIN) {
				dim = DIM_MAX;
			}
			f_save = 1;
			to_idle();
			break;
		case SIG_K_sound:	//	音效键
			pro_sound(me->key);
			set_out();
			tran(ST_NORM);
			break;
		case SIG_K_sleep:	//	睡眠键
			if(me->disp == DISP_SLEEP) {
				pro_sleep();
			}
			disp(DISP_SLEEP, T_3s);
			to_idle();
			break;
		default:
			to_idle();
			break;
	}
}
void st_play(struct state * me) {
	switch(me->sig) {
		case SIG_entry:
			mute(MUTE_OFF);
			set_out();
			timing(time, T_5s);
			to_idle();
			break;
		case SIG_idle:	//	空闲时检测是否有信号，如果2秒钟都无信号则进入通常状态
			if(signal()) {
				timing(time, T_5s);
			}
			else	if(time_up(time)) {
				tran(ST_NORM);
			}
			break;
		case SIG_exit:
			mute(MUTE_ON);
			set_out();
			exit();
			break;
		case SIG_K_stdby:	//	待机键
			preset = 1;
		case SIG_sleep:
			f_mute = OFF;
			f_sleep = 0;
			t_sleep = 0;
			tran(ST_STDBY);
			break;
		case SIG_K_mute:	//	静音键
			f_mute = ON;
			disp(DISP_MUTE, T_always);
			tran(ST_NORM);
			break;
		case SIG_K_input:	//	输入键
			if(pro_input(me->key)){ 
				mute(MUTE_ON);
				set_out();
				tran(ST_NORM);
			}
			else {
				to_idle();
			}
			disp(DISP_INPUT,T_always);
			break;
		case SIG_K_Mvol:	//	音量键
			pro_Mvol(me->key);
			set_out();
			disp(DISP_MVOL, T_1s);
			to_idle();
			break;
		case SIG_K_Bvol:	//	音量键
			pro_Bvol(me->key);
			set_out();
			disp(DISP_BVOL, T_1s);
			to_idle();
			break;
		case SIG_K_dim:	//	调光键
			if(-- dim < DIM_MIN) {
				dim = DIM_MAX;
			}
			f_save = 1;
			to_idle();
			break;
		case SIG_K_sound:	//	音效键
			pro_sound(me->key);
			set_out();
			to_idle();
			break;
		case SIG_K_sleep:	//	睡眠键
			if(me->disp == DISP_SLEEP) {
				pro_sleep();
			}
			disp(DISP_SLEEP, T_3s);
			to_idle();
			break;
		default:
			to_idle();
			break;
	}
}


void sys_dispatch(struct state * me) {
	switch(me->st) {
		case ST_STDBY:
			st_stdby(me);
			break;
		case ST_NORM:
			st_normal(me);
			break;
//		case ST_MUTE:
//			st_mute(me);
//			break;
		case ST_PLAY:
			st_play(me);
			break;
		
	}
}

code uchar REM_CODE[] = 
{
	0x00,	//	NOKEY 				0
	0x01,	//	STDBY					13
	0x0d,	//	DIMMER				14
	0x0a,	//	MUTE					15
	0x03,	//	STANDARD			16
	0x1f,	//	MOVIE/GAME		17
	0x2f,	//	MUSIC					18
	0x3d,	//	NEWS					19
	0x11,	//	VOL+					20
	0x14,	//	VOL-					21
	0x13,	//	BVOL+					22
	0x12,	//	BVOL-					23
	0x15,	//	TV						24
	0x16,	//	AUX						25
	0x1e,	//	MP3						26
	0x0e,	//	SLEEP					27
};

uchar rem_key (void) {
	uchar i;
	for(i = (R_MAX - 1); i > 0; i --) {
		if(Rem_data == REM_CODE[i]) return(i);
	}
}

uchar pannel_key (void){
	uchar key = NOKEY;
	if(P_k1 == LOW) key = K_INPUT;
	if(P_k2 == LOW) key = K_STDBY;
	if((P_k1 == LOW) && (P_k2 == LOW)) key = K_SOUND;
	if(key == NOKEY) {
		P_k1 = LOW; NOP();
		if(P_k2 == LOW) key = K_MVOL_INC;
		P_k1 = HIGH;
	}
	if(key == NOKEY) {
		P_k2 = LOW; NOP();
		if(P_k1 == LOW) key = K_MVOL_DEC;
		P_k2 = HIGH;
	}
	return(key);
}

void read_key(struct state * me) {
	me->key = rem_key();
	if (me->key == NOKEY) me->key = pannel_key();
}

code uchar tab_sig[] = 
{
	SIG_idle,
	SIG_K_stdby,
	SIG_K_dim,
	SIG_K_mute,
	SIG_K_sound,
	SIG_K_sound,
	SIG_K_sound,
	SIG_K_sound,
	SIG_K_Mvol,
	SIG_K_Mvol,
	SIG_K_Bvol,
	SIG_K_Bvol,
	SIG_K_input,
	SIG_K_input,
	SIG_K_input,
	SIG_K_sleep,
	SIG_K_input,
	SIG_K_stdby,
	SIG_K_sound,
	SIG_K_Mvol,
	SIG_K_Mvol
};

void get_sig(struct state * me) {
	static uchar r_key;
	f_keep = 0;
	if(me->sig == SIG_idle) {
		if(r_key == me->key) {
			switch(me->key) {
				case K_MVOL_INC:
				case K_MVOL_DEC:
				case MVOL_inc:
				case MVOL_dec:
				case BVOL_inc:
				case BVOL_dec:
					if(key_time == 0) {
						key_time = T_100ms;
						me->sig = tab_sig[me->key];
					}
					f_keep = 1;
					break;
			}
		}
		else {
			r_key = me->key;
			me->sig = tab_sig[me->key];
			key_time = T_400ms;
		}
		if(me->sig == SIG_idle) {
			if((f_sleep == 1) && (t_sleep == 0)) {
				f_sleep = 0;
				me->sig = SIG_sleep;
			}
		}
	}
}

#define EEPROM

/***************************************************************void main(void)***********************************************************************
                                                                        Program start and main loop process
**********************************************************************************************************************************************************/
void main(void) {
	struct state me, * p;
	uchar count;
	sys_init();
	me.st = ST_STDBY;
	me.sig = SIG_idle;
	p = &me;
	Mvol = 30;
	sound = STANDARD;
	dim = DIM_MIN;
	Bvol=3;
	f_sleep = 0;
#ifdef EEPROM
	read_var();
#endif
	while(1)	{
		get_sig(p);
		sys_dispatch(p);
		disp_dispatch(p);
		if(f_T20ms  == 1){
			f_T20ms = 0;
			refresh();
			read_key(p);
		#ifdef EEPROM
			if(f_save == 1) {
				if(count > 50) {
					if(f_keep == 0) {
						count = 0;
						f_save = 0;
						write_var();
						protect_flash();
					}
				}
				else count ++;
			}
		#endif
		}
	}
}

