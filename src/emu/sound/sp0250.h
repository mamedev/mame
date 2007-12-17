#ifndef __SP0250_H__
#define __SP0250_H__

struct sp0250_interface {
	void (*drq_callback)(int state);
};

WRITE8_HANDLER( sp0250_w );

#endif
