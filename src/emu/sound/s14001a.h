#pragma once

#ifndef __S14001A_H__
#define __S14001A_H__

int s14001a_bsy_r(const device_config *device);     		/* read BUSY pin */
void s14001a_reg_w(const device_config *device, int data);		/* write to input latch */
void s14001a_rst_w(const device_config *device, int data);		/* write to RESET pin */
void s14001a_set_clock(const device_config *device, int clock);     /* set VSU-1000 clock */
void s14001a_set_volume(const device_config *device, int volume);    /* set VSU-1000 volume control */

DEVICE_GET_INFO( s14001a );
#define SOUND_S14001A DEVICE_GET_INFO_NAME( s14001a )

#endif /* __S14001A_H__ */

