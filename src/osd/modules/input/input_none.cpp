// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//============================================================
//
//  input_none.cpp - Default unimplemented input modules
//
//============================================================

#include "input_module.h"


namespace osd {

namespace {

class keyboard_input_none : public osd_module, public input_module
{
public:
	keyboard_input_none() : osd_module(OSD_KEYBOARDINPUT_PROVIDER, "none") { }
	int init(osd_interface &osd, const osd_options &options) override { return 0; }
	void input_init(running_machine &machine) override { }
	void poll_if_necessary(bool relative_reset) override { }
};


class mouse_input_none : public osd_module, public input_module
{
public:
	mouse_input_none() : osd_module(OSD_MOUSEINPUT_PROVIDER, "none") { }
	int init(osd_interface &osd, const osd_options &options) override { return 0; }
	void input_init(running_machine &machine) override { }
	void poll_if_necessary(bool relative_reset) override { }
};


class lightgun_input_none : public osd_module, public input_module
{
public:
	lightgun_input_none() : osd_module(OSD_LIGHTGUNINPUT_PROVIDER, "none") { }
	int init(osd_interface &osd, const osd_options &options) override { return 0; }
	void input_init(running_machine &machine) override { }
	void poll_if_necessary(bool relative_reset) override { }
};


class joystick_input_none : public osd_module, public input_module
{
public:
	joystick_input_none() : osd_module(OSD_JOYSTICKINPUT_PROVIDER, "none") { }
	int init(osd_interface &osd, const osd_options &options) override { return 0; }
	void input_init(running_machine &machine) override { }
	void poll_if_necessary(bool relative_reset) override { }
};

} // anonymous namesapce

} // namespace osd



MODULE_DEFINITION(KEYBOARD_NONE, osd::keyboard_input_none)
MODULE_DEFINITION(MOUSE_NONE, osd::mouse_input_none)
MODULE_DEFINITION(LIGHTGUN_NONE, osd::lightgun_input_none)
MODULE_DEFINITION(JOYSTICK_NONE, osd::joystick_input_none)
