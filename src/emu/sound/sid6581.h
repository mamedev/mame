/***************************************************************************

    sid6581.h

    MAME/MESS interface for SID6581 and SID8580 chips

***************************************************************************/

#ifndef SID6581_H
#define SID6581_H

#include "sndintrf.h"

typedef enum
{
	MOS6581,
	MOS8580
} SIDTYPE;


typedef struct
{
	int (*ad_read)(int channel);
} SID6581_interface;


READ8_HANDLER  ( sid6581_0_port_r );
READ8_HANDLER  ( sid6581_1_port_r );
WRITE8_HANDLER ( sid6581_0_port_w );
WRITE8_HANDLER ( sid6581_1_port_w );

#endif /* SID6581_H */
