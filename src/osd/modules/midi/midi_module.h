// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * midi_module.h
 *
 */
#ifndef MAME_OSD_MODULES_MIDI_MIDI_MODULE_H
#define MAME_OSD_MODULES_MIDI_MIDI_MODULE_H

#pragma once

#include "interface/midiport.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>


//============================================================
//  CONSTANTS
//============================================================

#define OSD_MIDI_PROVIDER   "midiprovider"

class midi_module
{
public:
	virtual ~midi_module() = default;

	// specific routines

	virtual std::unique_ptr<osd::midi_input_port> create_input(std::string_view name) = 0;
	virtual std::unique_ptr<osd::midi_output_port> create_output(std::string_view name) = 0;
	virtual std::vector<osd::midi_port_info> list_midi_ports() = 0;
};

#endif // MAME_OSD_MODULES_MIDI_MIDI_MODULE_H
