// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Pool Shark Audio

***************************************************************************/
#ifndef MAME_ATARI_POOLSHRK_A_H
#define MAME_ATARI_POOLSHRK_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define POOLSHRK_BUMP_EN    NODE_01
#define POOLSHRK_CLICK_EN   NODE_02
#define POOLSHRK_SCORE_EN   NODE_03

// Nodes - Sounds
#define POOLSHRK_BUMP_SND       NODE_10
#define POOLSHRK_SCRATCH_SND    NODE_11
#define POOLSHRK_CLICK_SND      NODE_12
#define POOLSHRK_SCORE_SND      NODE_13


DISCRETE_SOUND_EXTERN( poolshrk_discrete );

#endif // MAME_ATARI_POOLSHRK_A_H
