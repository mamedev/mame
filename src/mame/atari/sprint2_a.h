// license:BSD-3-Clause
// copyright-holders:Hans Andersson

/***************************************************************************

Sprint 2 / Sprint 1 / Dominos Audio

***************************************************************************/
#ifndef MAME_ATARI_SPRINT2_A_H
#define MAME_ATARI_SPRINT2_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define SPRINT2_SKIDSND1_EN        NODE_01
#define SPRINT2_SKIDSND2_EN        NODE_02
#define SPRINT2_MOTORSND1_DATA     NODE_03
#define SPRINT2_MOTORSND2_DATA     NODE_04
#define SPRINT2_CRASHSND_DATA      NODE_05
#define SPRINT2_ATTRACT_EN         NODE_06
#define SPRINT2_NOISE_RESET        NODE_07

#define DOMINOS_FREQ_DATA          SPRINT2_MOTORSND1_DATA
#define DOMINOS_AMP_DATA           SPRINT2_CRASHSND_DATA
#define DOMINOS_TUMBLE_EN          SPRINT2_SKIDSND1_EN
#define DOMINOS_ATTRACT_EN         SPRINT2_ATTRACT_EN


DISCRETE_SOUND_EXTERN( sprint2_discrete );
DISCRETE_SOUND_EXTERN( sprint1_discrete );
DISCRETE_SOUND_EXTERN( dominos_discrete );

#endif // MAME_ATARI_SPRINT2_A_H
