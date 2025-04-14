// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/audio_effect_eq.h

    Equalizer configuration

***************************************************************************/

#ifndef MAME_FRONTEND_UI_AUDIO_EFFECT_EQ_H
#define MAME_FRONTEND_UI_AUDIO_EFFECT_EQ_H

#pragma once

#include "ui/menu.h"

class audio_effect_eq;

namespace ui {

class menu_audio_effect_eq : public menu
{
public:
	menu_audio_effect_eq(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect);
	virtual ~menu_audio_effect_eq() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual void menu_activated() override;
	virtual void menu_deactivated() override;

private:
	enum { MODE = 1, SHELF, F, Q, DB };

	static const u32 freqs[3][43];

	u16 m_chain, m_entry;
	audio_effect_eq *m_effect;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	static std::string format_f(float f);
	static std::string format_q(float q);
	static std::string format_db(float db);
	u32 flag_mode() const;
	u32 flag_low_shelf() const;
	u32 flag_high_shelf() const;
	u32 flag_f(u32 band) const;
	u32 flag_q(u32 band) const;
	u32 flag_db(u32 band) const;

	std::pair<u32, u32> find_f(u32 band) const;
	void change_f(u32 band, s32 direction);
};

} // namespace ui

#endif // MAME_FRONTEND_UI_AUDIO_EFFECT_EQ_H
