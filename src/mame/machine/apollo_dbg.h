// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer
/*
 * apollo_dbg.h
 *
 *  Created on: Nov 22, 2015
 *      Author: Hans Ostermeyer
 *
 */

#ifndef MAME_MACHINE_APOLLO_DBG_H
#define MAME_MACHINE_APOLLO_DBG_H

#pragma once

#include "cpu/m68000/m68000.h"


int apollo_debug_instruction_hook(m68000_base_device *device, offs_t curpc);

#endif // MAME_MACHINE_APOLLO_DBG_H
