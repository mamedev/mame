#pragma once

#ifndef __ST0016_H__
#define __ST0016_H__

typedef struct _st0016_interface st0016_interface;
struct _st0016_interface
{
	UINT8 **p_soundram;
};

extern UINT8 *st0016_sound_regs;

WRITE8_HANDLER(st0016_snd_w);

SND_GET_INFO( st0016 );

#endif /* __ST0016_H__ */
