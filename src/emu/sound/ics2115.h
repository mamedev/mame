#ifndef __ICS2115_H__
#define __ICS2115_H__

typedef struct _ics2115_interface ics2115_interface;
struct _ics2115_interface {
	void (*irq_cb)(running_machine *, int);
};

READ8_HANDLER( ics2115_r );
WRITE8_HANDLER( ics2115_w );

#endif
