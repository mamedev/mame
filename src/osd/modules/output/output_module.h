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

	output_module(): m_machine(nullptr) { }

	virtual ~output_module() = default;

	virtual void notify(const char *outname, int32_t value) = 0;

	void set_machine(running_machine *machine) { m_machine = machine; }
	running_machine &machine() const { return *m_machine; }

private:
	running_machine *m_machine;
};

#endif // MAME_OSD_OUTPUT_OUTPUT_MODULE_H
