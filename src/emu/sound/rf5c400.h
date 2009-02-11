/* Ricoh RF5C400 emulator */

#pragma once

#ifndef __RF5C400_H__
#define __RF5C400_H__

READ16_DEVICE_HANDLER( rf5c400_r );
WRITE16_DEVICE_HANDLER( rf5c400_w );

DEVICE_GET_INFO( rf5c400 );
#define SOUND_RF5C400 DEVICE_GET_INFO_NAME( rf5c400 )

#endif /* __RF5C400_H__ */
