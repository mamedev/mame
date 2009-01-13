#pragma once

#ifndef __C352_H__
#define __C352_H__

READ16_HANDLER( c352_0_r );
WRITE16_HANDLER( c352_0_w );

SND_GET_INFO( c352 );
#define SOUND_C352 SND_GET_INFO_NAME( c352 )

#endif /* __C352_H__ */

