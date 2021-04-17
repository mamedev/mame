// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods

/***************************************************************************

    ui/slider.h

    Internal data representation for an adjustment slider.

***************************************************************************/

#ifndef MAME_FRONTEND_MAME_UI_SLIDER_H
#define MAME_FRONTEND_MAME_UI_SLIDER_H

#pragma once

#include <functional>
#include <string>

#define SLIDER_NOCHANGE     0x12345678

typedef std::function<std::int32_t (std::string *, std::int32_t)> slider_update;

struct slider_state
{
	slider_state(const std::string &title, std::int32_t min, std::int32_t def, std::int32_t max, std::int32_t inc, slider_update func)
		: update(func), minval(min), defval(def), maxval(max), incval(inc), description(title)
	{
	}

	slider_state(std::string &&title, std::int32_t min, std::int32_t def, std::int32_t max, std::int32_t inc, slider_update func)
		: update(func), minval(min), defval(def), maxval(max), incval(inc), description(std::move(title))
	{
	}

	slider_update   update;             // callback
	std::int32_t    minval;             // minimum value
	std::int32_t    defval;             // default value
	std::int32_t    maxval;             // maximum value
	std::int32_t    incval;             // increment value
	std::string     description;        // textual description
};

#endif // MAME_FRONTEND_MAME_UI_SLIDER_H
