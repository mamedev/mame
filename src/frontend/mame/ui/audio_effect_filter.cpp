// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audio_effect_filter.cpp

    Filter configuration

*********************************************************************/

#include "emu.h"
#include "ui/audio_effect_filter.h"

#include "ui/ui.h"

#include "audio_effects/aeffect.h"
#include "audio_effects/filter.h"

namespace ui {

namespace {

static constexpr u32 FH_MIN = 20;
static constexpr u32 FH_MAX = 5000;
static constexpr u32 FL_MIN = 100;
static constexpr u32 FL_MAX = 20000;

} // anonymous namespace

menu_audio_effect_filter::menu_audio_effect_filter(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect)
	: menu(mui, container)
{
	m_chain = chain;
	m_entry = entry;
	m_effect = dynamic_cast<audio_effect_filter *>(effect);
	set_heading(util::string_format(
			(chain == 0xffff) ? _("menu-aeffect-heading", "%1$s (default)") : _("menu-aeffect-heading", "%1$s (%2$s)"),
			_("audio-effect", audio_effect::effect_names[audio_effect::FILTER]),
			(chain == 0xffff) ? "" : machine().sound().effect_chain_tag(chain)));
	set_process_flags(PROCESS_LR_REPEAT);
}

menu_audio_effect_filter::~menu_audio_effect_filter()
{
}

u32 menu_audio_effect_filter::decrement_f(u32 f, bool alt_pressed, bool ctrl_pressed, bool shift_pressed)
{
	const u32 min = std::min(FH_MIN, FL_MIN);
	if(!shift_pressed && alt_pressed)
		return min;

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
	return (f < min) ? min : f;
}

u32 menu_audio_effect_filter::increment_f(u32 f, bool alt_pressed, bool ctrl_pressed, bool shift_pressed)
{
	const u32 max = std::max(FH_MAX, FL_MAX);
	if(!shift_pressed && alt_pressed)
		return max;

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
	return (f > max) ? max : f;
}

float menu_audio_effect_filter::decrement_q(float q, bool alt_pressed, bool ctrl_pressed, bool shift_pressed)
{
	const float min = 0.1f;
	if(alt_pressed)
		return min;

	float incval = ctrl_pressed ? -1 : shift_pressed ? -0.01f : -0.1f;
	q = roundf((q + incval) * 100.0f) / 100.0f;
	return (q < min) ? min : q;
}

float menu_audio_effect_filter::increment_q(float q, bool alt_pressed, bool ctrl_pressed, bool shift_pressed)
{
	const float max = 10.0f;
	if(alt_pressed)
		return max;

	float incval = ctrl_pressed ? 1 : shift_pressed ? 0.01f : 0.1f;
	q = roundf((q + incval) * 100.0f) / 100.0f;
	return (q > max) ? max : q;
}

bool menu_audio_effect_filter::handle(event const *ev)
{
	if(!ev)
		return false;

	bool alt_pressed = machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT);
	bool ctrl_pressed = machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL);
	bool shift_pressed = machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT);

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
		switch(uintptr_t(ev->itemref)) {
		case ACTIVE | HP:
			m_effect->set_highpass_active(false);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(_("menu-aeffect-filter", "Bypass"));
			ev->item->set_flags(flag_highpass_active());
			return true;

		case F | HP: {
			u32 f = decrement_f(m_effect->fh(), alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_fh(std::clamp(f, FH_MIN, FH_MAX));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_fh(m_effect->fh()));
			ev->item->set_flags(flag_fh());
			return true;
		}

		case Q | HP: {
			float q = decrement_q(m_effect->qh(), alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_qh(q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_q(m_effect->qh()));
			ev->item->set_flags(flag_qh());
			return true;
		}

		case ACTIVE | LP:
			m_effect->set_lowpass_active(false);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(_("menu-aeffect-filter", "Bypass"));
			ev->item->set_flags(flag_lowpass_active());
			return true;

		case F | LP: {
			u32 f = decrement_f(m_effect->fl(), alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_fl(std::clamp(f, FL_MIN, FL_MAX));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_fl(m_effect->fl()));
			ev->item->set_flags(flag_fl());
			return true;
		}

		case Q | LP: {
			float q = decrement_q(m_effect->ql(), alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_ql(q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_q(m_effect->ql()));
			ev->item->set_flags(flag_ql());
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
			ev->item->set_subtext(_("menu-aeffect-filter", "Active"));
			ev->item->set_flags(flag_highpass_active());
			return true;

		case F | HP: {
			u32 f = increment_f(m_effect->fh(), alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_fh(std::clamp(f, FH_MIN, FH_MAX));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_fh(m_effect->fh()));
			ev->item->set_flags(flag_fh());
			return true;
		}

		case Q | HP: {
			float q = increment_q(m_effect->qh(), alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_qh(q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_q(m_effect->qh()));
			ev->item->set_flags(flag_qh());
			return true;
		}

		case ACTIVE | LP:
			m_effect->set_lowpass_active(true);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(_("menu-aeffect-filter", "Active"));
			ev->item->set_flags(flag_lowpass_active());
			return true;

		case F | LP: {
			u32 f = increment_f(m_effect->fl(), alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_fl(std::clamp(f, FL_MIN, FL_MAX));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_fl(m_effect->fl()));
			ev->item->set_flags(flag_fl());
			return true;
		}

		case Q | LP: {
			float q = increment_q(m_effect->ql(), alt_pressed, ctrl_pressed, shift_pressed);
			m_effect->set_ql(q);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_q(m_effect->ql()));
			ev->item->set_flags(flag_ql());
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
			ev->item->set_subtext(m_effect->highpass_active() ? _("menu-aeffect-filter", "Active") : _("menu-aeffect-filter", "Bypass"));
			ev->item->set_flags(flag_highpass_active());
			return true;

		case F | HP:
			m_effect->reset_fh();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_fh(m_effect->fh()));
			ev->item->set_flags(flag_fh());
			return true;

		case Q | HP:
			m_effect->reset_qh();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_q(m_effect->qh()));
			ev->item->set_flags(flag_qh());
			return true;

		case ACTIVE | LP:
			m_effect->reset_lowpass_active();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(m_effect->lowpass_active() ? _("menu-aeffect-filter", "Active") : _("menu-aeffect-filter", "Bypass"));
			ev->item->set_flags(flag_lowpass_active());
			return true;

		case F | LP:
			m_effect->reset_fl();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_fl(m_effect->fl()));
			ev->item->set_flags(flag_fl());
			return true;

		case Q | LP:
			m_effect->reset_ql();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_q(m_effect->ql()));
			ev->item->set_flags(flag_ql());
			return true;
		}
		break;
	}
	}
	return false;
}

