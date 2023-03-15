// license:BSD-3-Clause
// copyright-holders:Derrick Renaud

/***************************************************************************

Triple Hunt Audio

***************************************************************************/
#ifndef MAME_ATARI_TRIPLHNT_A_H
#define MAME_ATARI_TRIPLHNT_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define TRIPLHNT_BEAR_ROAR_DATA NODE_01
#define TRIPLHNT_BEAR_EN        NODE_02
#define TRIPLHNT_SHOT_DATA      NODE_03
#define TRIPLHNT_SCREECH_EN     NODE_04
#define TRIPLHNT_LAMP_EN        NODE_05


DISCRETE_SOUND_EXTERN( triplhnt_discrete );
extern const char *const triplhnt_sample_names[];

#endif // MAME_ATARI_TRIPLHNT_A_H
