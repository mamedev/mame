#pragma once

#ifndef __YMF271_H__
#define __YMF271_H__


typedef struct _ymf271_interface ymf271_interface;
struct _ymf271_interface
{
	devcb_read8 ext_read;		/* external memory read */
	devcb_write8 ext_write;	/* external memory write */
	void (*irq_callback)(const device_config *device, int state);	/* irq callback */
};

READ8_DEVICE_HANDLER( ymf271_r );
WRITE8_DEVICE_HANDLER( ymf271_w );

DEVICE_GET_INFO( ymf271 );
#define SOUND_YMF271 DEVICE_GET_INFO_NAME( ymf271 )

#endif /* __YMF271_H__ */
