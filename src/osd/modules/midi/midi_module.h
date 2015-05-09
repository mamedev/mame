// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * midi_module.h
 *
 */

#ifndef MIDI_MODULE_H_
#define MIDI_MODULE_H_

#include "osdepend.h"
#include "modules/osdmodule.h"

//============================================================
//  CONSTANTS
//============================================================

#define OSD_MIDI_PROVIDER   "midiprovider"

class midi_module
{
public:
	virtual ~midi_module() { }
	// specific routines

	virtual osd_midi_device *create_midi_device() = 0;
	// FIXME: should return a list of strings ...
	virtual void list_midi_devices(void) = 0;
};


#endif /* MIDI_MODULE_H_ */
