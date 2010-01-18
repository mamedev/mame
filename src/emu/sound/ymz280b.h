/**********************************************************************************************
 *
 *   Yamaha YMZ280B driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __YMZ280B_H__
#define __YMZ280B_H__


typedef struct _ymz280b_interface ymz280b_interface;
struct _ymz280b_interface
{
	void (*irq_callback)(running_device *device, int state);	/* irq callback */
	devcb_read8 ext_read;			/* external RAM read */
	devcb_write8 ext_write;		/* external RAM write */
};

READ8_DEVICE_HANDLER ( ymz280b_r );
WRITE8_DEVICE_HANDLER( ymz280b_w );

DEVICE_GET_INFO( ymz280b );
#define SOUND_YMZ280B DEVICE_GET_INFO_NAME( ymz280b )

#endif /* __YMZ280B_H__ */
