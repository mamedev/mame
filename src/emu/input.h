// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    input.h

    Handle input from the user.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_INPUT_H
#define MAME_EMU_INPUT_H

#include "interface/inputcode.h"
#include "interface/inputman.h"
#include "interface/inputseq.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

using osd::input_seq; // too much trouble to qualify this everywhere


// ======================> input_manager

// global machine-level information about devices
class input_manager : public osd::input_manager
{
public:
	// controller alias table typedef
	using devicemap_table = std::map<std::string, std::string>;

	// construction/destruction
	input_manager(running_machine &machine);
	~input_manager();

	// OSD interface
	virtual osd::input_device &add_device(input_device_class devclass, std::string_view name, std::string_view id, void *internal) override;

	// getters
	running_machine &machine() const { return m_machine; }
	input_class &device_class(input_device_class devclass) { assert(devclass >= DEVICE_CLASS_FIRST_VALID && devclass <= DEVICE_CLASS_LAST_VALID); return *m_class[devclass]; }

	// input code readers
	s32 code_value(input_code code);
	bool code_pressed(input_code code) { return code_value(code) != 0; }
	bool code_pressed_once(input_code code);

	// input code helpers
	input_device *device_from_code(input_code code) const;
	input_device_item *item_from_code(input_code code) const;
	input_code code_from_itemid(input_item_id itemid) const;
	std::string code_name(input_code code) const;
	std::string code_to_token(input_code code) const;
	input_code code_from_token(std::string_view _token);
	const char *standard_token(input_item_id itemid) const;

	// input sequence readers
	bool seq_pressed(const input_seq &seq);
	s32 seq_axis_value(const input_seq &seq, input_item_class &itemclass);

	// input sequence helpers
	input_seq seq_clean(const input_seq &seq) const;
	std::string seq_name(const input_seq &seq) const;
	std::string seq_to_tokens(const input_seq &seq) const;
	void seq_from_tokens(input_seq &seq, std::string_view _token);

	// misc
	bool map_device_to_controller(const devicemap_table &table);

private:
	// internal helpers
	void reset_memory();

	// internal state
	running_machine &   m_machine;
	input_code          m_switch_memory[64];

	// classes
	std::array<std::unique_ptr<input_class>, DEVICE_CLASS_MAXIMUM> m_class;
};


#endif  // MAME_EMU_INPUT_H
