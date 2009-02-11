#pragma once

#ifndef __C352_H__
#define __C352_H__

READ16_DEVICE_HANDLER( c352_r );
WRITE16_DEVICE_HANDLER( c352_w );

DEVICE_GET_INFO( c352 );
#define SOUND_C352 DEVICE_GET_INFO_NAME( c352 )

#endif /* __C352_H__ */

