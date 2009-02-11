#pragma once

#ifndef __WAVE_H__
#define __WAVE_H__

/*****************************************************************************
 *  CassetteWave interface
 *****************************************************************************/

#ifdef MESS
#include "messdrv.h"
#endif

DEVICE_GET_INFO( wave );
#define SOUND_WAVE DEVICE_GET_INFO_NAME( wave )

#endif /* __WAVE_H__ */
