// license:BSD-3-Clause
// copyright-holders: Mike Balfour

/*************************************************************************

    Atari Canyon Bomber sound system

*************************************************************************/
#ifndef MAME_ATARI_CANYON_A_H
#define MAME_ATARI_CANYON_A_H

#pragma once

#include "sound/discrete.h"

// Discrete Sound Input Nodes
#define CANYON_MOTOR1_DATA      NODE_01
#define CANYON_MOTOR2_DATA      NODE_02
#define CANYON_EXPLODE_DATA     NODE_03
#define CANYON_WHISTLE1_EN      NODE_04
#define CANYON_WHISTLE2_EN      NODE_05
#define CANYON_ATTRACT1_EN      NODE_06
#define CANYON_ATTRACT2_EN      NODE_07


DISCRETE_SOUND_EXTERN( canyon_discrete );

#endif // MAME_ATARI_CANYON_A_H
