#ifndef _YMF271_H_
#define _YMF271_H_

struct YMF271interface
{
	int region;			/* memory region of sample ROMs */
	read8_handler ext_read;		/* external memory read */
	write8_handler ext_write;	/* external memory write */
	void (*irq_callback)(int state);	/* irq callback */
};

READ8_HANDLER( YMF271_0_r );
WRITE8_HANDLER( YMF271_0_w );
READ8_HANDLER( YMF271_1_r );
WRITE8_HANDLER( YMF271_1_w );

#endif
