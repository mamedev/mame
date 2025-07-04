// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audio_effect_eq.cpp

    Equalizer configuration

*********************************************************************/

#include "emu.h"
#include "ui/audio_effect_eq.h"

#include "ui/ui.h"

#include "audio_effects/aeffect.h"
#include "audio_effects/eq.h"

namespace ui {

namespace {

constexpr u32 FREQ_LIMITS[5][2] = {
	{ 20, 2000 },
	{ 100, 10000 },
	{ 100, 10000 },
	{ 100, 10000 },
	{ 500, 16000 }
};

} // anonymous namespace

menu_audio_effect_eq::menu_audio_effect_eq(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect)
	: menu(mui, container)
{
	m_chain = chain;
	m_entry = entry;
	m_effect = dynamic_cast<audio_effect_eq *>(effect);
	set_heading(util::string_format(
			(chain == 0xffff) ? _("menu-aeffect-heading", "%1$s (default)") : _("menu-aeffect-heading", "%1$s (%2$s)"),
			_("audio-effect", audio_effect::effect_names[audio_effect::EQ]),
			(chain == 0xffff) ? "" : machine().sound().effect_chain_tag(chain)));
	set_process_flags(PROCESS_LR_REPEAT);
}

menu_audio_effect_eq::~menu_audio_effect_eq()
{
}

u32 menu_audio_effect_eq::decrement_f(u32 band, bool alt_pressed, bool ctrl_pressed, bool shift_pressed)
{
	if(!shift_pressed && alt_pressed)
		return FREQ_LIMITS[band][0];

	u32 f = m_effect->f(band);

	// pseudo-logarithmic scale
	u32 incval;
	if(f <= 100)
		incval = 10;
	else if(f <= 500)
		incval = 50;
	else if(f <= 1000)
		incval = 100;
	else if(f <= 2500)
		incval = 250;
	else if(f <= 5000)
		incval = 500;
	else
		incval = 1000;

	if(shift_pressed && alt_pressed)
		incval /= 100;
	else if(shift_pressed)
		incval /= 10;
	else if(ctrl_pressed)
		incval *= 10;

	if(incval <= 1)
		incval = 1;
	else if(f % incval)
		f += incval - f % incval;

	f -= incval;
	return std::clamp(f, FREQ_LIMITS[band][0], FREQ_LIMITS[band][1]);
}

u32 menu_audio_effect_eq::increment_f(u32 band, bool alt_pressed, bool ctrl_pressed, bool shift_pressed)
{
	if(!shift_pressed && alt_pressed)
		return FREQ_LIMITS[band][1];

	u32 f = m_effect->f(band);

	// pseudo-logarithmic scale
	u32 incval;
	if(f >= 5000)
		incval = 1000;
	else if(f >= 2500)
		incval = 500;
	else if(f >= 1000)
		incval = 250;
	else if(f >= 500)
		incval = 100;
	else if(f >= 100)
		incval = 50;
	else
		incval = 10;

	if(shift_pressed && alt_pressed)
		incval /= 100;
	else if(shift_pressed)
		incval /= 10;
	else if(ctrl_pressed)
		incval *= 10;

	if(incval <= 1)
		incval = 1;
	else if(f % incval)
		f -= f % incval;

	f += incval;
	return std::clamp(f, FREQ_LIMITS[band][0], FREQ_LIMITS[band][1]);
}

float menu_audio_effect_eq::change_q(u32 band, bool inc, bool alt_pressed, bool ctrl_pressed, bool shift_pressed)
{
	float incval = alt_pressed ? 10000 : ctrl_pressed ? 1 : shift_pressed ? 0.01f : 0.1f;
	if(!inc)
		incval = -incval;

	float q = m_effect->q(band);
	q = roundf((q + incval) * 100.0f) / 100.0f;
	return std::clamp(q, 0.1f, 10.0f);
}

float menu_audio_effect_eq::change_db(u32 band, bool inc, bool alt_pressed, bool ctrl_pressed, bool shift_pressed)
{
	float incval = alt_pressed ? 10000 : ctrl_pressed ? 6 : shift_pressed ? 0.1f : 1;
	if(!inc)
		incval = -incval;

	float db = m_effect->db(band);
	db = roundf((db + incval) * 10.0f) / 10.0f;
	return std::clamp(db, -12.0f, 12.0f);
}

bool menu_audio_effect_eq::handle(event const *ev)
{
	if(!ev)
		return false;

	bool alt_pressed = machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT);
	bool ctrl_pressed = machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL);
	bool shift_pressed = machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT);

	u32 band = (uintptr_t(ev->itemref)) >> 16;
	u32 entry = (uintptr_t(ev->itemref)) & 0xffff;

	switch(ev->iptkey) {
	case IPT_UI_SELECT: {
		if(uintptr_t(ev->itemref) == RESET_ALL) {
			m_effect->reset_all();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_REF);
			return true;
		}
		break;
	}

	case IPT_UI_LEFT: {
		switch(entry) {
		case MODE:
			m_effect->set_mode(0);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case SHELF:
			if(band == 0)
				m_effect->set_low_shelf(true);
			else
				m_effect->set_high_shelf(true);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case F: {
			u32 f = decrement_f(band, alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_f(band, f);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_f(m_effect->f(band)));
			ev->item->set_flags(flag_f(band));
			return true;
		}

		case Q: {
			float q = change_q(band, false, alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_q(band, q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_q(m_effect->q(band)));
			ev->item->set_flags(flag_q(band));
			return true;
		}

		case DB: {
			float db = change_db(band, false, alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_db(band, db);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->db(band)));
			ev->item->set_flags(flag_db(band));
			return true;
		}
		}
		break;
	}

	case IPT_UI_RIGHT: {
		switch(entry) {
		case MODE:
			m_effect->set_mode(1);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case SHELF:
			if(band == 0)
				m_effect->set_low_shelf(false);
			else
				m_effect->set_high_shelf(false);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case F: {
			u32 f = increment_f(band, alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_f(band, f);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_f(m_effect->f(band)));
			ev->item->set_flags(flag_f(band));
			return true;
		}

		case Q: {
			float q = change_q(band, true, alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_q(band, q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_q(m_effect->q(band)));
			ev->item->set_flags(flag_q(band));
			return true;
		}

		case DB: {
			float db = change_db(band, true, alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_db(band, db);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->db(band)));
			ev->item->set_flags(flag_db(band));
			return true;
		}
		}
		break;
	}

	case IPT_UI_CLEAR: {
		switch(entry) {
		case MODE:
			m_effect->reset_mode();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case SHELF:
			if(band == 0)
				m_effect->reset_low_shelf();
			else
				m_effect->reset_high_shelf();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case F:
			m_effect->reset_f(band);
			ev->item->set_subtext(format_f(m_effect->f(band)));
			ev->item->set_flags(flag_f(band));
			return true;

		case Q: {
			m_effect->reset_q(band);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_q(m_effect->q(band)));
			ev->item->set_flags(flag_q(band));
			return true;
		}

		case DB: {
			m_effect->reset_db(band);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->db(band)));
			ev->item->set_flags(flag_db(band));
			return true;
		}
		}
		break;
	}

	case IPT_UI_PREV_GROUP:
		switch (entry)
		{
		case SHELF:
		case F:
		case Q:
		case DB:
			if(band > 0)
				set_selection((void *)uintptr_t(((band - 1) << 16) | ((band == 1) ? SHELF : F)));
			break;
		}
		break;

	case IPT_UI_NEXT_GROUP:
		switch (entry)
		{
		case SHELF:
		case F:
		case Q:
		case DB:
			if(band < 4)
				set_selection((void *)uintptr_t(((band + 1) << 16) | ((band == 3) ? SHELF : F)));
			break;
		}
		break;
	}
	return false;
}

