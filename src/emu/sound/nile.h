#pragma once

#ifndef __NILE_H__
#define __NILE_H__

extern UINT16 *nile_sound_regs;

WRITE16_HANDLER(nile_snd_w);
READ16_HANDLER(nile_snd_r);
WRITE16_HANDLER(nile_sndctrl_w);
READ16_HANDLER(nile_sndctrl_r);

SND_GET_INFO( nile );

#endif /* __NILE_H__ */
