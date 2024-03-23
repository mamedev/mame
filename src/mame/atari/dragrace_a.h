// license:BSD-3-Clause
// copyright-holders: Derrick Renaud

/***************************************************************************

Drag Race audio

***************************************************************************/

#ifndef MAME_ATARI_DRAGRACE_A_H
#define MAME_ATARI_DRAGRACE_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define DRAGRACE_SCREECH1_EN    NODE_01
#define DRAGRACE_SCREECH2_EN    NODE_02
#define DRAGRACE_LOTONE_EN      NODE_03
#define DRAGRACE_HITONE_EN      NODE_04
#define DRAGRACE_EXPLODE1_EN    NODE_05
#define DRAGRACE_EXPLODE2_EN    NODE_06
#define DRAGRACE_MOTOR1_DATA    NODE_07
#define DRAGRACE_MOTOR2_DATA    NODE_08
#define DRAGRACE_MOTOR1_EN      NODE_80
#define DRAGRACE_MOTOR2_EN      NODE_81
#define DRAGRACE_KLEXPL1_EN     NODE_82
#define DRAGRACE_KLEXPL2_EN     NODE_83
#define DRAGRACE_ATTRACT_EN     NODE_09


DISCRETE_SOUND_EXTERN( dragrace_discrete );

#endif // MAME_ATARI_DRAGRACE_A_H
