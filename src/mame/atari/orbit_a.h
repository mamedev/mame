// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Orbit Audio

***************************************************************************/
#ifndef MAME_ATARI_ORBIT_A_H
#define MAME_ATARI_ORBIT_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define ORBIT_NOTE_FREQ       NODE_01
#define ORBIT_ANOTE1_AMP      NODE_02
#define ORBIT_ANOTE2_AMP      NODE_03
#define ORBIT_NOISE1_AMP      NODE_04
#define ORBIT_NOISE2_AMP      NODE_05
#define ORBIT_WARNING_EN      NODE_06
#define ORBIT_NOISE_EN        NODE_07

DISCRETE_SOUND_EXTERN( orbit_discrete );

#endif // MAME_ATARI_ORBIT_A_H
