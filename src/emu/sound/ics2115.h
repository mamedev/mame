#pragma once

#ifndef __ICS2115_H__
#define __ICS2115_H__

typedef struct _ics2115_interface ics2115_interface;
struct _ics2115_interface {
	void (*irq_cb)(running_machine *, int);
};

READ8_HANDLER( ics2115_r );
WRITE8_HANDLER( ics2115_w );

SND_GET_INFO( ics2115 );
#define SOUND_ICS2115 SND_GET_INFO_NAME( ics2115 )

#endif /* __ICS2115_H__ */
