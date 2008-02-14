#ifndef _H_MSM5232_
#define _H_MSM5232_

struct MSM5232interface
{
	double	capacity[8];	/* in Farads, capacitors connected to pins: 24,25,26,27 and 37,38,39,40 */
};

WRITE8_HANDLER( MSM5232_0_w );
WRITE8_HANDLER( MSM5232_1_w );

#endif
