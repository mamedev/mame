#pragma once

#ifndef __WAVE_H__
#define __WAVE_H__

#include "devlegcy.h"

/*****************************************************************************
 *  CassetteWave interface
 *****************************************************************************/

DECLARE_LEGACY_SOUND_DEVICE(WAVE, wave);


#define MCFG_SOUND_WAVE_ADD(_tag, _cass_tag) \
	MCFG_SOUND_ADD( _tag, WAVE, 0 ) \
	MCFG_DEVICE_CONFIG( _cass_tag )

#endif /* __WAVE_H__ */