std::string menu_audio_effect_eq::format_f(u32 f)
{
	return util::string_format(_("menu-aeffect-eq", "%1$d Hz"), f);
}

std::string menu_audio_effect_eq::format_q(float q)
{
	return util::string_format(_("menu-aeffect-eq", "%1$.2f"), q);
}

std::string menu_audio_effect_eq::format_db(float db)
{
	return util::string_format(_("menu-aeffect-eq", "%1$+g dB"), db);
}

u32 menu_audio_effect_eq::flag_mode() const
{
	u32 flag = 0;
	if(!m_effect->isset_mode())
		flag |= FLAG_INVERT;
	if(m_effect->mode() == 1)
		flag |= FLAG_LEFT_ARROW;
	if(m_effect->mode() == 0)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_eq::flag_low_shelf() const
{
	u32 flag = 0;
	if(!m_effect->isset_low_shelf())
		flag |= FLAG_INVERT;
	if(m_effect->low_shelf())
		flag |= FLAG_RIGHT_ARROW;
	else
		flag |= FLAG_LEFT_ARROW;
	return flag;
}

u32 menu_audio_effect_eq::flag_high_shelf() const
{
	u32 flag = 0;
	if(!m_effect->isset_high_shelf())
		flag |= FLAG_INVERT;
	if(m_effect->high_shelf())
		flag |= FLAG_RIGHT_ARROW;
	else
		flag |= FLAG_LEFT_ARROW;
	return flag;
}

u32 menu_audio_effect_eq::flag_f(u32 band) const
{
	u32 flag = 0;
	if(!m_effect->isset_f(band))
		flag |= FLAG_INVERT;
	u32 f = m_effect->f(band);
	if(f > FREQ_LIMITS[band][0])
		flag |= FLAG_LEFT_ARROW;
	if(f < FREQ_LIMITS[band][1])
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_eq::flag_q(u32 band) const
{
	u32 flag = 0;
	if(!m_effect->isset_q(band))
		flag |= FLAG_INVERT;
	u32 q = roundf(m_effect->q(band) * 100.0f);
	if(q > 10)
		flag |= FLAG_LEFT_ARROW;
	if(q < 1000)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_eq::flag_db(u32 band) const
{
	u32 flag = 0;
	if(!m_effect->isset_db(band))
		flag |= FLAG_INVERT;
	s32 db = roundf(m_effect->db(band) * 10.0f);
	if(db > -120)
		flag |= FLAG_LEFT_ARROW;
	if(db < 120)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

void menu_audio_effect_eq::populate()
{
	item_append(
			_("menu-aeffect-eq", "Mode"),
			m_effect->mode() ? _("menu-aeffect-eq", "Active") : _("menu-aeffect-eq", "Bypass"),
			flag_mode(),
			(void *)MODE);

	auto const add_band_controls =
			[this] (u32 band, bool need_q)
			{
				item_append(
						_("menu-aeffect-eq", "Frequency"),
						format_f(m_effect->f(band)),
						flag_f(band),
						(void *)uintptr_t(F | (band << 16)));
				if(need_q)
				{
					item_append(
							_("menu-aeffect-eq", "Q factor"),
							format_q(m_effect->q(band)),
							flag_q(band),
							(void *)uintptr_t(Q | (band << 16)));
				}
				item_append(
						_("menu-aeffect-mode", "Gain"),
						format_db(m_effect->db(band)),
						flag_db(band),
						(void *)uintptr_t(DB | (band << 16)));
			};

	item_append(
			_("menu-aeffect-eq", "Low Band"),
			FLAG_UI_HEADING | FLAG_DISABLE,
			nullptr);
	item_append(
			_("menu-aeffect-eq", "Mode"),
			m_effect->low_shelf() ? _("menu-aeffect-eq", "Shelf") : _("menu-aeffect-eq", "Peak"),
			flag_low_shelf(),
			(void *)uintptr_t(SHELF | (0 << 16)));
	add_band_controls(0, !m_effect->low_shelf());

	item_append(
			_("menu-aeffect-eq", "Low Mid Band"),
			FLAG_UI_HEADING | FLAG_DISABLE,
			nullptr);
	add_band_controls(1, true);

	item_append(
			_("menu-aeffect-eq", "Mid Band"),
			FLAG_UI_HEADING | FLAG_DISABLE,
			nullptr);
	add_band_controls(2, true);

	item_append(
			_("menu-aeffect-eq", "High Mid Band"),
			FLAG_UI_HEADING | FLAG_DISABLE,
			nullptr);
	add_band_controls(3, true);

	item_append(
			_("menu-aeffect-eq", "High Band"),
			FLAG_UI_HEADING | FLAG_DISABLE,
			nullptr);
	item_append(
			_("menu-aeffect-eq", "Mode"),
			m_effect->high_shelf() ? _("menu-aeffect-eq", "Shelf") : _("menu-aeffect-eq", "Peak"),
			flag_high_shelf(),
			(void *)uintptr_t(SHELF | (4 << 16)));
	add_band_controls(4, !m_effect->high_shelf());

	item_append(menu_item_type::SEPARATOR);
	item_append(_("menu-aeffect-eq", "Reset All"), 0, (void *)RESET_ALL);
}

void menu_audio_effect_eq::menu_activated()
{
	// scripts or the other form of the menu could have changed something in the mean time
	reset(reset_options::REMEMBER_POSITION);
}

void menu_audio_effect_eq::menu_deactivated()
{
}

} // namespace ui
