// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/sndmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/sndmenu.h"
#include "ui/selector.h"
#include "cliopts.h"
#include "../osd/modules/lib/osdobj_common.h" // TODO: remove

const int ui_menu_sound_options::m_sound_rate[] = { 11025, 22050, 44100, 48000 };

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_sound_options::ui_menu_sound_options(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	osd_options &options = downcast<osd_options &>(machine.options());

	m_sample_rate = machine.options().sample_rate();
	m_sound = (strcmp(options.sound(), OSDOPTVAL_NONE) && strcmp(options.sound(), "0"));
	m_samples = machine.options().samples();

	int total = ARRAY_LENGTH(m_sound_rate);

	for (m_cur_rates = 0; m_cur_rates < total; m_cur_rates++)
		if (m_sample_rate == m_sound_rate[m_cur_rates])
			break;

	if (m_cur_rates == total)
		m_cur_rates = 2;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_sound_options::~ui_menu_sound_options()
{
	std::string error_string;
	emu_options &moptions = machine().options();

	if (strcmp(moptions.value(OSDOPTION_SOUND),m_sound ? OSDOPTVAL_AUTO : OSDOPTVAL_NONE)!=0)
	{
		moptions.set_value(OSDOPTION_SOUND, m_sound ? OSDOPTVAL_AUTO : OSDOPTVAL_NONE, OPTION_PRIORITY_CMDLINE, error_string);
		machine().options().mark_changed(OSDOPTION_SOUND);
	}
	if (moptions.int_value(OPTION_SAMPLERATE)!=m_sound_rate[m_cur_rates])
	{
		moptions.set_value(OPTION_SAMPLERATE, m_sound_rate[m_cur_rates], OPTION_PRIORITY_CMDLINE, error_string);
		machine().options().mark_changed(OPTION_SAMPLERATE);
	}
	if (moptions.bool_value(OPTION_SAMPLES)!=m_samples)
	{
		moptions.set_value(OPTION_SAMPLES, m_samples, OPTION_PRIORITY_CMDLINE, error_string);
		machine().options().mark_changed(OPTION_SAMPLES);
	}
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_sound_options::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		switch ((FPTR)m_event->itemref)
		{
			case ENABLE_SOUND:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT || m_event->iptkey == IPT_UI_SELECT)
				{
					m_sound = !m_sound;
					changed = true;
				}
				break;

			case SAMPLE_RATE:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_LEFT) ? m_cur_rates-- : m_cur_rates++;
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
				{
					int total = ARRAY_LENGTH(m_sound_rate);
					std::vector<std::string> s_sel(total);
					for (int index = 0; index < total; index++)
						s_sel[index] = std::to_string(m_sound_rate[index]);

					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, s_sel, m_cur_rates));
				}
				break;

			case ENABLE_SAMPLES:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT || m_event->iptkey == IPT_UI_SELECT)
				{
					m_samples = !m_samples;
					changed = true;
				}
				break;
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);

}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_sound_options::populate()
{
	UINT32 arrow_flags = get_arrow_flags(0, ARRAY_LENGTH(m_sound_rate) - 1, m_cur_rates);
	m_sample_rate = m_sound_rate[m_cur_rates];

	// add options items
	item_append(_("Sound"), m_sound ? _("On") : _("Off"), m_sound ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)(FPTR)ENABLE_SOUND);
	item_append(_("Sample Rate"), string_format("%d", m_sample_rate).c_str(), arrow_flags, (void *)(FPTR)SAMPLE_RATE);
	item_append(_("Use External Samples"), m_samples ? _("On") : _("Off"), m_samples ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)(FPTR)ENABLE_SAMPLES);
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	customtop = machine().ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_sound_options::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();
	mui.draw_text_full(container, _("Sound Options"), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, _("Sound Options"), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}
