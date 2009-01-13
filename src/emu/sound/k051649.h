#pragma once

#ifndef __K051649_H__
#define __K051649_H__

WRITE8_HANDLER( k051649_waveform_w );
READ8_HANDLER( k051649_waveform_r );
WRITE8_HANDLER( k051649_volume_w );
WRITE8_HANDLER( k051649_frequency_w );
WRITE8_HANDLER( k051649_keyonoff_w );
WRITE8_HANDLER( k052539_waveform_w );

SND_GET_INFO( k051649 );
#define SOUND_K051649 SND_GET_INFO_NAME( k051649 )

#endif /* __K051649_H__ */
