#pragma once

#ifndef __ICS2115_H__
#define __ICS2115_H__

typedef struct _ics2115_interface ics2115_interface;
struct _ics2115_interface {
	void (*irq_cb)(running_device *, int);
};

READ8_DEVICE_HANDLER( ics2115_r );
WRITE8_DEVICE_HANDLER( ics2115_w );

DEVICE_GET_INFO( ics2115 );
#define SOUND_ICS2115 DEVICE_GET_INFO_NAME( ics2115 )

#endif /* __ICS2115_H__ */
