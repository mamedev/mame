// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//============================================================
//
//  input_module.h - OSD input module contracts
//
//============================================================
#ifndef MAME_OSD_INPUT_INPUT_MODULE_H
#define MAME_OSD_INPUT_INPUT_MODULE_H

#pragma once

#include "osdepend.h"

#include "modules/osdmodule.h"


class input_module
{
public:
	virtual ~input_module() = default;

	virtual void input_init(running_machine &machine) = 0;
	virtual void poll_if_necessary(bool relative_reset) = 0;
};

//============================================================
//  CONSTANTS
//============================================================

#define OSD_KEYBOARDINPUT_PROVIDER   "keyboardprovider"
#define OSD_MOUSEINPUT_PROVIDER      "mouseprovider"
#define OSD_LIGHTGUNINPUT_PROVIDER   "lightgunprovider"
#define OSD_JOYSTICKINPUT_PROVIDER   "joystickprovider"

#endif // MAME_OSD_INPUT_INPUT_MODULE_H
