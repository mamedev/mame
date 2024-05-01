// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Starship 1 Audio

***************************************************************************/
#ifndef MAME_ATARI_STARSHP1_A_H
#define MAME_ATARI_STARSHP1_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define STARSHP1_NOISE_AMPLITUDE    NODE_01
#define STARSHP1_TONE_PITCH         NODE_02
#define STARSHP1_MOTOR_SPEED        NODE_03
#define STARSHP1_NOISE_FREQ         NODE_04
#define STARSHP1_MOLVL              NODE_05
#define STARSHP1_SL2                NODE_06
#define STARSHP1_SL1                NODE_07
#define STARSHP1_KICKER             NODE_08
#define STARSHP1_PHASOR_ON          NODE_09
#define STARSHP1_ATTRACT            NODE_10


DISCRETE_SOUND_EXTERN( starshp1_discrete );

#endif // MAME_ATARI_STARSHP1_A_H
