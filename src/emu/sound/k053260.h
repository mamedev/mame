/*********************************************************

    Konami 053260 PCM/ADPCM Sound Chip

*********************************************************/
#ifndef __K053260_H__
#define __K053260_H__

struct K053260_interface {
	int region;					/* memory region of sample ROM(s) */
	void (*irq)(running_machine *machine, int param );	/* called on SH1 complete cycle ( clock / 32 ) */
};


WRITE8_HANDLER( K053260_0_w );
WRITE8_HANDLER( K053260_1_w );
READ8_HANDLER( K053260_0_r );
READ8_HANDLER( K053260_1_r );
WRITE16_HANDLER( K053260_0_lsb_w );
READ16_HANDLER( K053260_0_lsb_r );
WRITE16_HANDLER( K053260_1_lsb_w );
READ16_HANDLER( K053260_1_lsb_r );

#endif /* __K053260_H__ */
