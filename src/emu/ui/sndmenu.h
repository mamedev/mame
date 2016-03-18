// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/sndmenu.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef __UI_SNDMENU_H__
#define __UI_SNDMENU_H__

//-------------------------------------------------
//  class sound options menu
//-------------------------------------------------
class ui_menu_sound_options : public ui_menu
{
public:
	ui_menu_sound_options(running_machine &machine, render_container *container);
	virtual ~ui_menu_sound_options();
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

#endif /* __UI_SNDMENU_H__ */
