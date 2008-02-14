#ifndef intf5220_h
#define intf5220_h

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

struct TMS5220interface
{
	void (*irq)(int state);		/* IRQ callback function */

	int (*read)(int count);			/* speech ROM read callback */
	void (*load_address)(int data);	/* speech ROM load address callback */
	void (*read_and_branch)(void);	/* speech ROM read and branch callback */
};

WRITE8_HANDLER( tms5220_data_w );
READ8_HANDLER( tms5220_status_r );
int tms5220_ready_r(void);
double tms5220_time_to_ready(void);
int tms5220_int_r(void);

void tms5220_set_frequency(int frequency);

enum
{
	SNDINFO_INT_TMS5220_VARIANT = SNDINFO_INT_CORE_SPECIFIC
};

#endif

