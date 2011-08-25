/*********************************************************

    Irem GA20 PCM Sound Chip

*********************************************************/
#pragma once

#ifndef __IREMGA20_H__
#define __IREMGA20_H__

#include "devlegcy.h"

WRITE8_DEVICE_HANDLER( irem_ga20_w );
READ8_DEVICE_HANDLER( irem_ga20_r );

DECLARE_LEGACY_SOUND_DEVICE(IREMGA20, iremga20);

#endif /* __IREMGA20_H__ */
