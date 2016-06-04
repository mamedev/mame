// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//======================================================================
//
//  sliderchangednotifier.cpp - Interface for a slider-changed callback
//
//======================================================================

#pragma once

#ifndef __SLIDER_CHANGED_NOTIFIER__
#define __SLIDER_CHANGED_NOTIFIER__

#include <cstdint>
#include <string>

using INT32 = std::int32_t;

class running_machine;

class slider_changed_notifier
{
public:
	virtual ~slider_changed_notifier() { }

	virtual INT32 slider_changed(running_machine &machine, void *arg, int id, std::string *str, INT32 newval) = 0;
};

#endif // __SLIDER_CHANGED_NOTIFIER__
