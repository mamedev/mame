#pragma once

#ifndef __VOTRAX_H__
#define __VOTRAX_H__

#include "devlegcy.h"

WRITE8_DEVICE_HANDLER( votrax_w );
int votrax_status_r(device_t *device);

DECLARE_LEGACY_SOUND_DEVICE(VOTRAX, votrax);

#endif /* __VOTRAX_H__ */
