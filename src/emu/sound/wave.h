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


#define MDRV_SOUND_WAVE_ADD(_tag, _cass_tag) \
	MDRV_SOUND_ADD( _tag, WAVE, 0 ) \
	MDRV_DEVICE_CONFIG( _cass_tag )

#endif /* __WAVE_H__ */
