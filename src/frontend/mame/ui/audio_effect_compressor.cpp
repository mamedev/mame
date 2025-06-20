// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audio_effect_filter.cpp

    Filter configuration

*********************************************************************/

#include "emu.h"
#include "ui/audio_effect_compressor.h"
#include "audio_effects/aeffect.h"
#include "audio_effects/compressor.h"

#include "ui/ui.h"

namespace ui {

menu_audio_effect_compressor::menu_audio_effect_compressor(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect)
	: menu(mui, container)
{
	m_chain = chain;
	m_entry = entry;
	m_effect = static_cast<audio_effect_compressor *>(effect);
	set_heading(util::string_format("%s (%s)",
			_(audio_effect::effect_names[audio_effect::COMPRESSOR]),
			chain == 0xffff ? _("Default") : machine().sound().effect_chain_tag(chain)));
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
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ATTACK:
			m_effect->set_attack(max(0, m_effect->attack() - (alt_pressed ? 10000 : ctrl_pressed ? 100 : shift_pressed ? 1 : 10)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RELEASE:
			m_effect->set_release(max(0, m_effect->release() - ((alt_pressed && !shift_pressed) ? 10000 : ctrl_pressed ? 1000 : (shift_pressed && alt_pressed) ? 1 : shift_pressed ? 10 : 100)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RATIO:
			m_effect->set_ratio(max(1, m_effect->ratio() - (alt_pressed ? 10000 : ctrl_pressed ? 5 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case INPUT_GAIN:
			m_effect->set_input_gain(max(-12, m_effect->input_gain() - (alt_pressed ? 10000 : ctrl_pressed ? 6 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case OUTPUT_GAIN:
			m_effect->set_output_gain(max(-12, m_effect->output_gain() - (alt_pressed ? 10000 : ctrl_pressed ? 6 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case CONVEXITY:
			m_effect->set_convexity(max(-2, m_effect->convexity() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case THRESHOLD:
			m_effect->set_threshold(max(-60, m_effect->threshold() - (alt_pressed ? 10000 : ctrl_pressed ? 20 : shift_pressed ? 1 : 5)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case CHANNEL_LINK:
			m_effect->set_channel_link(max(0, m_effect->channel_link() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case FEEDBACK:
			m_effect->set_feedback(max(0, m_effect->feedback() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case INERTIA:
			m_effect->set_inertia(max(-1, m_effect->inertia() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case INERTIA_DECAY:
			m_effect->set_inertia_decay(max(0.8, m_effect->inertia_decay() - (alt_pressed ? 10000 : ctrl_pressed ? 0.05 : 0.01)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case CEILING:
			m_effect->set_ceiling(max(0.3, m_effect->ceiling() - (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
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
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ATTACK:
			m_effect->set_attack(min(300, m_effect->attack() + (alt_pressed ? 10000 : ctrl_pressed ? 100 : shift_pressed ? 1 : 10)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RELEASE:
			m_effect->set_release(min(3000, m_effect->release() + ((alt_pressed && !shift_pressed) ? 10000 : ctrl_pressed ? 1000 : (shift_pressed && alt_pressed) ? 1 : shift_pressed ? 10 : 100)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RATIO:
			m_effect->set_ratio(min(20, m_effect->ratio() + (alt_pressed ? 10000 : ctrl_pressed ? 5 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case INPUT_GAIN:
			m_effect->set_input_gain(min(24, m_effect->input_gain() + (alt_pressed ? 10000 : ctrl_pressed ? 6 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case OUTPUT_GAIN:
			m_effect->set_output_gain(min(24, m_effect->output_gain() + (alt_pressed ? 10000 : ctrl_pressed ? 6 : shift_pressed ? 0.1 : 1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case CONVEXITY:
			m_effect->set_convexity(min(2, m_effect->convexity() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case THRESHOLD:
			m_effect->set_threshold(min(6, m_effect->threshold() + (alt_pressed ? 10000 : ctrl_pressed ? 20 : shift_pressed ? 1 : 5)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case CHANNEL_LINK:
			m_effect->set_channel_link(min(1, m_effect->channel_link() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case FEEDBACK:
			m_effect->set_feedback(min(1, m_effect->feedback() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case INERTIA:
			m_effect->set_inertia(min(0.3, m_effect->inertia() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case INERTIA_DECAY:
			m_effect->set_inertia_decay(min(0.96, m_effect->inertia_decay() + (alt_pressed ? 10000 : ctrl_pressed ? 0.05 : 0.01)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case CEILING:
			m_effect->set_ceiling(min(3, m_effect->ceiling() + (alt_pressed ? 10000 : ctrl_pressed ? 0.5 : shift_pressed ? 0.01 : 0.1)));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
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
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ATTACK:
			m_effect->reset_attack();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RELEASE:
			m_effect->reset_release();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RATIO:
			m_effect->reset_ratio();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case INPUT_GAIN:
			m_effect->reset_input_gain();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case OUTPUT_GAIN:
			m_effect->reset_output_gain();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case CONVEXITY:
			m_effect->reset_convexity();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case THRESHOLD:
			m_effect->reset_threshold();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case CHANNEL_LINK:
			m_effect->reset_channel_link();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case FEEDBACK:
			m_effect->reset_feedback();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case INERTIA:
			m_effect->reset_inertia();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case INERTIA_DECAY:
			m_effect->reset_inertia_decay();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case CEILING:
			m_effect->reset_ceiling();
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

std::string menu_audio_effect_compressor::format_2dec(float val)
{
	return util::string_format("%.2f", val);
}

std::string menu_audio_effect_compressor::format_db(float val)
{
	return util::string_format("%g dB", val);
}

std::string menu_audio_effect_compressor::format_ms(float val)
{
	return util::string_format("%.0f ms", val);
}

std::string menu_audio_effect_compressor::format_ratio(float val)
{
	return util::string_format("%g:1", val);
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

void menu_audio_effect_compressor::populate()
{
	item_append(_("Mode"), m_effect->mode() ? _("Active") : _("Bypass"), flag_mode(), (void *)MODE);
	item_append(_("Threshold"), format_db(m_effect->threshold()), flag_lim(m_effect->threshold(), -60, 6, m_effect->isset_threshold()), (void *)THRESHOLD);
	item_append(_("Ratio"), format_ratio(m_effect->ratio()), flag_lim(m_effect->ratio(), 1, 20, m_effect->isset_ratio()), (void *)RATIO);
	item_append(_("Attack"), format_ms(m_effect->attack()), flag_lim(m_effect->attack(), 0, 300, m_effect->isset_attack()), (void *)ATTACK);
	item_append(_("Release"), format_ms(m_effect->release()), flag_lim(m_effect->release(), 0, 3000, m_effect->isset_release()), (void *)RELEASE);
	item_append(_("Input gain"), format_db(m_effect->input_gain()), flag_lim(m_effect->input_gain(), -12, 24, m_effect->isset_input_gain()), (void *)INPUT_GAIN);
	item_append(_("Output gain"), format_db(m_effect->output_gain()), flag_lim(m_effect->output_gain(), -12, 24, m_effect->isset_output_gain()), (void *)OUTPUT_GAIN);

	item_append(menu_item_type::SEPARATOR);
	item_append(_("Convexity"), format_2dec(m_effect->convexity()), flag_lim(m_effect->convexity(), -2, 2, m_effect->isset_convexity()), (void *)CONVEXITY);
	item_append(_("Channel link"), format_2dec(m_effect->channel_link()), flag_lim(m_effect->channel_link(), 0, 1, m_effect->isset_channel_link()), (void *)CHANNEL_LINK);
	item_append(_("Feedback"), format_2dec(m_effect->feedback()), flag_lim(m_effect->feedback(), 0, 1, m_effect->isset_feedback()), (void *)FEEDBACK);
	item_append(_("Inertia"), format_2dec(m_effect->inertia()), flag_lim(m_effect->inertia(), -1, 0.3, m_effect->isset_inertia()), (void *)INERTIA);
	item_append(_("Inertia decay"), format_2dec(m_effect->inertia_decay()), flag_lim(m_effect->inertia_decay(), 0.8, 0.96, m_effect->isset_inertia_decay()), (void *)INERTIA_DECAY);
	item_append(_("Ceiling"), format_2dec(m_effect->ceiling()), flag_lim(m_effect->ceiling(), 0.3, 3, m_effect->isset_ceiling()), (void *)CEILING);

	item_append(menu_item_type::SEPARATOR);
	item_append(_("Reset All"), 0, (void *)RESET_ALL);
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
