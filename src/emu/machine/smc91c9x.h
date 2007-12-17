/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles

**************************************************************************/

#ifndef __SMC91C9X__
#define __SMC91C9X__

struct smc91c9x_interface
{
	void (*irq_handler)(int state);
};

void smc91c94_init(struct smc91c9x_interface *config);
void smc91c94_reset(void);
READ16_HANDLER( smc91c94_r );
WRITE16_HANDLER( smc91c94_w );

#endif
