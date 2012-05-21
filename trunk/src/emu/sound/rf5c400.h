/* Ricoh RF5C400 emulator */

#pragma once

#ifndef __RF5C400_H__
#define __RF5C400_H__

#include "devlegcy.h"

READ16_DEVICE_HANDLER( rf5c400_r );
WRITE16_DEVICE_HANDLER( rf5c400_w );

DECLARE_LEGACY_SOUND_DEVICE(RF5C400, rf5c400);

#endif /* __RF5C400_H__ */
