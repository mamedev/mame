#ifndef _YMF271_H_
#define _YMF271_H_

struct YMF271interface
{
	read8_machine_func ext_read;		/* external memory read */
	write8_machine_func ext_write;	/* external memory write */
	void (*irq_callback)(running_machine *machine, int state);	/* irq callback */
};

READ8_HANDLER( YMF271_0_r );
WRITE8_HANDLER( YMF271_0_w );
READ8_HANDLER( YMF271_1_r );
WRITE8_HANDLER( YMF271_1_w );

#endif
