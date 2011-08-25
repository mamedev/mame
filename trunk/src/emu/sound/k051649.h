#pragma once

#ifndef __K051649_H__
#define __K051649_H__

#include "devlegcy.h"

WRITE8_DEVICE_HANDLER( k051649_waveform_w );
READ8_DEVICE_HANDLER( k051649_waveform_r );
WRITE8_DEVICE_HANDLER( k051649_volume_w );
WRITE8_DEVICE_HANDLER( k051649_frequency_w );
WRITE8_DEVICE_HANDLER( k051649_keyonoff_w );

WRITE8_DEVICE_HANDLER( k052539_waveform_w );

DECLARE_LEGACY_SOUND_DEVICE(K051649, k051649);

#endif /* __K051649_H__ */
