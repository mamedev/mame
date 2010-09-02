/**********************************************************************

    speaker.h
    Sound driver to emulate a simple speaker,
    driven by one or more output bits

**********************************************************************/

#pragma once

#ifndef __SPEAKER_H__
#define __SPEAKER_H__

#include "devlegcy.h"


typedef struct _speaker_interface speaker_interface;
struct _speaker_interface
{
	int num_level;	/* optional: number of levels (if not two) */
	const INT16 *levels;	/* optional: pointer to level lookup table */
};

void speaker_level_w (running_device *device, int new_level);

DECLARE_LEGACY_SOUND_DEVICE(SPEAKER_SOUND, speaker_sound);

#endif /* __SPEAKER_H__ */
