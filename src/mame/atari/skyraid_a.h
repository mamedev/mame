// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Sky Raider Audio

***************************************************************************/
#ifndef MAME_ATARI_SKYRAID_A_H
#define MAME_ATARI_SKYRAID_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define SKYRAID_PLANE_SWEEP_EN      NODE_01
#define SKYRAID_MISSILE_EN          NODE_02
#define SKYRAID_EXPLOSION_EN        NODE_03
#define SKYRAID_PLANE_ON_EN         NODE_04
#define SKYRAID_PLANE_ON__IC_E8     SKYRAID_PLANE_ON_EN
#define SKYRAID_ATTRACT_EN          NODE_05


DISCRETE_SOUND_EXTERN( skyraid_discrete );

#endif // MAME_ATARI_SKYRAID_A_H
