// license:BSD-3-Clause
// copyright-holders: Derrick Renaud

/************************************************************************
 * m79amb Sound System Analog emulation
 * Nov 2008, Derrick Renaud
 ************************************************************************/

#ifndef MAME_RAMTEK_M79AMB_A_H
#define MAME_RAMTEK_M79AMB_A_H

#pragma once

#include "sound/discrete.h"

// discrete sound input nodes

#define M79AMB_BOOM_EN                  NODE_01
#define M79AMB_THUD_EN                  NODE_02
#define M79AMB_SHOT_EN                  NODE_03
#define M79AMB_MC_REV_EN                NODE_04
#define M79AMB_MC_CONTROL_EN            NODE_05
#define M79AMB_TANK_TRUCK_JEEP_EN       NODE_06
#define M79AMB_WHISTLE_A_EN             NODE_07
#define M79AMB_WHISTLE_B_EN             NODE_08


DISCRETE_SOUND_EXTERN( m79amb_discrete );

#endif // MAME_RAMTEK_M79AMB_A_H
