// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer
/*
 * apollo_dbg.h
 *
 *  Created on: Nov 22, 2015
 *      Author: Hans Ostermeyer
 *
 */

#pragma once

#ifndef __APOLLO_DBG_H__
#define __APOLLO_DBG_H__

#include "emu.h"

int apollo_debug_instruction_hook(m68000_base_device *device, offs_t curpc);

#endif
