// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//====================================================================
//
//  sliderdirtynotifier.cpp - Interface for a slider-changed callback
//
//====================================================================

#pragma once

#ifndef __RENDER_SLIDER_DIRTY_NOTIFIER__
#define __RENDER_SLIDER_DIRTY_NOTIFIER__

class slider_dirty_notifier
{
public:
	virtual ~slider_dirty_notifier() { }

	virtual void set_sliders_dirty() = 0;
};

#endif // __RENDER_SLIDER_DIRTY_NOTIFIER__
