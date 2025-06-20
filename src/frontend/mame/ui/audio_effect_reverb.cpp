// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audio_effect_reverb.cpp

    Reverb configuration

*********************************************************************/

#include "emu.h"
#include "ui/audio_effect_reverb.h"
#include "audio_effects/aeffect.h"
#include "audio_effects/reverb.h"

#include "ui/ui.h"

namespace ui {
menu_audio_effect_reverb::menu_audio_effect_reverb(mame_ui_manager &mui, render_container &container, u16 chain, u16 entry, audio_effect *effect)
	: menu(mui, container)
{
	m_chain = chain;
	m_entry = entry;
	m_effect = static_cast<audio_effect_reverb *>(effect);
	m_preset = m_effect->find_current_preset();
	set_heading(util::string_format("%s (%s)",
			_(audio_effect::effect_names[audio_effect::REVERB]),
			chain == 0xffff ? _("Default") : machine().sound().effect_chain_tag(chain)));
	set_process_flags(PROCESS_LR_REPEAT);
}

menu_audio_effect_reverb::~menu_audio_effect_reverb()
{
}

bool menu_audio_effect_reverb::handle(event const *ev)
{
	if(!ev)
		return false;

	bool alt_pressed = machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT);
	bool ctrl_pressed = machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL);
	bool shift_pressed = machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT);

	switch(ev->iptkey) {
	case IPT_UI_SELECT: {
		switch(uintptr_t(ev->itemref)) {
		case PRESET:
			m_effect->load_preset(m_preset);
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_REF);
			return true;

		case RESET_ALL:
			m_effect->reset_all();
			m_preset = m_effect->find_current_preset();
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

		case PRESET:
			if(m_preset != 0)
				m_preset --;
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case DRYL:
			m_effect->set_dry_level(change_percent(m_effect->dry_level(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case EL:
			m_effect->set_early_level(change_percent(m_effect->early_level(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LL:
			m_effect->set_late_level(change_percent(m_effect->late_level(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case E2LL:
			m_effect->set_early_to_late_level(change_percent(m_effect->early_to_late_level(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ERS:
			m_effect->set_early_room_size(change_percent(m_effect->early_room_size(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LRS:
			m_effect->set_late_room_size(change_percent(m_effect->late_room_size(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case SW:
			m_effect->set_stereo_width(change_percent(m_effect->stereo_width(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LDIFF:
			m_effect->set_late_diffusion(change_percent(m_effect->late_diffusion(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LWANDER:
			m_effect->set_late_wander(change_percent(m_effect->late_wander(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case EDAMP:
			m_effect->set_early_damping(change_freq(m_effect->early_damping(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LDAMP:
			m_effect->set_late_damping(change_freq(m_effect->late_damping(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LPDELAY:
			m_effect->set_late_predelay(change_ms(m_effect->late_predelay(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LDECAY:
			m_effect->set_late_global_decay(change_decay(m_effect->late_global_decay(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LSPIN:
			m_effect->set_late_spin(change_spin(m_effect->late_spin(), false, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ETAP:
			if(m_effect->early_tap_setup() != 0) {
				m_effect->set_early_tap_setup(m_effect->early_tap_setup() - 1);
				if(m_chain == 0xffff)
					machine().sound().default_effect_changed(m_entry);
				reset(reset_options::REMEMBER_POSITION);
			}
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

		case PRESET:
			if(m_preset != audio_effect_reverb::preset_count() - 1)
				m_preset ++;
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case DRYL:
			m_effect->set_dry_level(change_percent(m_effect->dry_level(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case EL:
			m_effect->set_early_level(change_percent(m_effect->early_level(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LL:
			m_effect->set_late_level(change_percent(m_effect->late_level(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case E2LL:
			m_effect->set_early_to_late_level(change_percent(m_effect->early_to_late_level(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ERS:
			m_effect->set_early_room_size(change_percent(m_effect->early_room_size(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LRS:
			m_effect->set_late_room_size(change_percent(m_effect->late_room_size(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case SW:
			m_effect->set_stereo_width(change_percent(m_effect->stereo_width(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LDIFF:
			m_effect->set_late_diffusion(change_percent(m_effect->late_diffusion(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LWANDER:
			m_effect->set_late_wander(change_percent(m_effect->late_wander(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case EDAMP:
			m_effect->set_early_damping(change_freq(m_effect->early_damping(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LDAMP:
			m_effect->set_late_damping(change_freq(m_effect->late_damping(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LPDELAY:
			m_effect->set_late_predelay(change_ms(m_effect->late_predelay(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LDECAY:
			m_effect->set_late_global_decay(change_decay(m_effect->late_global_decay(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LSPIN:
			m_effect->set_late_spin(change_spin(m_effect->late_spin(), true, shift_pressed, ctrl_pressed, alt_pressed));
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ETAP:
			if(m_effect->early_tap_setup() != audio_effect_reverb::early_tap_setup_count() - 1) {
				m_effect->set_early_tap_setup(m_effect->early_tap_setup() + 1);
				if(m_chain == 0xffff)
					machine().sound().default_effect_changed(m_entry);
				reset(reset_options::REMEMBER_POSITION);
			}
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

		case DRYL:
			m_effect->reset_dry_level();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case EL:
			m_effect->reset_early_level();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LL:
			m_effect->reset_late_level();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case E2LL:
			m_effect->reset_early_to_late_level();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ERS:
			m_effect->reset_early_room_size();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LRS:
			m_effect->reset_late_room_size();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case SW:
			m_effect->reset_stereo_width();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LDIFF:
			m_effect->reset_late_diffusion();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LWANDER:
			m_effect->reset_late_wander();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case EDAMP:
			m_effect->reset_early_damping();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LDAMP:
			m_effect->reset_late_damping();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LPDELAY:
			m_effect->reset_late_predelay();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LDECAY:
			m_effect->reset_late_global_decay();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case LSPIN:
			m_effect->reset_late_spin();
			if(m_chain == 0xffff)
				machine().sound().default_effect_changed(m_entry);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case ETAP:
			m_effect->reset_early_tap_setup();
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

std::string menu_audio_effect_reverb::format_percent(double val)
{
	return util::string_format("%d%%", u32(val));
}

std::string menu_audio_effect_reverb::format_freq(double val)
{
	return util::string_format("%d Hz", u32(val + 0.5));
}

std::string menu_audio_effect_reverb::format_ms(double val)
{
	return util::string_format("%5.1f ms", val);
}

std::string menu_audio_effect_reverb::format_decay(double val)
{
	return util::string_format("%5.2f s", val);
}

std::string menu_audio_effect_reverb::format_spin(double val)
{
	return util::string_format("%3.2f Hz", val);
}

u32 menu_audio_effect_reverb::flag_mode() const
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

u32 menu_audio_effect_reverb::flag_tap_setup() const
{
	u32 flag = 0;
	if(!m_effect->isset_early_tap_setup())
		flag |= FLAG_INVERT;
	if(m_effect->early_tap_setup() != 0)
		flag |= FLAG_LEFT_ARROW;
	if(m_effect->early_tap_setup() != audio_effect_reverb::early_tap_setup_count() - 1)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_reverb::flag_preset() const
{
	u32 flag = 0;
	if(m_preset != 0)
		flag |= FLAG_LEFT_ARROW;
	if(m_preset != audio_effect_reverb::preset_count() - 1)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_reverb::flag_percent(double val, bool isset)
{
	u32 flag = 0;
	if(!isset)
		flag |= FLAG_INVERT;
	if(val > 0)
		flag |= FLAG_LEFT_ARROW;
	if(val < 100)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_reverb::flag_freq(double val, bool isset)
{
	u32 flag = 0;
	if(!isset)
		flag |= FLAG_INVERT;
	if(val > 100)
		flag |= FLAG_LEFT_ARROW;
	if(val < 16000)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_reverb::flag_ms(double val, bool isset)
{
	u32 flag = 0;
	if(!isset)
		flag |= FLAG_INVERT;
	if(val > 0)
		flag |= FLAG_LEFT_ARROW;
	if(val < 200)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_reverb::flag_decay(double val, bool isset)
{
	u32 flag = 0;
	if(!isset)
		flag |= FLAG_INVERT;
	if(val > 0.1)
		flag |= FLAG_LEFT_ARROW;
	if(val < 30)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effect_reverb::flag_spin(double val, bool isset)
{
	u32 flag = 0;
	if(!isset)
		flag |= FLAG_INVERT;
	if(val > 0)
		flag |= FLAG_LEFT_ARROW;
	if(val < 5)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

double menu_audio_effect_reverb::change_percent(double val, bool inc, bool shift, bool ctrl, bool alt)
{
	double step = alt ? 100 : ctrl ? 20 : shift ? 1 : 5;
	if(inc)
		val = std::min(100.0, val + step);
	else
		val = std::max(0.0, val - step);
	return val;
}

double menu_audio_effect_reverb::change_freq(double val, bool inc, bool shift, bool ctrl, bool alt)
{
	double step = alt ? 16000 : ctrl ? 20 : shift ? 1 : 5;
	if(val >= 10000)
		step *= 100;
	else if(val >= 5000)
		step *= 50;
	else if(val >= 2500)
		step *= 20;
	else if(val >= 1000)
		step *= 10;
	else if(val >= 500)
		step *= 5;
	else if(val >= 250)
		step *= 2;

	if(inc)
		val = std::min(16000.0, val + step);
	else
		val = std::max(100.0, val - step);
	return val;
}

double menu_audio_effect_reverb::change_ms(double val, bool inc, bool shift, bool ctrl, bool alt)
{
	double step = alt ? 200 : ctrl ? 10 : shift ? 0.1 : 1;
	if(inc)
		val = std::min(200.0, val + step);
	else
		val = std::max(0.0, val - step);
	val = u32(val * 10 + 0.5) / 10.0;
	return val;
}

double menu_audio_effect_reverb::change_decay(double val, bool inc, bool shift, bool ctrl, bool alt)
{
	double step = alt ? 30 : ctrl ? 1 : shift ? 0.01 : 0.1;
	if(inc)
		val = std::min(30.0, val + step);
	else
		val = std::max(0.1, val - step);
	val = u32(val * 100 + 0.5) / 100.0;
	return val;
}

double menu_audio_effect_reverb::change_spin(double val, bool inc, bool shift, bool ctrl, bool alt)
{
	double step = alt ? 5 : ctrl ? 1 : shift ? 0.01 : 0.1;
	if(inc)
		val = std::min(5.0, val + step);
	else
		val = std::max(0.0, val - step);
	val = u32(val * 100 + 0.5) / 100.0;
	return val;
}


void menu_audio_effect_reverb::populate()
{
	item_append(_("Mode"), m_effect->mode() ? _("Active") : _("Bypass"), flag_mode(), (void *)MODE);
	item_append(_("Load preset"), util::string_format("%s%s", audio_effect_reverb::preset_name(m_preset), m_preset == m_effect->default_preset() ? _(" (Default)") : ""), flag_preset(), (void *)PRESET);
	item_append(_("Dry level"), format_percent(m_effect->dry_level()), flag_percent(m_effect->dry_level(), m_effect->isset_dry_level()), (void *)DRYL);
	item_append(_("Stereo width"), format_percent(m_effect->stereo_width()), flag_percent(m_effect->stereo_width(), m_effect->isset_stereo_width()), (void *)SW);

	item_append(_("Early Reflections"), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
	item_append(_("Room size"), format_percent(m_effect->early_room_size()), flag_percent(m_effect->early_room_size(), m_effect->isset_early_room_size()), (void *)ERS);
	item_append(_("Tap setup"), audio_effect_reverb::early_tap_setup_name(m_effect->early_tap_setup()), flag_tap_setup(), (void *)ETAP);
	item_append(_("Damping"), format_freq(m_effect->early_damping()), flag_freq(m_effect->early_damping(), m_effect->isset_early_damping()), (void *)EDAMP);
	item_append(_("Level"), format_percent(m_effect->early_level()), flag_percent(m_effect->early_level(), m_effect->isset_early_level()), (void *)EL);
	item_append(_("Send to Late"), format_percent(m_effect->early_to_late_level()), flag_percent(m_effect->early_to_late_level(), m_effect->isset_early_to_late_level()), (void *)E2LL);

	item_append(_("Late Reflections"), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
	item_append(_("Room size"), format_percent(m_effect->late_room_size()), flag_percent(m_effect->late_room_size(), m_effect->isset_late_room_size()), (void *)LRS);
	item_append(_("Damping"), format_freq(m_effect->late_damping()), flag_freq(m_effect->late_damping(), m_effect->isset_late_damping()), (void *)LDAMP);
	item_append(_("Pre-delay"), format_ms(m_effect->late_predelay()), flag_ms(m_effect->late_predelay(), m_effect->isset_late_predelay()), (void *)LPDELAY);
	item_append(_("Diffusion"), format_percent(m_effect->late_diffusion()), flag_percent(m_effect->late_diffusion(), m_effect->isset_late_diffusion()), (void *)LDIFF);
	item_append(_("Wander"), format_percent(m_effect->late_wander()), flag_percent(m_effect->late_wander(), m_effect->isset_late_wander()), (void *)LWANDER);
	item_append(_("Decay"), format_decay(m_effect->late_global_decay()), flag_decay(m_effect->late_global_decay(), m_effect->isset_late_global_decay()), (void *)LDECAY);
	item_append(_("Spin"), format_spin(m_effect->late_spin()), flag_spin(m_effect->late_spin(), m_effect->isset_late_spin()), (void *)LSPIN);
	item_append(_("Level"), format_percent(m_effect->late_level()), flag_percent(m_effect->late_level(), m_effect->isset_late_level()), (void *)LL);
	item_append(menu_item_type::SEPARATOR);
	item_append(_("Reset All"), 0, (void *)RESET_ALL);
}

void menu_audio_effect_reverb::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);
}

void menu_audio_effect_reverb::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
}

void menu_audio_effect_reverb::menu_activated()
{
	// scripts or the other form of the menu could have changed something in the mean time
	reset(reset_options::REMEMBER_POSITION);
}

void menu_audio_effect_reverb::menu_deactivated()
{
}

}
