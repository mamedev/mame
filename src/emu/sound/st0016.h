#pragma once

#ifndef __ST0016_H__
#define __ST0016_H__

typedef struct _st0016_interface st0016_interface;
struct _st0016_interface
{
	UINT8 **p_soundram;
};

READ8_DEVICE_HANDLER( st0016_snd_r );
WRITE8_DEVICE_HANDLER( st0016_snd_w );

DEVICE_GET_INFO( st0016 );
#define SOUND_ST0016 DEVICE_GET_INFO_NAME( st0016 )

#endif /* __ST0016_H__ */
