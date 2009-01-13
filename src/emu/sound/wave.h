#pragma once

#ifndef __WAVE_H__
#define __WAVE_H__

/*****************************************************************************
 *  CassetteWave interface
 *****************************************************************************/

#ifdef MESS
#include "messdrv.h"
#endif

SND_GET_INFO( wave );
#define SOUND_WAVE SND_GET_INFO_NAME( wave )

#endif /* __WAVE_H__ */
