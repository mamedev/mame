// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//======================================================================
//
//  sliderchangednotifier.cpp - Interface for a slider-changed callback
//
//======================================================================

#ifndef MAME_FRONTEND_MAME_UI_SLIDERCHANGEDNOTIFIER_H
#define MAME_FRONTEND_MAME_UI_SLIDERCHANGEDNOTIFIER_H

#pragma once

#include <cstdint>
#include <string>

class running_machine;

class slider_changed_notifier
{
public:
	virtual ~slider_changed_notifier() { }

	virtual std::int32_t slider_changed(running_machine &machine, void *arg, int id, std::string *str, std::int32_t newval) = 0;
};

#endif // MAME_FRONTEND_MAME_UI_SLIDERCHANGEDNOTIFIER_H
