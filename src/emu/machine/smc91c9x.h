/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles

**************************************************************************/

#ifndef __SMC91C9X__
#define __SMC91C9X__

struct smc91c9x_interface
{
	void (*irq_handler)(running_machine *machine, int state);
};

void smc91c94_init(const struct smc91c9x_interface *config);
void smc91c94_reset(running_machine *machine);
READ16_HANDLER( smc91c94_r );
WRITE16_HANDLER( smc91c94_w );

#endif
