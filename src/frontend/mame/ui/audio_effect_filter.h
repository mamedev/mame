// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/audio_effect_filter.h

    Filter configuration

***************************************************************************/

#ifndef MAME_FRONTEND_UI_AUDIO_EFFECT_FILTER_H
#define MAME_FRONTEND_UI_AUDIO_EFFECT_FILTER_H

#pragma once

#include "ui/menu.h"

class audio_effect_filter;

namespace ui {

class menu_audio_effect_filter : public menu
{
public:
	menu_audio_effect_filter(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect);
	virtual ~menu_audio_effect_filter() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual void menu_activated() override;
	virtual void menu_deactivated() override;

private:
	enum { ACTIVE = 1, F = 2, Q = 3, HP = 0, LP = 8 };

	static const u32 freqs[2][38];

	u16 m_chain, m_entry;
	audio_effect_filter *m_effect;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	static std::string format_f(float f);
	static std::string format_q(float q);
	u32 flag_highpass_active() const;
	u32 flag_fh() const;
	u32 flag_qh() const;
	u32 flag_lowpass_active() const;
	u32 flag_fl() const;
	u32 flag_ql() const;

	std::pair<u32, u32> find_f(bool lp) const;
	void change_f(bool lp, s32 direction);
};

} // namespace ui

#endif // MAME_FRONTEND_UI_AUDIO_EFFECT_FILTER_H
