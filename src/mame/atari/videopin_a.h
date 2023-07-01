// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Video Pinball Audio

***************************************************************************/
#ifndef MAME_ATARI_VIDEOPIN_A_H
#define MAME_ATARI_VIDEOPIN_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define VIDEOPIN_OCTAVE_DATA    NODE_01
#define VIDEOPIN_NOTE_DATA      NODE_02
#define VIDEOPIN_BELL_EN        NODE_03
#define VIDEOPIN_BONG_EN        NODE_04
#define VIDEOPIN_ATTRACT_EN     NODE_05
#define VIDEOPIN_VOL_DATA       NODE_06


DISCRETE_SOUND_EXTERN( videopin_discrete );

#endif // MAME_ATARI_VIDEOPIN_A_H
