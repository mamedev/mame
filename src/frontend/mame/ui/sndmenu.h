// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/sndmenu.h

    Internal UI user interface.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_SNDMENU_H
#define MAME_FRONTEND_UI_SNDMENU_H

#pragma once

#include "ui/menu.h"

namespace ui {

//-------------------------------------------------
//  class sound options menu
//-------------------------------------------------

class menu_sound_options : public menu
{
public:
	menu_sound_options(mame_ui_manager &mui, render_container &container);

protected:
	virtual void menu_dismissed() override;

private:
	enum
	{
		ENABLE_SOUND = 1,
		SAMPLE_RATE,
		ENABLE_SAMPLES
	};

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	uint16_t          m_cur_rates;
	static const int  m_sound_rate[];
	int               m_sample_rate;
	bool              m_samples, m_sound;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SNDMENU_H
