// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//============================================================
//
//  input_none.cpp - Default unimplemented input modules
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

class keyboard_input_none : public input_module
{
public:
	keyboard_input_none()
		: input_module(OSD_KEYBOARDINPUT_PROVIDER, "none") {}
	int init(const osd_options &options) override { return 0; }
	void poll_if_necessary(running_machine &machine) override {};
	void input_init(running_machine &machine) override {};
	void pause() override {};
	void resume() override {};
};

MODULE_DEFINITION(KEYBOARD_NONE, keyboard_input_none)

class mouse_input_none : public input_module
{
public:
	mouse_input_none()
		: input_module(OSD_MOUSEINPUT_PROVIDER, "none") {}
	int init(const osd_options &options) override { return 0; }
	void input_init(running_machine &machine) override {};
	void poll_if_necessary(running_machine &machine) override {};
	void pause() override {};
	void resume() override {};
};

MODULE_DEFINITION(MOUSE_NONE, mouse_input_none)

class lightgun_input_none : public input_module
{
public:
	lightgun_input_none()
		: input_module(OSD_LIGHTGUNINPUT_PROVIDER, "none") {}
	int init(const osd_options &options) override { return 0; }
	void input_init(running_machine &machine) override {};
	void poll_if_necessary(running_machine &machine) override {};
	void pause() override {};
	void resume() override {};
};

MODULE_DEFINITION(LIGHTGUN_NONE, lightgun_input_none)

class joystick_input_none : public input_module
{
public:
	joystick_input_none()
		: input_module(OSD_JOYSTICKINPUT_PROVIDER, "none") {}
	int init(const osd_options &options) override { return 0; }
	void input_init(running_machine &machine) override {};
	void poll_if_necessary(running_machine &machine) override {};
	void pause() override {};
	void resume() override {};
};

MODULE_DEFINITION(JOYSTICK_NONE, joystick_input_none)
