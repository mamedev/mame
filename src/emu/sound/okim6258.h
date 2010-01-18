#pragma once

#ifndef __OKIM6258_H__
#define __OKIM6258_H__

/* an interface for the OKIM6258 and similar chips */

typedef struct _okim6258_interface okim6258_interface;
struct _okim6258_interface
{
	int divider;
	int adpcm_type;
	int output_12bits;
};


#define FOSC_DIV_BY_1024	0
#define FOSC_DIV_BY_768		1
#define FOSC_DIV_BY_512		2

#define TYPE_3BITS      	0
#define TYPE_4BITS			1

#define	OUTPUT_10BITS		0
#define	OUTPUT_12BITS		1

void okim6258_set_divider(running_device *device, int val);
void okim6258_set_clock(running_device *device, int val);
int okim6258_get_vclk(running_device *device);

READ8_DEVICE_HANDLER( okim6258_status_r );
WRITE8_DEVICE_HANDLER( okim6258_data_w );
WRITE8_DEVICE_HANDLER( okim6258_ctrl_w );

DEVICE_GET_INFO( okim6258 );
#define SOUND_OKIM6258 DEVICE_GET_INFO_NAME( okim6258 )

#endif /* __OKIM6258_H__ */
