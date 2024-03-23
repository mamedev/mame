// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Sky Diver Audio

***************************************************************************/
#ifndef MAME_ATARI_SKYDIVER_A_H
#define MAME_ATARI_SKYDIVER_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define SKYDIVER_RANGE_DATA     NODE_01
#define SKYDIVER_NOTE_DATA      NODE_02
#define SKYDIVER_RANGE3_EN      NODE_03
#define SKYDIVER_NOISE_DATA     NODE_04
#define SKYDIVER_NOISE_RST      NODE_05
#define SKYDIVER_WHISTLE1_EN    NODE_06
#define SKYDIVER_WHISTLE2_EN    NODE_07
#define SKYDIVER_OCT1_EN        NODE_08
#define SKYDIVER_OCT2_EN        NODE_09
#define SKYDIVER_SOUND_EN       NODE_10


DISCRETE_SOUND_EXTERN( skydiver_discrete );

#endif // MAME_ATARI_SKYDIVER_A_H
