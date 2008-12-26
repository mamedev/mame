#pragma once

#ifndef __NAMCO_H__
#define __NAMCO_H__

typedef struct _namco_interface namco_interface;
struct _namco_interface
{
	int voices;		/* number of voices */
	int stereo;		/* set to 1 to indicate stereo (e.g., System 1) */
};

WRITE8_HANDLER( pacman_sound_enable_w );
WRITE8_HANDLER( pacman_sound_w );

void polepos_sound_enable(int enable);
WRITE8_HANDLER( polepos_sound_w );

void mappy_sound_enable(int enable);
WRITE8_HANDLER( namco_15xx_w );

WRITE8_HANDLER( namcos1_cus30_w );	/* wavedata + sound registers + RAM */
READ8_HANDLER( namcos1_cus30_r );

WRITE8_HANDLER( _20pacgal_wavedata_w );

extern UINT8 *namco_soundregs;
extern UINT8 *namco_wavedata;

#define pacman_soundregs namco_soundregs
#define polepos_soundregs namco_soundregs

SND_GET_INFO( namco );
SND_GET_INFO( namco_15xx );
SND_GET_INFO( namco_cus30 );

#endif /* __NAMCO_H__ */

