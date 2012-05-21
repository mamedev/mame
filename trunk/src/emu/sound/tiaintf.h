#pragma once

#ifndef __TIAINTF_H__
#define __TIAINTF_H__

#include "devlegcy.h"

WRITE8_DEVICE_HANDLER( tia_sound_w );

DECLARE_LEGACY_SOUND_DEVICE(TIA, tia);

#endif /* __TIAINTF_H__ */
