#ifndef _NILE_H_
#define _NILE_H_

struct NiLe_interface
{
	int region;
};

extern UINT16 *nile_sound_regs;

WRITE16_HANDLER(nile_snd_w);
READ16_HANDLER(nile_snd_r);
WRITE16_HANDLER(nile_sndctrl_w);
READ16_HANDLER(nile_sndctrl_r);

#endif

