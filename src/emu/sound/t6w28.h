#pragma once

#ifndef __T6W28_H__
#define __T6W28_H__

WRITE8_DEVICE_HANDLER( t6w28_w );

DEVICE_GET_INFO( t6w28 );

#define SOUND_T6W28 DEVICE_GET_INFO_NAME( t6w28 )

#endif /* __T6W28_H__ */
