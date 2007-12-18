#ifndef m58817_h
#define m58817_h

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

#define M58817_MAX_SAMPLES 20

struct M58817interface
{
	int rom_region;							/* set to -1 to use samples */
	int sample_addr[M58817_MAX_SAMPLES];
};

WRITE8_HANDLER( m58817_CTL_w );
WRITE8_HANDLER( m58817_DRQ_w );

READ8_HANDLER( m58817_status_r );

void m58817_set_frequency(int frequency);

#endif

