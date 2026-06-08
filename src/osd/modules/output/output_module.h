// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * outpout_module.h
 *
 */
#ifndef MAME_OSD_OUTPUT_OUTPUT_MODULE_H
#define MAME_OSD_OUTPUT_OUTPUT_MODULE_H

#pragma once

#include "interface/output.h"

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

	virtual void notify(osd::output_item const &item, std::int32_t seconds, std::int64_t attoseconds) = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void update() = 0;
};

#endif // MAME_OSD_OUTPUT_OUTPUT_MODULE_H
