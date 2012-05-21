#pragma once

#ifndef __YMF271_H__
#define __YMF271_H__

#include "devlegcy.h"


typedef struct _ymf271_interface ymf271_interface;
struct _ymf271_interface
{
	devcb_read8 ext_read;		/* external memory read */
	devcb_write8 ext_write;	/* external memory write */
	void (*irq_callback)(device_t *device, int state);	/* irq callback */
};

READ8_DEVICE_HANDLER( ymf271_r );
WRITE8_DEVICE_HANDLER( ymf271_w );

DECLARE_LEGACY_SOUND_DEVICE(YMF271, ymf271);

#endif /* __YMF271_H__ */
