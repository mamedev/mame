#ifndef NAMCOIO_H
#define NAMCOIO_H

#include "devintrf.h"


enum
{
	NAMCOIO_56XX,
	NAMCOIO_58XX,
	NAMCOIO_59XX,
	NAMCOIO_62XX
};

#define MAX_NAMCOIO 8


struct namcoio_interface
{
	read8_space_func in[4];	/* read handlers for ports A-D */
	write8_space_func out[2];	/* write handlers for ports A-B */
};


READ8_HANDLER( namcoio_r );
WRITE8_HANDLER( namcoio_w );
void namcoio_init(running_machine *machine, int chipnum, int type, const struct namcoio_interface *intf, const char *device);
void namcoio_set_reset_line(int chipnum, int state);
void namcoio_set_irq_line(running_machine *machine, int chipnum, int state);


#endif
