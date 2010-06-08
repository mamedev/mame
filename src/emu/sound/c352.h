#pragma once

#ifndef __C352_H__
#define __C352_H__

#include  "devlegcy.h"

READ16_DEVICE_HANDLER( c352_r );
WRITE16_DEVICE_HANDLER( c352_w );

DECLARE_LEGACY_SOUND_DEVICE(C352, c352);

#endif /* __C352_H__ */

