// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
#ifndef MAME_BUS_A800_A800_CARTS_H
#define MAME_BUS_A800_A800_CARTS_H

#pragma once

#include "rom.h"
#include "a5200_supercart.h"
#include "atrax.h"
#include "bbsb.h"
#include "corina.h"
#include "maxflash.h"
#include "oss.h"
#include "phoenix.h"
#include "rtime8.h"
#include "sic.h"
#include "sparta.h"
#include "supercharger.h"
#include "telelink2.h"
#include "ultracart.h"
#include "williams.h"

void a800_left(device_slot_interface &device);
void a800_right(device_slot_interface &device);
void a5200_carts(device_slot_interface &device);

#endif // MAME_BUS_A800_A800_CARTS_H
