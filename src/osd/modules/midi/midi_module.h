// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * midi_module.h
 *
 */
#ifndef MAME_OSD_MODULES_MIDI_MIDI_MODULE_H
#define MAME_OSD_MODULES_MIDI_MIDI_MODULE_H

#pragma once

#include "osdepend.h"
#include "modules/osdmodule.h"

#include <memory>


//============================================================
//  CONSTANTS
//============================================================

#define OSD_MIDI_PROVIDER   "midiprovider"

class midi_module
{
public:
	virtual ~midi_module() { }
	// specific routines

	virtual std::unique_ptr<osd_midi_device> create_midi_device() = 0;
	// FIXME: should return a list of strings ...
	virtual void list_midi_devices() = 0;
};

#endif // MAME_OSD_MODULES_MIDI_MIDI_MODULE_H
