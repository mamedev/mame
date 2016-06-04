// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/sndmenu.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_SNDMENU_H
#define MAME_FRONTEND_UI_SNDMENU_H

#include "ui/menu.h"

namespace ui {

//-------------------------------------------------
//  class sound options menu
//-------------------------------------------------
class menu_sound_options : public menu
{
public:
	menu_sound_options(mame_ui_manager &mui, render_container *container);
	virtual ~menu_sound_options() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	enum
	{
		ENABLE_SOUND = 1,
		SAMPLE_RATE,
		ENABLE_SAMPLES
	};

	UINT16            m_cur_rates;
	static const int  m_sound_rate[];
	int               m_sample_rate;
	bool              m_samples, m_sound;
};

} // namespace ui

#endif /* MAME_FRONTEND_UI_SNDMENU_H */
