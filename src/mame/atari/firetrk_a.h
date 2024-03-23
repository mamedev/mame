// license:BSD-3-Clause
// copyright-holders: K.Wilkins, Derrick Renaud

/***************************************************************************

Fire Truck / Monte Carlo - Super Bug audio

***************************************************************************/

#ifndef MAME_ATARI_FIRETRK_A_H
#define MAME_ATARI_FIRETRK_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define FIRETRUCK_MOTOR_DATA    NODE_01
#define FIRETRUCK_HORN_EN       NODE_02
#define FIRETRUCK_SIREN_DATA    NODE_03
#define FIRETRUCK_CRASH_DATA    NODE_04
#define FIRETRUCK_SKID_EN       NODE_05
#define FIRETRUCK_BELL_EN       NODE_06
#define FIRETRUCK_ATTRACT_EN    NODE_07
#define FIRETRUCK_XTNDPLY_EN    NODE_08

#define MONTECAR_MOTOR_DATA         FIRETRUCK_MOTOR_DATA
#define MONTECAR_CRASH_DATA         FIRETRUCK_CRASH_DATA
#define MONTECAR_DRONE_MOTOR_DATA   FIRETRUCK_SIREN_DATA
#define MONTECAR_SKID_EN            FIRETRUCK_SKID_EN
#define MONTECAR_DRONE_LOUD_DATA    FIRETRUCK_BELL_EN
#define MONTECAR_BEEPER_EN          FIRETRUCK_XTNDPLY_EN
#define MONTECAR_ATTRACT_INV        FIRETRUCK_ATTRACT_EN

#define SUPERBUG_SPEED_DATA     FIRETRUCK_MOTOR_DATA
#define SUPERBUG_CRASH_DATA     FIRETRUCK_CRASH_DATA
#define SUPERBUG_SKID_EN        FIRETRUCK_SKID_EN
#define SUPERBUG_ASR_EN         FIRETRUCK_XTNDPLY_EN
#define SUPERBUG_ATTRACT_EN     FIRETRUCK_ATTRACT_EN


DISCRETE_SOUND_EXTERN( firetrk_discrete );
DISCRETE_SOUND_EXTERN( montecar_discrete );
DISCRETE_SOUND_EXTERN( superbug_discrete );

#endif // MAME_ATARI_FIRETRK_A_H
