/*****************************************************
**																									**
**	File:					Version:															**
**		njw1180a.c				0.0.0.1													**
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

#include<sb2010.h>

void I_start(void) {
	P_I2C_scl= HIGH;
	NOP();
	P_I2C_sda = LOW;
	NOP();
	P_I2C_scl = LOW;
	NOP(); 
}

void I_stop(void) {
	P_I2C_sda = LOW;
	NOP();
	P_I2C_scl = HIGH;
	NOP();
	P_I2C_sda = HIGH;
	NOP();  
}

bit I_clk(void) {
	bit sample;
	
	NOP();
	P_I2C_scl = HIGH;
	NOP();
	sample = P_I2C_sda;
	NOP();
	P_I2C_scl = LOW;
	NOP();
	return(sample);
}

bit I_send(uchar I_data) {
	uchar i;
	for(i = 0; i < 8; i ++) {
		P_I2C_sda = (bit)(I_data & 0x80);
		I_data <<= 1;
		I_clk();
	}
	P_I2C_sda = HIGH;
	return(!I_clk());
}

uchar wr_I2C(uchar Dev_addr,uchar addr,uchar dat) {
	I_start();
	if(I_send(Dev_addr)) {
		if(I_send(addr)) {
			if(I_send(dat)) {
				I_stop();
				return(TRUE);
			}
		}
	}
	else {
		I_stop();
		return(FALSE);
	}
}