std::string menu_audio_effect_filter::format_fh(u32 f)
{
	return (f <= FH_MIN) ? _("menu-aeffect-filter", "DC removal") : util::string_format(_("menu-aeffect-filter", "%1$d Hz"), f);
}

std::string menu_audio_effect_filter::format_fl(u32 f)
{
	return util::string_format(_("menu-aeffect-filter", "%1$d Hz"), f);
}

std::string menu_audio_effect_filter::format_q(float q)
{
	return util::string_format(_("menu-aeffect-filter", "%1$.2f"), q);
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
	u32 f = m_effect->fh();
	if(f > FH_MIN)
		flag |= FLAG_LEFT_ARROW;
	if(f < FH_MAX)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_filter::flag_qh() const
{
	u32 flag = 0;
	if(!m_effect->isset_qh())
		flag |= FLAG_INVERT;
	u32 q = roundf(m_effect->qh() * 100.0f);
	if(q > 10)
		flag |= FLAG_LEFT_ARROW;
	if(q < 1000)
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
	u32 f = m_effect->fl();
	if(f > FL_MIN)
		flag |= FLAG_LEFT_ARROW;
	if(f < FL_MAX)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_filter::flag_ql() const
{
	u32 flag = 0;
	if(!m_effect->isset_ql())
		flag |= FLAG_INVERT;
	u32 q = roundf(m_effect->ql() * 100.0f);
	if(q > 10)
		flag |= FLAG_LEFT_ARROW;
	if(q < 1000)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

void menu_audio_effect_filter::populate()
{
	item_append(
			_("menu-aeffect-filter", "High-pass Filter"),
			FLAG_UI_HEADING | FLAG_DISABLE,
			nullptr);
	item_append(
			_("menu-aeffect-filter", "Mode"),
			m_effect->highpass_active() ? _("menu-aeffect-filter", "Active") : _("menu-aeffect-filter", "Bypass"),
			flag_highpass_active(),
			(void *)(ACTIVE | HP));
	item_append(
			_("menu-aeffect-filter", "Cutoff frequency"),
			format_fh(m_effect->fh()),
			flag_fh(),
			(void *)(F | HP));
	item_append(
			_("menu-aeffect-filter", "Q factor"),
			format_q(m_effect->qh()),
			flag_qh(),
			(void *)(Q | HP));

	item_append(
			_("menu-aeffect-filter", "Low-pass Filter"),
			FLAG_UI_HEADING | FLAG_DISABLE,
			nullptr);
	item_append(
			_("menu-aeffect-filter", "Mode"),
			m_effect->lowpass_active() ? _("menu-aeffect-filter", "Active") : _("menu-aeffect-filter", "Bypass"),
			flag_lowpass_active(),
			(void *)(ACTIVE | LP));
	item_append(
			_("menu-aeffect-filter", "Cutoff frequency"),
			format_fl(m_effect->fl()),
			flag_fl(),
			(void *)(F | LP));
	item_append(
			_("menu-aeffect-filter", "Q factor"),
			format_q(m_effect->ql()),
			flag_ql(),
			(void *)(Q | LP));

	item_append(menu_item_type::SEPARATOR);
	item_append(_("menu-aeffect-filter", "Reset All"), 0, (void *)RESET_ALL);
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
