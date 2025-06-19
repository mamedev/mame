// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/audio_effect_compressor.h

    Compressor configuration

***************************************************************************/

#ifndef MAME_FRONTEND_UI_AUDIO_EFFECT_COMPRESSOR_H
#define MAME_FRONTEND_UI_AUDIO_EFFECT_COMPRESSOR_H

#pragma once

#include "ui/menu.h"

class audio_effect_compressor;

namespace ui {

class menu_audio_effect_compressor : public menu
{
public:
	menu_audio_effect_compressor(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect);
	virtual ~menu_audio_effect_compressor() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual void menu_activated() override;
	virtual void menu_deactivated() override;

private:
	enum {
		MODE,
		ATTACK,
		RELEASE,
		RATIO,
		INPUT_GAIN,
		OUTPUT_GAIN,
		CONVEXITY,
		THRESHOLD,
		CHANNEL_LINK,
		FEEDBACK,
		INERTIA,
		INERTIA_DECAY,
		CEILING
	};

	u16 m_chain, m_entry;
	audio_effect_compressor *m_effect;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	static std::string format_nodec(float val);
	static std::string format_1dec(float val);
	static std::string format_2dec(float val);
	static std::string format_db(float val);
	u32 flag_mode() const;
	static u32 flag_lim(float value, float min, float max, bool isset);
	static float max(float a, float b) { return a > b ? a : b; }
	static float min(float a, float b) { return a < b ? a : b; }
};

} // namespace ui

#endif // MAME_FRONTEND_UI_AUDIO_EFFECT_COMPRESSOR_H
