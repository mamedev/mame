#ifndef _ST0016_H_
#define _ST0016_H_

struct ST0016interface
{
	UINT8 **p_soundram;
};

extern UINT8 *st0016_sound_regs;

WRITE8_HANDLER(st0016_snd_w);

#endif

