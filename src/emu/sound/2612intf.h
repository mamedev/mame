#pragma once

#ifndef __2612INTF_H__
#define __2612INTF_H__

void ym2612_update_request(void *param);

typedef struct _ym2612_interface ym2612_interface;
struct _ym2612_interface
{
	void (*handler)(running_device *device, int irq);
};

READ8_DEVICE_HANDLER( ym2612_r );
WRITE8_DEVICE_HANDLER( ym2612_w );

READ8_DEVICE_HANDLER( ym2612_status_port_a_r );
READ8_DEVICE_HANDLER( ym2612_status_port_b_r );
READ8_DEVICE_HANDLER( ym2612_data_port_a_r );
READ8_DEVICE_HANDLER( ym2612_data_port_b_r );

WRITE8_DEVICE_HANDLER( ym2612_control_port_a_w );
WRITE8_DEVICE_HANDLER( ym2612_control_port_b_w );
WRITE8_DEVICE_HANDLER( ym2612_data_port_a_w );
WRITE8_DEVICE_HANDLER( ym2612_data_port_b_w );


DEVICE_GET_INFO( ym2612 );
#define SOUND_YM2612 DEVICE_GET_INFO_NAME( ym2612 )


typedef struct _ym3438_interface ym3438_interface;
struct _ym3438_interface
{
	void (*handler)(running_device *device, int irq);
};


#define ym3438_r				ym2612_r
#define ym3438_w				ym2612_w

#define ym3438_status_port_a_r	ym2612_status_port_a_r
#define ym3438_status_port_b_r	ym2612_status_port_b_r
#define ym3438_data_port_a_r	ym2612_data_port_a_r
#define ym3438_data_port_b_r	ym2612_data_port_b_r

#define ym3438_control_port_a_w	ym2612_control_port_a_w
#define ym3438_control_port_b_w	ym2612_control_port_b_w
#define ym3438_data_port_a_w	ym2612_data_port_a_w
#define ym3438_data_port_b_w	ym2612_data_port_b_w


DEVICE_GET_INFO( ym3438 );
#define SOUND_YM3438 DEVICE_GET_INFO_NAME( ym3438 )

#endif /* __2612INTF_H__ */
