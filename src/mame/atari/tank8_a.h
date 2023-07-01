// license:BSD-3-Clause
// copyright-holders:Hans Andersson

/***************************************************************************

Tank 8 Audio

***************************************************************************/
#ifndef MAME_ATARI_TANK8_A_H
#define MAME_ATARI_TANK8_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define TANK8_CRASH_EN          NODE_01
#define TANK8_BUGLE_EN          NODE_02
#define TANK8_MOTOR1_EN         NODE_03
#define TANK8_MOTOR2_EN         NODE_04
#define TANK8_MOTOR3_EN         NODE_05
#define TANK8_MOTOR4_EN         NODE_06
#define TANK8_MOTOR5_EN         NODE_07
#define TANK8_MOTOR6_EN         NODE_08
#define TANK8_MOTOR7_EN         NODE_09
#define TANK8_MOTOR8_EN         NODE_10
#define TANK8_EXPLOSION_EN      NODE_11
#define TANK8_ATTRACT_EN        NODE_12
#define TANK8_BUGLE_DATA1       NODE_13
#define TANK8_BUGLE_DATA2       NODE_14


DISCRETE_SOUND_EXTERN( tank8_discrete );

#endif // MAME_ATARI_TANK8_A_H
