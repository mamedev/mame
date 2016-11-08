// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//===========================================================================
//
//  effectchangenotifier.h - Interface for callbacks associated with
//  changing effect sliders or effect lists themselves
//
//===========================================================================

#pragma once

#ifndef RENDER_EFFECT_CHANGE_NOTIFIER
#define RENDER_EFFECT_CHANGE_NOTIFIER

class effect_change_notifier
{
public:
	effect_change_notifier() : m_sliders_dirty(false), m_effects_list_changed(false) { }
	virtual ~effect_change_notifier() { }

	bool sliders_dirty() { return m_sliders_dirty; }
	bool effects_list_changed() { return m_effects_list_changed; }

	virtual void set_sliders_dirty(const bool dirty) { m_sliders_dirty = dirty; }
	virtual void set_effects_list_changed(const bool changed) { m_effects_list_changed = changed; }

protected:
	bool m_sliders_dirty;
	bool m_effects_list_changed;
};

#endif // RENDER_EFFECTS_CHANGE_NOTIFIER
