#pragma once

#ifndef __YMF271_H__
#define __YMF271_H__

typedef struct _ymf271_interface ymf271_interface;
struct _ymf271_interface
{
	read8_device_func ext_read;		/* external memory read */
	write8_device_func ext_write;	/* external memory write */
	void (*irq_callback)(running_machine *machine, int state);	/* irq callback */
};

READ8_HANDLER( ymf271_0_r );
WRITE8_HANDLER( ymf271_0_w );
READ8_HANDLER( ymf271_1_r );
WRITE8_HANDLER( ymf271_1_w );

SND_GET_INFO( ymf271 );
#define SOUND_YMF271 SND_GET_INFO_NAME( ymf271 )

#endif /* __YMF271_H__ */
