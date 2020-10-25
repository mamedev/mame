// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods

/***************************************************************************

    ui/slider.h

    Internal data representation for an adjustment slider.

***************************************************************************/

#ifndef MAME_FRONTEND_MAME_UI_SLIDER_H
#define MAME_FRONTEND_MAME_UI_SLIDER_H

#pragma once

#include "sliderchangednotifier.h"

#include <functional>

#define SLIDER_NOCHANGE     0x12345678

typedef std::function<std::int32_t (running_machine &, void *, int, std::string *, std::int32_t)> slider_update;

struct slider_state
{
	slider_update   update;             // callback
	void *          arg = nullptr;      // argument
	std::int32_t    minval = 0;         // minimum value
	std::int32_t    defval = 0;         // default value
	std::int32_t    maxval = 0;         // maximum value
	std::int32_t    incval = 0;         // increment value
	int             id = 0;
	std::string     description;        // textual description
};

#endif // MAME_FRONTEND_MAME_UI_SLIDER_H
