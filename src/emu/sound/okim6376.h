#pragma once

#ifndef __OKIM6376_H__
#define __OKIM6376_H__

#include "devlegcy.h"

/* an interface for the OKIM6376 and similar chips */

READ8_DEVICE_HANDLER( okim6376_r );
WRITE8_DEVICE_HANDLER( okim6376_w );

DECLARE_LEGACY_SOUND_DEVICE(OKIM6376, okim6376);

#endif /* __OKIM6376_H__ */
