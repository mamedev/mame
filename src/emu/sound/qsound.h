/*********************************************************

    Capcom Q-Sound system

*********************************************************/

#pragma once

#ifndef __QSOUND_H__
#define __QSOUND_H__

#include "devlegcy.h"

#define QSOUND_CLOCK    4000000   /* default 4MHz clock */

WRITE8_DEVICE_HANDLER( qsound_w );
READ8_DEVICE_HANDLER( qsound_r );

DECLARE_LEGACY_SOUND_DEVICE(QSOUND, qsound);

#endif /* __QSOUND_H__ */
