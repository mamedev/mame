// license:BSD-3-Clause
// copyright-holders:James Wallace
///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Electro mechanical meters                                             //
//                                                                       //
// 23-07-2004: Re-Animator                                               //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __METERS_H__
#define __METERS_H__

#include "emu.h"


#define MAXMECHMETERS 8

#define METERREACTTIME 0.025 // number of seconds meter has to be active to tick

void MechMtr_config(running_machine &machine, int number);

int  MechMtr_GetNumberMeters(void);

void MechMtr_Setcount( int id, long count);

long MechMtr_Getcount( int id);

void MechMtr_ReactTime(int id, long cycles);

int  MechMtr_update(   int id, int state);

int MechMtr_GetActivity(int id);
#endif
