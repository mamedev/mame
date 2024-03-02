// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/sndmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/sndmenu.h"

#include "ui/selector.h"
#include "ui/ui.h"

#include "../osd/modules/lib/osdobj_common.h" // TODO: remove


namespace ui {

const int menu_sound_options::m_sound_rate[] = { 11025, 22050, 44100, 48000 };

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_sound_options::menu_sound_options(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
	set_heading(_("Sound Options"));

	osd_options &options = downcast<osd_options &>(mui.machine().options());

	m_sample_rate = mui.machine().options().sample_rate();
	m_sound = (strcmp(options.sound(), OSDOPTVAL_NONE) && strcmp(options.sound(), "0"));
	m_samples = mui.machine().options().samples();
	m_compressor = mui.machine().options().compressor();

	int total = std::size(m_sound_rate);

	for (m_cur_rates = 0; m_cur_rates < total; m_cur_rates++)
		if (m_sample_rate == m_sound_rate[m_cur_rates])
			break;

	if (m_cur_rates == total)
		m_cur_rates = 2;
}

//-------------------------------------------------
//  menu_dismissed
//-------------------------------------------------

void menu_sound_options::menu_dismissed()
{
	emu_options &moptions = machine().options();

	if (strcmp(moptions.value(OSDOPTION_SOUND), m_sound ? OSDOPTVAL_AUTO : OSDOPTVAL_NONE))
		moptions.set_value(OSDOPTION_SOUND, m_sound ? OSDOPTVAL_AUTO : OSDOPTVAL_NONE, OPTION_PRIORITY_CMDLINE);

	if (moptions.bool_value(OPTION_COMPRESSOR) != m_compressor)
		moptions.set_value(OPTION_COMPRESSOR, m_compressor, OPTION_PRIORITY_CMDLINE);

	if (moptions.int_value(OPTION_SAMPLERATE) != m_sound_rate[m_cur_rates])
		moptions.set_value(OPTION_SAMPLERATE, m_sound_rate[m_cur_rates], OPTION_PRIORITY_CMDLINE);

	if (moptions.bool_value(OPTION_SAMPLES) != m_samples)
		moptions.set_value(OPTION_SAMPLES, m_samples, OPTION_PRIORITY_CMDLINE);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_sound_options::handle(event const *ev)
{
	bool changed = false;

	// process the menu
	if (ev && ev->itemref)
	{
		switch ((uintptr_t)ev->itemref)
		{
		case ENABLE_SOUND:
			if (ev->iptkey == IPT_UI_LEFT || ev->iptkey == IPT_UI_RIGHT || ev->iptkey == IPT_UI_SELECT)
			{
				m_sound = !m_sound;
				changed = true;
			}
			break;

		case ENABLE_COMPRESSOR:
			if (ev->iptkey == IPT_UI_LEFT || ev->iptkey == IPT_UI_RIGHT || ev->iptkey == IPT_UI_SELECT)
			{
				m_compressor = !m_compressor;
				changed = true;
			}
			break;

		case SAMPLE_RATE:
			if (ev->iptkey == IPT_UI_LEFT || ev->iptkey == IPT_UI_RIGHT)
			{
				(ev->iptkey == IPT_UI_LEFT) ? m_cur_rates-- : m_cur_rates++;
				changed = true;
			}
			else if (ev->iptkey == IPT_UI_SELECT)
			{
				int total = std::size(m_sound_rate);
				std::vector<std::string> s_sel(total);
				for (int index = 0; index < total; index++)
					s_sel[index] = std::to_string(m_sound_rate[index]);

				menu::stack_push<menu_selector>(
						ui(), container(), _("Sample Rate"), std::move(s_sel), m_cur_rates,
						[this] (int selection)
						{
							m_cur_rates = selection;
							reset(reset_options::REMEMBER_REF);
						});
			}
			break;

		case ENABLE_SAMPLES:
			if (ev->iptkey == IPT_UI_LEFT || ev->iptkey == IPT_UI_RIGHT || ev->iptkey == IPT_UI_SELECT)
			{
				m_samples = !m_samples;
				changed = true;
			}
			break;
		}
	}

	if (changed) // FIXME: most changes only require the item sub text to be updated
		reset(reset_options::REMEMBER_REF);
	return false;

}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_sound_options::populate()
{
	uint32_t arrow_flags = get_arrow_flags(uint16_t(0), uint16_t(std::size(m_sound_rate) - 1), m_cur_rates);
	m_sample_rate = m_sound_rate[m_cur_rates];

	// add options items
	item_append_on_off(_("Sound"), m_sound, 0, (void *)(uintptr_t)ENABLE_SOUND);
	item_append_on_off(_("Compressor"), m_compressor, 0, (void *)(uintptr_t)ENABLE_COMPRESSOR);
	item_append(_("Sample Rate"), string_format("%d", m_sample_rate), arrow_flags, (void *)(uintptr_t)SAMPLE_RATE);
	item_append_on_off(_("Use External Samples"), m_samples, 0, (void *)(uintptr_t)ENABLE_SAMPLES);
	item_append(menu_item_type::SEPARATOR);
}

} // namespace ui
