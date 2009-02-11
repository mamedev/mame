#pragma once

#ifndef __TIAINTF_H__
#define __TIAINTF_H__

WRITE8_DEVICE_HANDLER( tia_sound_w );

DEVICE_GET_INFO( tia );
#define SOUND_TIA DEVICE_GET_INFO_NAME( tia )

#endif /* __TIAINTF_H__ */
