#pragma once

#ifndef __S14001A_H__
#define __S14001A_H__

int s14001a_bsy_0_r(void);     		/* read BUSY pin */
void s14001a_reg_0_w(int data);		/* write to input latch */
void s14001a_rst_0_w(int data);		/* write to RESET pin */
void s14001a_set_clock(int clock);     /* set VSU-1000 clock */
void s14001a_set_volume(int volume);    /* set VSU-1000 volume control */

SND_GET_INFO( s14001a );

#endif /* __S14001A_H__ */

