#pragma once

#ifndef __WAVE_H__
#define __WAVE_H__

#include "devlegcy.h"

/*****************************************************************************
 *  CassetteWave interface
 *****************************************************************************/

#ifdef MESS
#include "messdrv.h"
#endif

DECLARE_LEGACY_SOUND_DEVICE(WAVE, wave);


#define MDRV_SOUND_WAVE_ADD(_tag, _cass_tag) \
	MDRV_SOUND_ADD( _tag, WAVE, 0 ) \
	MDRV_DEVICE_CONFIG( _cass_tag )

#endif /* __WAVE_H__ */
