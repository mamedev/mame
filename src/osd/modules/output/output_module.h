// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * outpout_module.h
 *
 */
#ifndef MAME_OSD_OUTPUT_OUTPUT_MODULE_H
#define MAME_OSD_OUTPUT_OUTPUT_MODULE_H

#pragma once

#include "osdepend.h"

#include <cstdint>


//============================================================
//  CONSTANTS
//============================================================

#define OSD_OUTPUT_PROVIDER   "output"

class output_module
{
public:
	static inline constexpr unsigned IM_MAME_PAUSE     = 0;
	static inline constexpr unsigned IM_MAME_SAVESTATE = 1;

	virtual ~output_module() = default;

	virtual void notify(const char *outname, int32_t value) = 0;
};

#endif // MAME_OSD_OUTPUT_OUTPUT_MODULE_H
