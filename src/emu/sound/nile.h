#pragma once

#ifndef __NILE_H__
#define __NILE_H__

#include "devlegcy.h"

extern UINT16 *nile_sound_regs;

WRITE16_DEVICE_HANDLER( nile_snd_w );
READ16_DEVICE_HANDLER( nile_snd_r );
WRITE16_DEVICE_HANDLER( nile_sndctrl_w );
READ16_DEVICE_HANDLER( nile_sndctrl_r );

DECLARE_LEGACY_SOUND_DEVICE(NILE, nile);

#endif /* __NILE_H__ */
