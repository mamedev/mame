#pragma once

#ifndef __NILE_H__
#define __NILE_H__

extern UINT16 *nile_sound_regs;

WRITE16_DEVICE_HANDLER( nile_snd_w );
READ16_DEVICE_HANDLER( nile_snd_r );
WRITE16_DEVICE_HANDLER( nile_sndctrl_w );
READ16_DEVICE_HANDLER( nile_sndctrl_r );

DEVICE_GET_INFO( nile );
#define SOUND_NILE DEVICE_GET_INFO_NAME( nile )

#endif /* __NILE_H__ */
