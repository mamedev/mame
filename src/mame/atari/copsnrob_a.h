// license:BSD-3-Clause
// copyright-holders: Derrick Renaud

/***************************************************************************

Cops'n Rob audio

***************************************************************************/

#ifndef MAME_ATARI_COPSNROB_A_H
#define MAME_ATARI_COPSNROB_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define COPSNROB_MOTOR0_INV         NODE_01
#define COPSNROB_MOTOR1_INV         NODE_02
#define COPSNROB_MOTOR2_INV         NODE_03
#define COPSNROB_MOTOR3_INV         NODE_04
#define COPSNROB_ZINGS_INV          NODE_05
#define COPSNROB_FIRES_INV          NODE_06
#define COPSNROB_CRASH_INV          NODE_07
#define COPSNROB_SCREECH_INV        NODE_08
#define COPSNROB_AUDIO_ENABLE       NODE_09


DISCRETE_SOUND_EXTERN( copsnrob_discrete );

#endif // MAME_ATARI_COPSNROB_A_H
