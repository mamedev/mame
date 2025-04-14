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
	virtual void populate() override;
	virtual bool handle(event const *ev) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_AUDIOEFFECTS_H
