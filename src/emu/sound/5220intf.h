#pragma once

#ifndef __5200INTF_H__
#define __5200INTF_H__

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

typedef struct _tms5220_interface tms5220_interface;
struct _tms5220_interface
{
	void (*irq)(running_machine *machine, int state);		/* IRQ callback function */

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

SND_GET_INFO( tms5220 );
SND_GET_INFO( tmc0285 );
SND_GET_INFO( tms5200 );

#define SOUND_TMS5220 SND_GET_INFO_NAME( tms5220 )
#define SOUND_TMC0285 SND_GET_INFO_NAME( tmc0285 )
#define SOUND_TMS5200 SND_GET_INFO_NAME( tms5200 )

#endif /* __5200INTF_H__ */
