// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audio_effect_filter.cpp

    Filter configuration

*********************************************************************/

#include "emu.h"
#include "ui/audio_effect_compressor.h"

#include "ui/ui.h"

#include "audio_effects/aeffect.h"
#include "audio_effects/compressor.h"

namespace ui {

menu_audio_effect_compressor::menu_audio_effect_compressor(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect)
	: menu(mui, container)
{
	m_chain = chain;
	m_entry = entry;
	m_effect = dynamic_cast<audio_effect_compressor *>(effect);
	set_heading(util::string_format(
			(chain == 0xffff) ? _("menu-aeffect-heading", "%1$s (default)") : _("menu-aeffect-heading", "%1$s (%2$s)"),
			_("audio-effect", audio_effect::effect_names[audio_effect::COMPRESSOR]),
			(chain == 0xffff) ? "" : machine().sound().effect_chain_tag(chain)));
	set_process_flags(PROCESS_LR_REPEAT);
}

menu_audio_effect_compressor::~menu_audio_effect_compressor()
{
}

bool menu_audio_effect_compressor::handle(event const *ev)
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
		case MODE:
			m_effect->set_mode(0);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(_("menu-aeffect-compressor", "Bypass"));
			ev->item->set_flags(flag_mode());
			return true;

		case ATTACK:
			m_effect->set_attack(max(0, m_effect->attack() - (alt_pressed ? 10000 : ctrl_pressed ? 100 : shift_pressed ? 1 : 10)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_ms(m_effect->attack()));
			ev->item->set_flags(flag_lim(m_effect->attack(), 0, 300, m_effect->isset_attack()));
			return true;

		case RELEASE:
			if(alt_pressed && !shift_pressed)
				m_effect->set_release(0);
			else if(m_effect->release() < 0)
				m_effect->set_release(3000);
			else
				m_effect->set_release(max(0, m_effect->release() - (ctrl_pressed ? 1000 : (shift_pressed && alt_pressed) ? 1 : shift_pressed ? 10 : 100)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_release(m_effect->release()));
			ev->item->set_flags(flag_lim_special(m_effect->release(), 0, m_effect->isset_release()));
			return true;

		case RATIO:
			if(alt_pressed)
				m_effect->set_ratio(1);
			else if(m_effect->ratio() <= 0)
				m_effect->set_ratio(20);
			else
				m_effect->set_ratio(max(1, m_effect->ratio() - (ctrl_pressed ? 5 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_ratio(m_effect->ratio()));
			ev->item->set_flags(flag_lim_special(m_effect->ratio(), 1, m_effect->isset_ratio()));
			return true;

		case INPUT_GAIN:
			m_effect->set_input_gain(max(-12, m_effect->input_gain() - (alt_pressed ? 10000 : ctrl_pressed ? 6 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->input_gain()));
			ev->item->set_flags(flag_lim(m_effect->input_gain(), -12, 24, m_effect->isset_input_gain()));
			return true;

		case OUTPUT_GAIN:
			m_effect->set_output_gain(max(-12, m_effect->output_gain() - (alt_pressed ? 10000 : ctrl_pressed ? 6 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->output_gain()));
			ev->item->set_flags(flag_lim(m_effect->output_gain(), -12, 24, m_effect->isset_output_gain()));
			return true;

		case CONVEXITY:
			m_effect->set_convexity(max(-2, m_effect->convexity() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->convexity()));
			ev->item->set_flags(flag_lim(m_effect->convexity(), -2, 2, m_effect->isset_convexity()));
			return true;

		case THRESHOLD:
			m_effect->set_threshold(max(-60, m_effect->threshold() - (alt_pressed ? 10000 : ctrl_pressed ? 12 : shift_pressed ? 1 : 3)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->threshold()));
			ev->item->set_flags(flag_lim(m_effect->threshold(), -60, 6, m_effect->isset_threshold()));
			return true;

		case CHANNEL_LINK:
			m_effect->set_channel_link(max(0, m_effect->channel_link() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->channel_link()));
			ev->item->set_flags(flag_lim(m_effect->channel_link(), 0, 1, m_effect->isset_channel_link()));
			return true;

		case FEEDBACK:
			m_effect->set_feedback(max(0, m_effect->feedback() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->feedback()));
			ev->item->set_flags(flag_lim(m_effect->feedback(), 0, 1, m_effect->isset_feedback()));
			return true;

		case INERTIA:
			m_effect->set_inertia(max(-1, m_effect->inertia() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->inertia()));
			ev->item->set_flags(flag_lim(m_effect->inertia(), -1, 0.3, m_effect->isset_inertia()));
			return true;

		case INERTIA_DECAY:
			m_effect->set_inertia_decay(max(0.8, m_effect->inertia_decay() - (alt_pressed ? 10000 : ctrl_pressed ? 0.05 : 0.01)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->inertia_decay()));
			ev->item->set_flags(flag_lim(m_effect->inertia_decay(), 0.8, 0.96, m_effect->isset_inertia_decay()));
			return true;

		case CEILING:
			m_effect->set_ceiling(max(0.3, m_effect->ceiling() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->ceiling()));
			ev->item->set_flags(flag_lim(m_effect->ceiling(), 0.3, 3, m_effect->isset_ceiling()));
			return true;
		}
		break;
	}

	case IPT_UI_RIGHT: {
		switch(uintptr_t(ev->itemref)) {
		case MODE:
			m_effect->set_mode(1);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(_("menu-aeffect-compressor", "Active"));
			ev->item->set_flags(flag_mode());
			return true;

		case ATTACK:
			m_effect->set_attack(min(300, m_effect->attack() + (alt_pressed ? 10000 : ctrl_pressed ? 100 : shift_pressed ? 1 : 10)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_ms(m_effect->attack()));
			ev->item->set_flags(flag_lim(m_effect->attack(), 0, 300, m_effect->isset_attack()));
			return true;

		case RELEASE:
			if((alt_pressed && !shift_pressed) || m_effect->release() == 3000)
				m_effect->set_release(-1);
			else if(m_effect->release() >= 0)
				m_effect->set_release(min(3000, m_effect->release() + (ctrl_pressed ? 1000 : (shift_pressed && alt_pressed) ? 1 : shift_pressed ? 10 : 100)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_release(m_effect->release()));
			ev->item->set_flags(flag_lim_special(m_effect->release(), 0, m_effect->isset_release()));
			return true;

		case RATIO:
			if(alt_pressed || m_effect->ratio() == 20)
				m_effect->set_ratio(-1);
			else if(m_effect->ratio() > 0)
				m_effect->set_ratio(min(20, m_effect->ratio() + (ctrl_pressed ? 5 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_ratio(m_effect->ratio()));
			ev->item->set_flags(flag_lim_special(m_effect->ratio(), 1, m_effect->isset_ratio()));
			return true;

		case INPUT_GAIN:
			m_effect->set_input_gain(min(24, m_effect->input_gain() + (alt_pressed ? 10000 : ctrl_pressed ? 6 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->input_gain()));
			ev->item->set_flags(flag_lim(m_effect->input_gain(), -12, 24, m_effect->isset_input_gain()));
			return true;

		case OUTPUT_GAIN:
			m_effect->set_output_gain(min(24, m_effect->output_gain() + (alt_pressed ? 10000 : ctrl_pressed ? 6 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->output_gain()));
			ev->item->set_flags(flag_lim(m_effect->output_gain(), -12, 24, m_effect->isset_output_gain()));
			return true;

		case CONVEXITY:
			m_effect->set_convexity(min(2, m_effect->convexity() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->convexity()));
			ev->item->set_flags(flag_lim(m_effect->convexity(), -2, 2, m_effect->isset_convexity()));
			return true;

		case THRESHOLD:
			m_effect->set_threshold(min(6, m_effect->threshold() + (alt_pressed ? 10000 : ctrl_pressed ? 12 : shift_pressed ? 1 : 3)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->threshold()));
			ev->item->set_flags(flag_lim(m_effect->threshold(), -60, 6, m_effect->isset_threshold()));
			return true;

		case CHANNEL_LINK:
			m_effect->set_channel_link(min(1, m_effect->channel_link() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->channel_link()));
			ev->item->set_flags(flag_lim(m_effect->channel_link(), 0, 1, m_effect->isset_channel_link()));
			return true;

		case FEEDBACK:
			m_effect->set_feedback(min(1, m_effect->feedback() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->feedback()));
			ev->item->set_flags(flag_lim(m_effect->feedback(), 0, 1, m_effect->isset_feedback()));
			return true;

		case INERTIA:
			m_effect->set_inertia(min(0.3, m_effect->inertia() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->inertia()));
			ev->item->set_flags(flag_lim(m_effect->inertia(), -1, 0.3, m_effect->isset_inertia()));
			return true;

		case INERTIA_DECAY:
			m_effect->set_inertia_decay(min(0.96, m_effect->inertia_decay() + (alt_pressed ? 10000 : ctrl_pressed ? 0.05 : 0.01)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->inertia_decay()));
			ev->item->set_flags(flag_lim(m_effect->inertia_decay(), 0.8, 0.96, m_effect->isset_inertia_decay()));
			return true;

		case CEILING:
			m_effect->set_ceiling(min(3, m_effect->ceiling() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->ceiling()));
			ev->item->set_flags(flag_lim(m_effect->ceiling(), 0.3, 3, m_effect->isset_ceiling()));
			return true;
		}
		break;
	}

	case IPT_UI_CLEAR: {
		switch(uintptr_t(ev->itemref)) {
		case MODE:
			m_effect->reset_mode();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(m_effect->mode() ? _("menu-aeffect-compressor", "Active") : _("menu-aeffect-compressor", "Bypass"));
			ev->item->set_flags(flag_mode());
			return true;

		case ATTACK:
			m_effect->reset_attack();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_ms(m_effect->attack()));
			ev->item->set_flags(flag_lim(m_effect->attack(), 0, 300, m_effect->isset_attack()));
			return true;

		case RELEASE:
			m_effect->reset_release();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_ms(m_effect->release()));
			ev->item->set_flags(flag_lim(m_effect->release(), 0, 3000, m_effect->isset_release()));
			return true;

		case RATIO:
			m_effect->reset_ratio();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_ratio(m_effect->ratio()));
			ev->item->set_flags(flag_lim(m_effect->ratio(), 1, 20, m_effect->isset_ratio()));
			return true;

		case INPUT_GAIN:
			m_effect->reset_input_gain();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->input_gain()));
			ev->item->set_flags(flag_lim(m_effect->input_gain(), -12, 24, m_effect->isset_input_gain()));
			return true;

		case OUTPUT_GAIN:
			m_effect->reset_output_gain();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->output_gain()));
			ev->item->set_flags(flag_lim(m_effect->output_gain(), -12, 24, m_effect->isset_output_gain()));
			return true;

		case CONVEXITY:
			m_effect->reset_convexity();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->convexity()));
			ev->item->set_flags(flag_lim(m_effect->convexity(), -2, 2, m_effect->isset_convexity()));
			return true;

		case THRESHOLD:
			m_effect->reset_threshold();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_db(m_effect->threshold()));
			ev->item->set_flags(flag_lim(m_effect->threshold(), -60, 6, m_effect->isset_threshold()));
			return true;

		case CHANNEL_LINK:
			m_effect->reset_channel_link();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->channel_link()));
			ev->item->set_flags(flag_lim(m_effect->channel_link(), 0, 1, m_effect->isset_channel_link()));
			return true;

