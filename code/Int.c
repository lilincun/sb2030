/*****************************************************
**																									**
**	File:					Version:															**
**		int.c				0.0.0.1																**
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

void int_ext0(void) interrupt 0 {
	;
}

void int_ext1(void) interrupt 2 {
	if(f_rem == INVALID) {
		f_rem = VALID;
	#ifdef f_12MHz
		TL0 = 0x06;             //0.25ms
	#else
		TL0 = 0xAD;             //0.25ms
	#endif
		TH0 = 0xFF;
		TR0 = 1;
		ET0 = 1;
	}
}

void sys_clear(void)
{
	static uchar idata d_time, temp;
	if((preset != temp) || (preset == 0))
	{
		if(preset == 1) d_time = T_2s;
		else d_time = T_1s;
		temp = preset;
	}
	else if(--d_time == 0)
	{
		preset = 0;
		temp = 0;
	}
}

void  int_timer1(void) interrupt 3 {		
	static uchar i;	
	static uint j;
	#ifdef f_12MHz
	if(++ i >= 80) {                 	///80
	#else
	if(++ i >= 33) {                 	///80
	#endif
		i = 0;
		f_T20ms = 1;
		if(time != 0) time --;
		if(disp_time != 0) disp_time --;
		if(key_time != 0) key_time --;
		if(mute_time != 0) mute_time --;
		sys_clear();
		if(t_sleep > 0) {
			if(++j > 3000) {
				j = 0;
				t_sleep --; 
			}
		}
		else j = 0;
	}
}

/* ============================================================================*/
/*   	Remote  time counter			 			               */
/*  	System (&=4MHz)				               	                       */
/*  	System mathine clock :  (12/&)S=3uS			       	       */
/*  	Timer1 overtime: 2e16 * 1/1uS= 65535*1 uS=65.535mS 	       */
/* ============================================================================*/

#define	IR_START	0x00	
#define	IR_LEAD_L	0x01	
#define	IR_LEAD_H	0x02	
#define	IR_CODE	0x03	
#define	IR_END   	0x04

/*=============================================================================*/
/*   Remote   lead  dector				         	                               */
/*						                 	                                               */
/*    System (&=4MHz)				 			                               */
/*    System mathine clock :(12&)S=3uS	 				               */
/*  Lead1 : 9mS+2.25mS=11.25mS    Lead2 : 9mS+4.5mS=13.5mS              */
/*             :     9<11.25<12.375        : 12.375<13.5<16	 	                       */
/*             :0x2328<0x2BF2<0x3057       : 0x3057<0x34BC<0x3E80	       */
/*=============================================================================*/

/*******************************************************************************/
/*	Remote  check Processor				       	                  */
/*******************************************************************************/
#define	IR_WIDTH_HEAD9_MIN	6800/250	/*HEADER PULSE 8msec:*/
						/*6.5msec - 9.5msec*/
#define IR_WIDTH_HEAD9_MAX	11800/250	
#define IR_WIDTH_HEAD4_5_MIN	3000/250	/*HEADER PULSE 12msec:*/
						/*10msec - 14msec*/
#define IR_WIDTH_HEAD4_5_MAX	5000/250	
#define IR_WIDTH_HEAD2_25_MIN	1800/250	/*HEADER PULSE 12msec:*/
						/*10msec - 14msec*/
#define IR_WIDTH_HEAD2_25_MAX	3000/250

#define IR_WIDTH_DATA0_MIN	500/250		/*DATA [0]: 0.5msec - 1.5msec*/
#define IR_WIDTH_DATA0_MAX	1800/250	
#define IR_WIDTH_DATA1_MIN	1800/250	/*DATA [1]: 1.5msec - 2.5msec*/
#define IR_WIDTH_DATA1_MAX	2500/250	
#define IR_WIDTH_OFF		108000/250	/* 108000/250 //REMO-CON OFF: 60msec*/
#define IR_WIDTH_END		4000/250	/*REMO-CON DATA: 4.0msec*/

#define	IR_USERCODE_H	0xF5
#define	IR_USERCODE_L	0x08

void  int_timer0(void) interrupt 1
{
	static bit f_IRreceive;
	static uchar r_IRwidthtime;
	static uchar r_IRphase;
	static uint r_IRofftime;
	static uchar r_count;
	static ulong r_IR;

	TR0 = 0;
	r_IRwidthtime ++;
	switch(r_IRphase)
	{
		case IR_START:	
			if (P_remote == 0)
			{
				r_IRwidthtime = 0;
				r_IRphase = IR_LEAD_L;
			}
			else
			{
				if (r_IRofftime != 0) r_IRofftime --;
				else
				{
					Rem_data = 0x00;
		   		
					if(f_IRreceive == ON)
					{
						r_IR = 0;
						r_count = 0;
						f_IRreceive = OFF;
					}
					
					ET0 = 0;	
					f_rem = INVALID;	
				}
			}
			break;
		case IR_LEAD_L:	
			if(r_IRwidthtime > IR_WIDTH_HEAD9_MAX) r_IRphase = IR_START;
			else if(P_remote == 1)
			{
				if(r_IRwidthtime < IR_WIDTH_HEAD9_MIN) r_IRphase = IR_START;
				else
				{
					r_IRwidthtime = 0x00;	// check HEADER PULSE 12msec
					r_IRphase = IR_LEAD_H;
				}
			}
			break;
		case IR_LEAD_H:	
			if(r_IRwidthtime > IR_WIDTH_HEAD4_5_MAX) r_IRphase = IR_START;
			else if(P_remote == 0)
			{
				if (r_IRwidthtime > IR_WIDTH_HEAD4_5_MIN)
				{
					r_IRphase = IR_CODE;
					r_IRwidthtime = 0x00;
					r_IR = 0;
					r_count = 0;
					Rem_data = 0x00;
				}
				else
				{
					r_IRphase = IR_START;
					if((r_IRwidthtime >= IR_WIDTH_HEAD2_25_MIN) 
						&& (f_IRreceive == ON))
					{
						r_IRofftime = IR_WIDTH_OFF;
					}
				}
			}
			break;
		case IR_CODE:	
			if(r_IRwidthtime > IR_WIDTH_DATA1_MAX) r_IRphase = IR_START;
			else if(P_remote == 0)
			{
				if (r_IRwidthtime > IR_WIDTH_DATA0_MIN)
				{
					if (r_IRwidthtime >= IR_WIDTH_DATA1_MIN) ((uchar *)(&r_IR))[3] ++;  
					r_IR = _lror_(r_IR,1);
					if(++r_count >= 32) r_IRphase = IR_END;
					r_IRwidthtime = 0x00;
				}
			}		
			break;
		case IR_END:	
			if (r_IRwidthtime > IR_WIDTH_END) 
			{
				if(((((uchar *)(&r_IR))[3] == IR_USERCODE_L)
					&& (((uchar *)(&r_IR))[2] == IR_USERCODE_H))
					&& (((uchar *)(&r_IR))[1] == ~((uchar *)(&r_IR))[0]))
			   	{
	    		   	Rem_data = ((uchar *)(&r_IR))[1];
					r_IRofftime = IR_WIDTH_OFF;
					f_IRreceive = ON;
			   	}
				r_IRphase = IR_START;
			}
			break;
		default:	
			r_IRphase = IR_START;
			break;
	}	
	#ifdef f_12MHz
	TL0 = 0x06;
	#else
	TL0 = 0xAD;
	#endif
	TH0 = 0xFF;
	TR0 = 1;
}


