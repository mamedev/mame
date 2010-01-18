#pragma once

#ifndef __MULTIPCM_H__
#define __MULTIPCM_H__

WRITE8_DEVICE_HANDLER( multipcm_w );
READ8_DEVICE_HANDLER( multipcm_r );

void multipcm_set_bank(running_device *device, UINT32 leftoffs, UINT32 rightoffs);

DEVICE_GET_INFO( multipcm );
#define SOUND_MULTIPCM DEVICE_GET_INFO_NAME( multipcm )

#endif /* __MULTIPCM_H__ */
