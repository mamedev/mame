// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//============================================================
//
//  input_module.h - OSD input module contracts
//
//============================================================

#ifndef INPUT_MODULE_H_
#define INPUT_MODULE_H_

#include "osdepend.h"
#include "modules/osdmodule.h"

class input_module : public osd_module
{
public:
	input_module(const char *type, const char *name)
		: osd_module(type, name)
	{
	}

	virtual void input_init(running_machine &machine) = 0;
	virtual void poll_if_necessary(running_machine &machine) = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void exit() override {};
};

//============================================================
//  CONSTANTS
//============================================================

#define OSD_KEYBOARDINPUT_PROVIDER   "keyboardprovider"
#define OSD_MOUSEINPUT_PROVIDER      "mouseprovider"
#define OSD_LIGHTGUNINPUT_PROVIDER   "lightgunprovider"
#define OSD_JOYSTICKINPUT_PROVIDER   "joystickprovider"

#endif /* INPUT_MODULE_H_ */
