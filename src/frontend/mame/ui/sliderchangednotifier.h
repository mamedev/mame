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

using int32_t = std::int32_t;

class running_machine;

class slider_changed_notifier
{
public:
	virtual ~slider_changed_notifier() { }

	virtual int32_t slider_changed(running_machine &machine, void *arg, int id, std::string *str, int32_t newval) = 0;
};

#endif // __SLIDER_CHANGED_NOTIFIER__
