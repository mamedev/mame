/**********************************************************************************************
 *
 *   Yamaha YMZ280B driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __YMZ280B_H__
#define __YMZ280B_H__

#include "devlegcy.h"


typedef struct _ymz280b_interface ymz280b_interface;
struct _ymz280b_interface
{
	void (*irq_callback)(device_t *device, int state);	/* irq callback */
	devcb_read8 ext_read;			/* external RAM read */
	devcb_write8 ext_write;		/* external RAM write */
};

READ8_DEVICE_HANDLER ( ymz280b_r );
WRITE8_DEVICE_HANDLER( ymz280b_w );

DECLARE_LEGACY_SOUND_DEVICE(YMZ280B, ymz280b);

#endif /* __YMZ280B_H__ */
