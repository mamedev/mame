#ifndef _S14001A_H_
#define _S14001A_H_

struct S14001A_interface
{
	int region;			/* memory region where the sample ROM lives */
};

int S14001A_bsy_0_r(void);     		/* read BUSY pin */
void S14001A_reg_0_w(int data);		/* write to input latch */
void S14001A_rst_0_w(int data);		/* write to RESET pin */
void S14001A_set_clock(int clock);     /* set VSU-1000 clock */
void S14001A_set_volume(int volume);    /* set VSU-1000 volume control */

#endif

