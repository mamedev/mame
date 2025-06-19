// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/audio_effect_reverb.h

    Reverb configuration

***************************************************************************/

#ifndef MAME_FRONTEND_UI_AUDIO_EFFECT_REVERB_H
#define MAME_FRONTEND_UI_AUDIO_EFFECT_REVERB_H

#pragma once

#include "ui/menu.h"

class audio_effect_reverb;

namespace ui {

class menu_audio_effect_reverb : public menu
{
public:
	menu_audio_effect_reverb(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect);
	virtual ~menu_audio_effect_reverb() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual void menu_activated() override;
	virtual void menu_deactivated() override;

private:
	enum {
		MODE = 1,
		PRESET,
		DRYL,
		SW,

		ERS,
		ETAP,
		EDAMP,
		EL,

		LRS,
		LDAMP,
		LPDELAY,
		LDIFF,
		LWANDER,
		LDECAY,
		LSPIN,
		LL,
		E2LL,

		RESET_ALL
	};

	u16 m_chain, m_entry;
	u32 m_preset;
	audio_effect_reverb *m_effect;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	static std::string format_percent(double val);
	static std::string format_freq(double val);
	static std::string format_ms(double val);
	static std::string format_decay(double val);
	static std::string format_spin(double val);

	u32 flag_mode() const;
	u32 flag_tap_setup() const;
	u32 flag_preset() const;

	static u32 flag_percent(double val, bool isset);
	static u32 flag_freq(double val, bool isset);
	static u32 flag_ms(double val, bool isset);
	static u32 flag_decay(double val, bool isset);
	static u32 flag_spin(double val, bool isset);

	static double change_percent(double val, bool inc, bool shift, bool ctrl, bool alt);
	static double change_freq(double val, bool inc, bool shift, bool ctrl, bool alt);
	static double change_ms(double val, bool inc, bool shift, bool ctrl, bool alt);
	static double change_decay(double val, bool inc, bool shift, bool ctrl, bool alt);
	static double change_spin(double val, bool inc, bool shift, bool ctrl, bool alt);

};

} // namespace ui

#endif // MAME_FRONTEND_UI_AUDIO_EFFECT_REVERB_H
