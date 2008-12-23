/***************************************************************************

    sid6581.h

    MAME/MESS interface for SID6581 and SID8580 chips

***************************************************************************/

#pragma once

#ifndef __SID6581_H__
#define __SID6581_H__

#include "sndintrf.h"

typedef enum
{
	MOS6581,
	MOS8580
} SIDTYPE;


typedef struct _sid6581_interface sid6581_interface;
struct _sid6581_interface
{
	int (*ad_read)(const device_config *device, int channel);
} ;


READ8_HANDLER  ( sid6581_0_port_r );
READ8_HANDLER  ( sid6581_1_port_r );
WRITE8_HANDLER ( sid6581_0_port_w );
WRITE8_HANDLER ( sid6581_1_port_w );

#endif /* __SID6581_H__ */
