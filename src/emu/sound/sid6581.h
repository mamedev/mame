/***************************************************************************

    sid6581.h

    MAME/MESS interface for SID6581 and SID8580 chips

***************************************************************************/

#pragma once

#ifndef __SID6581_H__
#define __SID6581_H__

#include "devlegcy.h"


typedef enum
{
	MOS6581,
	MOS8580
} SIDTYPE;

#define MOS6581_INTERFACE(name) \
	const sid6581_interface (name) =

typedef struct _sid6581_interface sid6581_interface;
struct _sid6581_interface
{
	devcb_read8 in_potx_cb;
	devcb_read8 in_poty_cb;
};


READ8_DEVICE_HANDLER  ( sid6581_r );
WRITE8_DEVICE_HANDLER ( sid6581_w );

DECLARE_LEGACY_SOUND_DEVICE(SID6581, sid6581);
DECLARE_LEGACY_SOUND_DEVICE(SID8580, sid8580);

#endif /* __SID6581_H__ */
