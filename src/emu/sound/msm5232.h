#ifndef _H_MSM5232_
#define _H_MSM5232_

struct MSM5232interface
{
	double capacity[8];	/* in Farads, capacitors connected to pins: 24,25,26,27 and 37,38,39,40 */
	void (*gate_handler)(int state);	/* callback called when the GATE output pin changes state */
};

WRITE8_HANDLER( MSM5232_0_w );
WRITE8_HANDLER( MSM5232_1_w );

void msm5232_set_clock(void *chip, int clock);

#endif
