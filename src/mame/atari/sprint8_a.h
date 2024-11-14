// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Sprint 8 Audio

***************************************************************************/
#ifndef MAME_ATARI_SPRINT8_A_H
#define MAME_ATARI_SPRINT8_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define SPRINT8_CRASH_EN            NODE_01
#define SPRINT8_SCREECH_EN          NODE_02
#define SPRINT8_ATTRACT_EN          NODE_03
#define SPRINT8_MOTOR1_EN           NODE_04
#define SPRINT8_MOTOR2_EN           NODE_05
#define SPRINT8_MOTOR3_EN           NODE_06
#define SPRINT8_MOTOR4_EN           NODE_07
#define SPRINT8_MOTOR5_EN           NODE_08
#define SPRINT8_MOTOR6_EN           NODE_09
#define SPRINT8_MOTOR7_EN           NODE_10
#define SPRINT8_MOTOR8_EN           NODE_11


DISCRETE_SOUND_EXTERN( sprint8_discrete );

#endif // MAME_ATARI_SPRINT8_A_H
