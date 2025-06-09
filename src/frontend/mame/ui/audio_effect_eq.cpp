// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audio_effect_eq.cpp

    Equalizer configuration

*********************************************************************/

#include "emu.h"
#include "ui/audio_effect_eq.h"
#include "audio_effects/aeffect.h"
#include "audio_effects/eq.h"

#include "ui/ui.h"

namespace ui {

const u32 menu_audio_effect_eq::freqs[3][43] = {
	{ 0,  32,  36,  40,  45,  50,  56,   63,   70,   80,   90,  100,  110,  125,  140,  160,  180,  200,  225,  250,  280,  315,  355,  400,  450,  500,  560,   630,   700,   800,   900,  1000, 1100, 1200, 1400, 1600, 1800, 2000 },
	{ 0, 100, 110, 125, 140, 160, 180,  200,  225,  250,  280,  315,  355,  400,  450,  500,  560,  630,  700,  800,  900, 1000, 1100, 1200, 1400, 1600, 1800,  2000,  2200,  2500,  2800,  3200, 3600, 4000, 4500, 5000, 5600, 6300, 7000, 8000, 9000, 10000 },
	{ 0, 500, 560, 630, 700, 800, 900, 1000, 1100, 1200, 1400, 1600, 1800, 2000, 2200, 2500, 2800, 3200, 3600, 4000, 4500, 5000, 5600, 6300, 7000, 8000, 9000, 10000, 11000, 12000, 14000, 16000 },
};

menu_audio_effect_eq::menu_audio_effect_eq(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect)
	: menu(mui, container)
{
	m_chain = chain;
	m_entry = entry;
	m_effect = static_cast<audio_effect_eq *>(effect);
	set_heading(util::string_format("%s #%u", chain == 0xffff ? _("Default") : machine().sound().effect_chain_tag(chain), entry+1));
	set_process_flags(PROCESS_LR_REPEAT | PROCESS_LR_ALWAYS);
}

menu_audio_effect_eq::~menu_audio_effect_eq()
{
}

std::pair<u32, u32> menu_audio_effect_eq::find_f(u32 band) const
{
	u32 variant = band == 0 ? 0 : band < 4 ? 1 : 2;
	u32 bi = 0;
	s32 dt = 40000;
	s32 f = s32(m_effect->f(band) + 0.5);
	for(u32 index = 1; freqs[variant][index]; index++) {
		s32 d1 = f - freqs[variant][index];
		if(d1 < 0)
			d1 = -d1;
		if(d1 < dt) {
			dt = d1;
			bi = index;
		}
	}
	return std::make_pair(variant, bi);
}

void menu_audio_effect_eq::change_f(u32 band, s32 direction)
{
	auto [variant, bi] = find_f(band);
	bi += direction;
	if(!freqs[variant][bi])
		bi -= direction;
	m_effect->set_f(band, freqs[variant][bi]);
	if(m_chain == 0xffff)
		machine().sound().default_effect_changed(m_entry);
}

bool menu_audio_effect_eq::handle(event const *ev)
{
	if(!ev)
		return false;

	u32 band = (uintptr_t(ev->itemref)) >> 16;
	u32 entry = (uintptr_t(ev->itemref)) & 0xffff;

	switch(ev->iptkey) {
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
			
		case F:
			change_f(band, -1);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case Q: {
			float q = m_effect->q(band);
			q = (int(q*10 + 0.5) - 1) / 10.0;
			if(q < 0.1)
				q = 0.1;
			m_effect->set_q(band, q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}

		case DB: {
			float db = m_effect->db(band);
			db -= 1;
			if(db < -12)
				db = -12;
			m_effect->set_db(band, db);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
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
			
		case F:
			change_f(band, +1);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case Q: {
			float q = m_effect->q(band);
			q = (int(q*10 + 0.5) + 1) / 10.0;
			if(q > 12)
				q = 12;
			m_effect->set_q(band, q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}

		case DB: {
			float db = m_effect->db(band);
			db += 1;
			if(db > 12)
				db = 12;
			m_effect->set_db(band, db);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
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
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case Q: {
			m_effect->reset_q(band);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}

		case DB: {
			m_effect->reset_db(band);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
		}
		break;
	}
	}
	return false;
}

std::string menu_audio_effect_eq::format_f(float f)
{
	return f >= 1000 ? util::string_format("%.1fkHz", f/1000) : util::string_format("%.0fHz", f);
}

std::string menu_audio_effect_eq::format_q(float q)
{
	return util::string_format("%.1f", q);
}

std::string menu_audio_effect_eq::format_db(float db)
{
	return util::string_format("%+.0fdB", db);
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
	auto [variant, bi] = find_f(band);
	if(freqs[variant][bi-1])
		flag |= FLAG_LEFT_ARROW;
	if(freqs[variant][bi+1])
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_eq::flag_q(u32 band) const
{
	u32 flag = 0;
	if(!m_effect->isset_q(band))
		flag |= FLAG_INVERT;
	float q = m_effect->q(band);
	if(q < 10)
		flag |= FLAG_LEFT_ARROW;
	if(q > 0.1)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_eq::flag_db(u32 band) const
{
	u32 flag = 0;
	if(!m_effect->isset_db(band))
		flag |= FLAG_INVERT;
	float db = m_effect->db(band);
	if(db < 12)
		flag |= FLAG_LEFT_ARROW;
	if(db > -12)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

void menu_audio_effect_eq::populate()
{
	item_append(_(audio_effect::effect_names[audio_effect::EQ]), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
	item_append(_("Mode"), m_effect->mode() ? _("5-Band EQ") : _("Bypass"), flag_mode(), (void *)MODE);
	item_append(_("Low band mode"), m_effect->low_shelf() ? _("Shelf") : _("Peak"), flag_low_shelf(), (void *)uintptr_t(SHELF | (0 << 16)));
	item_append(_("Low band freq."), format_f(m_effect->f(0)), flag_f(0), (void *)uintptr_t(F | (0 << 16)));
	if(!m_effect->low_shelf())
		item_append(_("Low band Q"),    format_q(m_effect->q(0)), flag_q(0), (void *)uintptr_t(Q | (0 << 16)));
	item_append(_("Low band dB"), format_db(m_effect->db(0)), flag_db(0), (void *)uintptr_t(DB | (0 << 16)));

	item_append(_("Lo mid band freq."), format_f(m_effect->f(1)), flag_f(1), (void *)uintptr_t(F | (1 << 16)));
	item_append(_("Lo mid band Q"),    format_q(m_effect->q(1)), flag_q(1), (void *)uintptr_t(Q | (1 << 16)));
	item_append(_("Lo mid band dB"), format_db(m_effect->db(1)), flag_db(1), (void *)uintptr_t(DB | (1 << 16)));

	item_append(_("Mid band freq."), format_f(m_effect->f(2)), flag_f(2), (void *)uintptr_t(F | (2 << 16)));
	item_append(_("Mid band Q"),    format_q(m_effect->q(2)), flag_q(2), (void *)uintptr_t(Q | (2 << 16)));
	item_append(_("Mid band dB"), format_db(m_effect->db(2)), flag_db(2), (void *)uintptr_t(DB | (2 << 16)));

	item_append(_("Hi mid band freq."), format_f(m_effect->f(3)), flag_f(3), (void *)uintptr_t(F | (3 << 16)));
	item_append(_("Hi mid band Q"),    format_q(m_effect->q(3)), flag_q(3), (void *)uintptr_t(Q | (3 << 16)));
	item_append(_("Hi mid band dB"), format_db(m_effect->db(3)), flag_db(3), (void *)uintptr_t(DB | (3 << 16)));


	item_append(_("High band mode"), m_effect->high_shelf() ? _("Shelf") : _("Peak"), flag_high_shelf(), (void *)uintptr_t(SHELF | (4 << 16)));
	item_append(_("High band freq."), format_f(m_effect->f(4)), flag_f(4), (void *)uintptr_t(F | (4 << 16)));
	if(!m_effect->high_shelf())
		item_append(_("High band Q"),    format_q(m_effect->q(4)), flag_q(4), (void *)uintptr_t(Q | (4 << 16)));
	item_append(_("High band dB"), format_db(m_effect->db(4)), flag_db(4), (void *)uintptr_t(DB | (4 << 16)));
	item_append(menu_item_type::SEPARATOR);
}

void menu_audio_effect_eq::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);
}

void menu_audio_effect_eq::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
}

void menu_audio_effect_eq::menu_activated()
{
	// scripts or the other form of the menu could have changed something in the mean time
	reset(reset_options::REMEMBER_POSITION);
}

void menu_audio_effect_eq::menu_deactivated()
{
}

}
