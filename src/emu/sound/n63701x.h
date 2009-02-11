#pragma once

#ifndef __N63701X_H__
#define __N63701X_H__

WRITE8_DEVICE_HANDLER( namco_63701x_w );

DEVICE_GET_INFO( namco_63701x );
#define SOUND_NAMCO_63701X DEVICE_GET_INFO_NAME( namco_63701x )

#endif /* __N63701X_H__ */
