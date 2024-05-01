// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Night Driver Audio

***************************************************************************/
#ifndef MAME_ATARI_NITEDRVR_A_H
#define MAME_ATARI_NITEDRVR_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define NITEDRVR_BANG_DATA  NODE_01
#define NITEDRVR_SKID1_EN   NODE_02
#define NITEDRVR_SKID2_EN   NODE_03
#define NITEDRVR_MOTOR_DATA NODE_04
#define NITEDRVR_CRASH_EN   NODE_05
#define NITEDRVR_ATTRACT_EN NODE_06

DISCRETE_SOUND_EXTERN( nitedrvr_discrete );

#endif // MAME_ATARI_NITEDRVR_A_H
