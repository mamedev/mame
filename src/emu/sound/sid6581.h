/***************************************************************************

    sid6581.h

    MAME/MESS interface for SID6581 and SID8580 chips

***************************************************************************/

#pragma once

#ifndef __SID6581_H__
#define __SID6581_H__


typedef enum
{
	MOS6581,
	MOS8580
} SIDTYPE;


typedef struct _sid6581_interface sid6581_interface;
struct _sid6581_interface
{
	int (*ad_read)(running_device *device, int channel);
} ;


READ8_DEVICE_HANDLER  ( sid6581_r );
WRITE8_DEVICE_HANDLER ( sid6581_w );

DEVICE_GET_INFO( sid6581 );
DEVICE_GET_INFO( sid8580 );
#define SOUND_SID6581 DEVICE_GET_INFO_NAME( sid6581 )
#define SOUND_SID8580 DEVICE_GET_INFO_NAME( sid8580 )

#endif /* __SID6581_H__ */