		case FEEDBACK:
			m_effect->reset_feedback();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->feedback()));
			ev->item->set_flags(flag_lim(m_effect->feedback(), 0, 1, m_effect->isset_feedback()));
			return true;

		case INERTIA:
			m_effect->reset_inertia();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->inertia()));
			ev->item->set_flags(flag_lim(m_effect->inertia(), -1, 0.3, m_effect->isset_inertia()));
			return true;

		case INERTIA_DECAY:
			m_effect->reset_inertia_decay();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->inertia_decay()));
			ev->item->set_flags(flag_lim(m_effect->inertia_decay(), 0.8, 0.96, m_effect->isset_inertia_decay()));
			return true;

		case CEILING:
			m_effect->reset_ceiling();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			ev->item->set_subtext(format_2dec(m_effect->ceiling()));
			ev->item->set_flags(flag_lim(m_effect->ceiling(), 0.3, 3, m_effect->isset_ceiling()));
			return true;

		}
		break;
	}
	}
	return false;
}

std::string menu_audio_effect_compressor::format_2dec(float val)
{
	return util::string_format(_("menu-aeffect-compressor", "%1$.2f"), val);
}

std::string menu_audio_effect_compressor::format_db(float val)
{
	return util::string_format(_("menu-aeffect-compressor", "%1$+g dB"), val);
}

