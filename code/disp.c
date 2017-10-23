/*****************************************************
**																									**
**	File:					Version:															**
**		disp.c				0.0.0.1																**
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

#include "sb2010.h"

enum
{
	_dark,				// 00
	_0,						// 01	
	_1,						// 02
	_2,						// 03
	_3,						// 04
	_4,						// 05
	_5,						// 06
	_6,						// 07
	_7,						// 08
	_8,						// 09
	_9,						// 10
	_A,						// 11
	_b,						// 12
	_C,						// 13
	_E,						// 14
	_F,						// 15
	_M,						// 16
	_P,						// 17
	_T,						// 18
	_U,						// 19
	_V,						// 20
	_X,						// 21
	_plus,					// 22
	_minus,				// 23
	_dot,					// 24
	_L,						// 25
	_I,						// 26
	_N,						// 27
	_K,						// 28
};	

code uint tab_char[] = {
	 0x0000,		// dark		00
	 0x3F00,		// 0			01
	 0x0021,		// 1			02
	 0x5B00,		// 2			03
	 0x4F00,		// 3			04
	 0x6600,		// 4			05
	 0x6D00,		// 5			06
	 0x7D00,		// 6			07
	 0x0700,		// 7			08
	 0x7F00,		// 8			09
	 0x6F00,		// 9			10
	 0x7700,  	 // A			11
	 0x7C00,		// b			12
	 0x3900,		// C			13
	 0x7900,		// E			14
	 0x7100,		// F			15
	 0xB602,		// M			16
	 0x7300,		// P			17
	 0x0121,		// T			18
	 0x3E00,		// U			19
	 0x3012,		// V			20
	 0x801A,		// X			21
	 0x4021,		// +			22
	 0x4000,		// -			23
	 0x0080,		// .			24
	 0x3800,		// _L		25
	 0x0921,		// _I		26
	 0xb608, 		// _N		27
	 0x002b,		// K			28
};


void wr_1694byte (uchar wrreg) {
	uchar i;
	for ( i = 0; i < 8; i++)
	{
		P_disp_scl = 0;
		P_disp_data = (wrreg&0x01);
		wrreg >>= 1;
		P_disp_scl = 1;
	}
	P_disp_data = 1;
}

void refresh( void ) {
	uchar i;
	disp_ram[10] = disp_ram[1] >> 4;
	disp_ram[10] += (disp_ram[3] >> 2) & 0x0C;
	disp_ram[10] += disp_ram[5] & 0x30;
	disp_ram[10] += (disp_ram[7] << 2) & 0xC0;
	disp_ram[11] = disp_ram[9] >> 4;
	
	P_disp_stb = 0;
	wr_1694byte(CMD_DISPMOD);	//CMD_DISPMOD
	P_disp_stb = 1;
	_nop_();
	_nop_();
	P_disp_stb = 0;
	wr_1694byte(CMD_ADDRMOD);	//CMD_ADDRMOD
	P_disp_stb = 1;
	_nop_();
	_nop_();
	P_disp_stb = 0;
	wr_1694byte(CMD_ADDRRST);	//CMD_ADDRRST
	for( i = 0; i < 12; i ++)	{
		wr_1694byte(disp_ram[i]);
	}
	P_disp_stb = 1;
	_nop_();
	_nop_();
	P_disp_stb = 0;	
	wr_1694byte(disp_cmd);	//CMD_DISPNORM
	P_disp_stb = 1;	
}

code uchar tab_sound[] = {0x00,0x10,0x20,0x40};

void disp_num( uchar r_vol) {
	if(r_vol==100){
		((uint *)(disp_ram))[1] =tab_char[_1];
		((uint *)(disp_ram))[2] = tab_char[_0];
		((uint *)(disp_ram))[3] = tab_char[_0];
	}
	else{
		((uint *)(disp_ram))[1] = tab_char[_dark];
		((uint *)(disp_ram))[2] = tab_char[ r_vol /10 + 1];
		((uint *)(disp_ram))[3] = tab_char[r_vol %10 + 1];
	}
	disp_ram[8] = tab_sound[sound];
	if(Bvol != 6) disp_ram[8] |= 0x80;
}
code uchar SEG_TV[] = {_dark,_T,_V,_dark};
code uchar SEG_AUX[] = {_A,_U,_X,_dark};
code uchar SEG_MP3[] = {_M,_P,_3,_dark};
code uchar SEG_MUTE[] = { _M,_U,_T,_E};
code uchar SEG_OFF[] = { _dark,_0,_F,_F};
code uchar SEG_OK[] = { _dark,_0,_K,_dark};

code uchar *TAB_input[3] = {SEG_TV,SEG_AUX,SEG_MP3 };

void disp_char(uchar * p) {
	uchar i;
	for(i = 0; i < 4; i ++) {
		((uint *)(disp_ram))[i] = tab_char[*(p + i)];
	}
	disp_ram[8] = tab_sound[sound];
	if(Bvol != 6) disp_ram[8] |= 0x80;
}

code uchar TAB_CMD_DISP[] = {0x80, 0x88, 0x8A, 0x8D};

void disp_dispatch(struct state * me) {
	disp_cmd = TAB_CMD_DISP[dim];
	switch(me->disp) {
		case DISP_DARK:
			disp_cmd = CMD_DISPDARK;
			refresh();
			break;
		case DISP_INPUT:
			disp_char(TAB_input[input]);
			if(disp_time == 0) {
				if(f_mute == ON) {
					disp_char(SEG_MUTE);
					disp(DISP_MUTE, T_always);
				}
			}
			break;
		case DISP_MUTE:
			disp_char(SEG_MUTE);
			break;
		case DISP_MVOL:
			((uint *)(disp_ram))[0] = tab_char[_V];
			disp_num(Mvol);
			if(disp_time == 0) {
				if(f_mute == ON) {disp(DISP_MUTE, T_always);}
				else {disp(DISP_INPUT, T_always);}
			}
			break;
		case DISP_BVOL:
			((uint *)(disp_ram))[0] = tab_char[_b];
			if(Bvol < 6) {
				disp_num(6 - Bvol);
				((uint *)(disp_ram))[2] = tab_char[_minus];
			}
			else {
				disp_num(Bvol - 6);
				if(Bvol == 6) {
					((uint *)(disp_ram))[2] = tab_char[_dark];
				}
				else {
					((uint *)(disp_ram))[2] = tab_char[_plus];
				}
			}
			if(disp_time == 0) {
				if(f_mute == ON) {disp(DISP_MUTE, T_always);}
				else {disp(DISP_INPUT, T_always);}
			}
			break;
		case DISP_SLEEP:
			((uint *)(disp_ram))[0] = tab_char[_T];
			if(f_sleep == 0) {
				disp_char(SEG_OFF);
			}
			else {
				disp_num(t_sleep);
			}
			if(disp_time == 0) {
				if(f_mute == ON) {disp(DISP_MUTE, T_always);}
				else {disp(DISP_INPUT, T_always);}
			}
			break;
		case DISP_OK:
			disp_char(SEG_OK);
			if(disp_time == 0) disp(DISP_DARK, T_always);
			break;
	}
}


