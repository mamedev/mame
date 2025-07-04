// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/audioeffects.h

    Audio effects control

***************************************************************************/

#ifndef MAME_FRONTEND_UI_AUDIOEFFECTS_H
#define MAME_FRONTEND_UI_AUDIOEFFECTS_H

#pragma once

#include "ui/menu.h"


namespace ui {

class menu_audio_effects : public menu
{
public:
	menu_audio_effects(mame_ui_manager &mui, render_container &container);
	virtual ~menu_audio_effects() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual void menu_activated() override;
	virtual void menu_deactivated() override;

private:
	enum { RS_TYPE = 0x1000, RS_LATENCY, RS_LENGTH, RS_PHASES };

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	u32 flag_type() const;
	u32 flag_lat() const;
	u32 flag_length() const;
	u32 flag_phases() const;

	static double change_f(const double *table, double value, int change);
	static u32 change_u32(const u32 *table, u32 value, int change);

	static std::string format_lat(double latency);
	static std::string format_u32(u32 val);
};

} // namespace ui

#endif // MAME_FRONTEND_UI_AUDIOEFFECTS_H
