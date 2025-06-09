// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audio_effect_filter.cpp

    Filter configuration

*********************************************************************/

#include "emu.h"
#include "ui/audio_effect_filter.h"
#include "audio_effects/aeffect.h"
#include "audio_effects/filter.h"

#include "ui/ui.h"

namespace ui {
const u32 menu_audio_effect_filter::freqs[2][38] = {
	{ 0, 20, 22, 24, 26, 28, 30, 32, 36, 40, 45, 50, 56, 63, 70, 80, 90, 100, 110, 125, 140, 160, 180, 200, 225, 250, 280, 315, 355, 400, 450, 500, 560, 630, 700, 800, 900, 1000 },
	{ 0, 1000, 1100, 1200, 1400, 1600, 1800, 2000, 2200, 2500, 2800, 3200, 3600, 4000, 4500, 5000, 5600, 6300, 7000, 8000, 9000, 10000, 11000, 12000, 14000, 16000, 18000, 20000 },
};

menu_audio_effect_filter::menu_audio_effect_filter(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect)
	: menu(mui, container)
{
	m_chain = chain;
	m_entry = entry;
	m_effect = static_cast<audio_effect_filter *>(effect);
	set_heading(util::string_format("%s #%u", chain == 0xffff ? _("Default") : machine().sound().effect_chain_tag(chain), entry+1));
	set_process_flags(PROCESS_LR_REPEAT | PROCESS_LR_ALWAYS);
}

menu_audio_effect_filter::~menu_audio_effect_filter()
{
}

std::pair<u32, u32> menu_audio_effect_filter::find_f(bool lp) const
{
	u32 variant = lp ? 1 : 0;
	u32 bi = 0;
	s32 dt = 40000;
	s32 f = s32((lp ? m_effect->fl() : m_effect->fh()) + 0.5);
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

void menu_audio_effect_filter::change_f(bool lp, s32 direction)
{
	auto [variant, bi] = find_f(lp);
	bi += direction;
	if(!freqs[variant][bi])
		bi -= direction;
	if(lp)
		m_effect->set_fl(freqs[variant][bi]);
	else
		m_effect->set_fh(freqs[variant][bi]);
	if(m_chain == 0xffff)
		machine().sound().default_effect_changed(m_entry);
}

bool menu_audio_effect_filter::handle(event const *ev)
{
	if(!ev)
		return false;

	switch(ev->iptkey) {
	case IPT_UI_LEFT: {
		switch(uintptr_t(ev->itemref)) {
		case ACTIVE | HP:
			m_effect->set_highpass_active(false);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case F | HP:
			change_f(false, -1);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case Q | HP: {
			float q = m_effect->qh();
			q = (int(q*10 + 0.5) - 1) / 10.0;
			if(q < 0.1)
				q = 0.1;
			m_effect->set_qh(q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}

		case ACTIVE | LP:
			m_effect->set_lowpass_active(false);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case F | LP:
			change_f(true, -1);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case Q | LP: {
			float q = m_effect->ql();
			q = (int(q*10 + 0.5) - 1) / 10.0;
			if(q < 0.1)
				q = 0.1;
			m_effect->set_ql(q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
		}
		break;
	}

	case IPT_UI_RIGHT: {
		switch(uintptr_t(ev->itemref)) {
		case ACTIVE | HP:
			m_effect->set_highpass_active(true);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case F | HP:
			change_f(false, +1);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case Q | HP: {
			float q = m_effect->qh();
			q = (int(q*10 + 0.5) + 1) / 10.0;
			if(q > 10)
				q = 10;
			m_effect->set_qh(q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}

		case ACTIVE | LP:
			m_effect->set_lowpass_active(true);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case F | LP:
			change_f(true, +1);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case Q | LP: {
			float q = m_effect->ql();
			q = (int(q*10 + 0.5) + 1) / 10.0;
			if(q > 10)
				q = 10;
			m_effect->set_ql(q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
		}
		break;
	}

	case IPT_UI_CLEAR: {
		switch(uintptr_t(ev->itemref)) {
		case ACTIVE | HP:
			m_effect->reset_highpass_active();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case F | HP:
			m_effect->reset_fh();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case Q | HP:
			m_effect->reset_qh();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ACTIVE | LP:
			m_effect->reset_lowpass_active();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case F | LP:
			m_effect->reset_fl();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case Q | LP:
			m_effect->reset_ql();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
		break;
	}
	}
	return false;
}

std::string menu_audio_effect_filter::format_f(float f)
{
	return f >= 1000 ? util::string_format("%.1fkHz", f/1000) : util::string_format("%.0fHz", f);
}

std::string menu_audio_effect_filter::format_q(float q)
{
	return util::string_format("%.1f", q);
}

u32 menu_audio_effect_filter::flag_highpass_active() const
{
	u32 flag = 0;
	if(!m_effect->isset_highpass_active())
		flag |= FLAG_INVERT;
	if(m_effect->highpass_active())
		flag |= FLAG_LEFT_ARROW;
	else
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_filter::flag_fh() const
{
	u32 flag = 0;
	if(!m_effect->isset_fh())
		flag |= FLAG_INVERT;
	auto [variant, bi] = find_f(false);
	if(freqs[variant][bi-1])
		flag |= FLAG_LEFT_ARROW;
	if(freqs[variant][bi+1])
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_filter::flag_qh() const
{
	u32 flag = 0;
	if(!m_effect->isset_qh())
		flag |= FLAG_INVERT;
	float q = m_effect->qh();
	if(q > 0.1)
		flag |= FLAG_LEFT_ARROW;
	if(q < 10)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_filter::flag_lowpass_active() const
{
	u32 flag = 0;
	if(!m_effect->isset_lowpass_active())
		flag |= FLAG_INVERT;
	if(m_effect->lowpass_active())
		flag |= FLAG_LEFT_ARROW;
	else
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_filter::flag_fl() const
{
	u32 flag = 0;
	if(!m_effect->isset_fl())
		flag |= FLAG_INVERT;
	auto [variant, bi] = find_f(true);
	if(freqs[variant][bi-1])
		flag |= FLAG_LEFT_ARROW;
	if(freqs[variant][bi+1])
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_filter::flag_ql() const
{
	u32 flag = 0;
	if(!m_effect->isset_ql())
		flag |= FLAG_INVERT;
	float q = m_effect->ql();
	if(q > 0.1)
		flag |= FLAG_LEFT_ARROW;
	if(q < 10)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

void menu_audio_effect_filter::populate()
{
	item_append(_(audio_effect::effect_names[audio_effect::FILTER]), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
	item_append(_("Highpass (DC removal)"), m_effect->highpass_active() ? _("Active") : _("Bypass"), flag_highpass_active(), (void *)(ACTIVE | HP));
	item_append(_("Highpass cutoff"), format_f(m_effect->fh()), flag_fh(), (void *)(F | HP)); 
	item_append(_("Highpass Q"), format_q(m_effect->qh()), flag_qh(), (void *)(Q | HP)); 

	item_append(_("Lowpass"), m_effect->lowpass_active() ? _("Active") : _("Bypass"), flag_lowpass_active(), (void *)(ACTIVE | LP));
	item_append(_("Lowpass cutoff"), format_f(m_effect->fl()), flag_fl(), (void *)(F | LP)); 
	item_append(_("Lowpass Q"), format_q(m_effect->ql()), flag_ql(), (void *)(Q | LP)); 

	item_append(menu_item_type::SEPARATOR);
}

void menu_audio_effect_filter::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);
}

void menu_audio_effect_filter::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
}

void menu_audio_effect_filter::menu_activated()
{
	// scripts or the other form of the menu could have changed something in the mean time
	reset(reset_options::REMEMBER_POSITION);
}

void menu_audio_effect_filter::menu_deactivated()
{
}

}