std::string menu_audio_effect_compressor::format_ms(float val)
{
	return util::string_format(_("menu-aeffect-compressor", "%1$.0f ms"), val);
}

std::string menu_audio_effect_compressor::format_ratio(float val)
{
	if(val > 0)
		return util::string_format(_("menu-aeffect-compressor", "%1$g:1"), val);
	else
		return _("menu-aeffect-compressor", "Infinity:1");
}

std::string menu_audio_effect_compressor::format_release(float val)
{
	if(val < 0)
		return _("menu-aeffect-compressor", "Infinite");
	else
		return format_ms(val);
}

u32 menu_audio_effect_compressor::flag_mode() const
{
	u32 flag = 0;
	if(!m_effect->isset_mode())
		flag |= FLAG_INVERT;
	if(m_effect->mode())
		flag |= FLAG_LEFT_ARROW;
	else
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_compressor::flag_lim(float value, float min, float max, bool isset)
{
	u32 flag = 0;
	if(!isset)
		flag |= FLAG_INVERT;
	if(value > min)
		flag |= FLAG_LEFT_ARROW;
	if(value < max)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_compressor::flag_lim_special(float value, float min, bool isset)
{
	u32 flag = 0;
	if(!isset)
		flag |= FLAG_INVERT;
	if(value != min)
		flag |= FLAG_LEFT_ARROW;
	if(value >= min)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

void menu_audio_effect_compressor::populate()
{
	item_append(_("menu-aeffect-compressor", "Mode"), m_effect->mode() ? _("menu-aeffect-compressor", "Active") : _("menu-aeffect-compressor", "Bypass"), flag_mode(), (void *)MODE);
	item_append(_("menu-aeffect-compressor", "Threshold"), format_db(m_effect->threshold()), flag_lim(m_effect->threshold(), -60, 6, m_effect->isset_threshold()), (void *)THRESHOLD);
	item_append(_("menu-aeffect-compressor", "Ratio"), format_ratio(m_effect->ratio()), flag_lim_special(m_effect->ratio(), 1, m_effect->isset_ratio()), (void *)RATIO);
	item_append(_("menu-aeffect-compressor", "Attack"), format_ms(m_effect->attack()), flag_lim(m_effect->attack(), 0, 300, m_effect->isset_attack()), (void *)ATTACK);
	item_append(_("menu-aeffect-compressor", "Release"), format_release(m_effect->release()), flag_lim_special(m_effect->release(), 0, m_effect->isset_release()), (void *)RELEASE);
	item_append(_("menu-aeffect-compressor", "Input gain"), format_db(m_effect->input_gain()), flag_lim(m_effect->input_gain(), -12, 24, m_effect->isset_input_gain()), (void *)INPUT_GAIN);
	item_append(_("menu-aeffect-compressor", "Output gain"), format_db(m_effect->output_gain()), flag_lim(m_effect->output_gain(), -12, 24, m_effect->isset_output_gain()), (void *)OUTPUT_GAIN);

	item_append(_("menu-aeffect-compressor", "Advanced"), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
	item_append(_("menu-aeffect-compressor", "Convexity"), format_2dec(m_effect->convexity()), flag_lim(m_effect->convexity(), -2, 2, m_effect->isset_convexity()), (void *)CONVEXITY);
	item_append(_("menu-aeffect-compressor", "Channel link"), format_2dec(m_effect->channel_link()), flag_lim(m_effect->channel_link(), 0, 1, m_effect->isset_channel_link()), (void *)CHANNEL_LINK);
	item_append(_("menu-aeffect-compressor", "Feedback"), format_2dec(m_effect->feedback()), flag_lim(m_effect->feedback(), 0, 1, m_effect->isset_feedback()), (void *)FEEDBACK);
	item_append(_("menu-aeffect-compressor", "Inertia"), format_2dec(m_effect->inertia()), flag_lim(m_effect->inertia(), -1, 0.3, m_effect->isset_inertia()), (void *)INERTIA);
	item_append(_("menu-aeffect-compressor", "Inertia decay"), format_2dec(m_effect->inertia_decay()), flag_lim(m_effect->inertia_decay(), 0.8, 0.96, m_effect->isset_inertia_decay()), (void *)INERTIA_DECAY);
	item_append(_("menu-aeffect-compressor", "Ceiling"), format_2dec(m_effect->ceiling()), flag_lim(m_effect->ceiling(), 0.3, 3, m_effect->isset_ceiling()), (void *)CEILING);

	item_append(menu_item_type::SEPARATOR);
	item_append(_("menu-aeffect-compressor", "Reset All"), 0, (void *)RESET_ALL);
}

void menu_audio_effect_compressor::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);
}

void menu_audio_effect_compressor::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
}

void menu_audio_effect_compressor::menu_activated()
{
	// scripts or the other form of the menu could have changed something in the mean time
	reset(reset_options::REMEMBER_POSITION);
}

void menu_audio_effect_compressor::menu_deactivated()
{
}

}
