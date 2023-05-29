// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Subs Audio

***************************************************************************/
#ifndef MAME_ATARI_SUBS_A_H
#define MAME_ATARI_SUBS_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define SUBS_SONAR1_EN          NODE_01
#define SUBS_SONAR2_EN          NODE_02
#define SUBS_LAUNCH_DATA        NODE_03
#define SUBS_CRASH_DATA         NODE_04
#define SUBS_CRASH_EN           NODE_05
#define SUBS_EXPLODE_EN         NODE_06
#define SUBS_NOISE_RESET        NODE_07


DISCRETE_SOUND_EXTERN( subs_discrete );

#endif // MAME_ATARI_SUBS_A_H
