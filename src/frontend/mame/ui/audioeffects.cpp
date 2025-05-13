// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audioeffects.cpp

    Audio effects control

*********************************************************************/

#include "emu.h"
#include "ui/audioeffects.h"
#include "audio_effects/aeffect.h"

#include "audio_effect_compressor.h"
#include "audio_effect_eq.h"
#include "audio_effect_filter.h"

#include "ui/ui.h"

#include "osdepend.h"
#include "speaker.h"

namespace ui {

menu_audio_effects::menu_audio_effects(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
{
	set_heading(_("Audio Effects"));
}

menu_audio_effects::~menu_audio_effects()
{
}

double menu_audio_effects::change_f(const double *table, double value, int change)
{
	u32 bi = 0;
	double dt = 1e300;
	u32 index;
	for(index = 0; table[index]; index++) {
		double d1 = value - table[index];
		if(d1 < 0)
			d1 = -d1;
		if(d1 < dt) {
			dt = d1;
			bi = index;
		}
	}
	if((change != -1 || bi != 0) && (change != 1 || bi != index-1))
		bi += change;
	return table[bi];
}

u32 menu_audio_effects::change_u32(const u32 *table, u32 value, int change)
{
	u32 bi = 0;
	s32 dt = 2e9;
	u32 index;
	for(index = 0; table[index]; index++) {
		s32 d1 = value - table[index];
		if(d1 < 0)
			d1 = -d1;
		if(d1 < dt) {
			dt = d1;
			bi = index;
		}
	}
	if((change != -1 || bi != 0) && (change != 1 || bi != index-1))
		bi += change;
	return table[bi];
}

bool menu_audio_effects::handle(event const *ev)
{
	static const double latencies[] = {
		0.0005, 0.0010, 0.0025, 0.0050, 0.0100, 0.0250, 0.0500, 0
	};

	static const u32 lengths[] = {
		10, 20, 30, 40, 50, 75, 100, 200, 300, 400, 500, 0
	};

	static const u32 phases[] = {
		10, 20, 30, 40, 50, 75, 100, 200, 300, 400, 500, 1000, 0
	};


	if(!ev)
		return false;

	switch(ev->iptkey) {
	case IPT_UI_SELECT: {
		u16 chain = (uintptr_t(ev->itemref)) >> 16;
		u16 entry = (uintptr_t(ev->itemref)) & 0xffff;
		audio_effect *eff = chain == 0xffff ? machine().sound().default_effect_chain()[entry] : machine().sound().effect_chain(chain)[entry];
		switch(eff->type()) {
		case audio_effect::COMPRESSOR:
			menu::stack_push<menu_audio_effect_compressor>(ui(), container(), chain, entry, eff);
			break;

		case audio_effect::EQ:
			menu::stack_push<menu_audio_effect_eq>(ui(), container(), chain, entry, eff);
			break;

		case audio_effect::FILTER:
			menu::stack_push<menu_audio_effect_filter>(ui(), container(), chain, entry, eff);
			break;
		}
		return true;
	}

	case IPT_UI_CLEAR: {
		switch(uintptr_t(ev->itemref)) {
		case RS_TYPE:
			machine().sound().set_resampler_type(sound_manager::RESAMPLER_LOFI);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RS_LATENCY:
			machine().sound().set_resampler_hq_latency(0.005);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RS_LENGTH:
			machine().sound().set_resampler_hq_length(400);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RS_PHASES:
			machine().sound().set_resampler_hq_phases(200);
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
		break;
	}

	case IPT_UI_LEFT: {
		switch(uintptr_t(ev->itemref)) {
		case RS_TYPE:
			machine().sound().set_resampler_type(sound_manager::RESAMPLER_LOFI);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RS_LATENCY:
			machine().sound().set_resampler_hq_latency(change_f(latencies, machine().sound().resampler_hq_latency(), -1));
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RS_LENGTH:
			machine().sound().set_resampler_hq_length(change_u32(lengths, machine().sound().resampler_hq_length(), -1));
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RS_PHASES:
			machine().sound().set_resampler_hq_phases(change_u32(phases, machine().sound().resampler_hq_phases(), -1));
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
		break;
	}

	case IPT_UI_RIGHT: {
		switch(uintptr_t(ev->itemref)) {
		case RS_TYPE:
			machine().sound().set_resampler_type(sound_manager::RESAMPLER_HQ);
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RS_LATENCY:
			machine().sound().set_resampler_hq_latency(change_f(latencies, machine().sound().resampler_hq_latency(), 1));
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RS_LENGTH:
			machine().sound().set_resampler_hq_length(change_u32(lengths, machine().sound().resampler_hq_length(), 1));
			reset(reset_options::REMEMBER_POSITION);
			return true;

		case RS_PHASES:
			machine().sound().set_resampler_hq_phases(change_u32(phases, machine().sound().resampler_hq_phases(), 1));
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
		break;
	}
	}

	return false;
}


std::string menu_audio_effects::format_lat(double latency)
{
	return util::string_format("%3.1fms", 1000*latency);
}

std::string menu_audio_effects::format_u32(u32 val)
{
	return util::string_format("%u", val);
}

u32 menu_audio_effects::flag_type() const
{
	u32 flag = 0;
	u32 type = machine().sound().resampler_type();
	if(type != sound_manager::RESAMPLER_LOFI)
		flag |= FLAG_LEFT_ARROW;
	if(type != sound_manager::RESAMPLER_HQ)
		flag |= FLAG_RIGHT_ARROW;
	return flag;
}

u32 menu_audio_effects::flag_lat() const
{
	u32 flag = 0;
	double latency = machine().sound().resampler_hq_latency();
	if(latency > 0.0005)
		flag |= FLAG_LEFT_ARROW;
	if(latency < 0.0500)
		flag |= FLAG_RIGHT_ARROW;
	if(machine().sound().resampler_type() != sound_manager::RESAMPLER_HQ)
		flag |= FLAG_INVERT | FLAG_DISABLE;
	return flag;
}

u32 menu_audio_effects::flag_length() const
{
	u32 flag = 0;
	double latency = machine().sound().resampler_hq_length();
	if(latency > 10)
		flag |= FLAG_LEFT_ARROW;
	if(latency < 500)
		flag |= FLAG_RIGHT_ARROW;
	if(machine().sound().resampler_type() != sound_manager::RESAMPLER_HQ)
		flag |= FLAG_INVERT | FLAG_DISABLE;
	return flag;
}

u32 menu_audio_effects::flag_phases() const
{
	u32 flag = 0;
	double latency = machine().sound().resampler_hq_phases();
	if(latency > 10)
		flag |= FLAG_LEFT_ARROW;
	if(latency < 1000)
		flag |= FLAG_RIGHT_ARROW;
	if(machine().sound().resampler_type() != sound_manager::RESAMPLER_HQ)
		flag |= FLAG_INVERT | FLAG_DISABLE;
	return flag;
}

void menu_audio_effects::populate()
{
	auto &sound = machine().sound();
	for(s32 chain = 0; chain != sound.effect_chains(); chain++) {
		std::string tag = sound.effect_chain_tag(chain);
		item_append(tag, FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
		auto eff = sound.effect_chain(chain);
		for(u32 e = 0; e != eff.size(); e++)
			item_append(_(audio_effect::effect_names[eff[e]->type()]), 0, (void *)intptr_t((chain << 16) | e));
	}
	item_append(_("Default"), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
	auto eff = sound.default_effect_chain();
	for(u32 e = 0; e != eff.size(); e++)
		item_append(_(audio_effect::effect_names[eff[e]->type()]), 0, (void *)intptr_t((0xffff << 16) | e));

	item_append(_("Resampler"), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
	item_append(_("Type"), sound.resampler_type_names(sound.resampler_type()), flag_type(), (void *)RS_TYPE);
	item_append(_("HQ latency"), format_lat(sound.resampler_hq_latency()), flag_lat(), (void *)RS_LATENCY);
	item_append(_("HQ filter max size"), format_u32(sound.resampler_hq_length()), flag_length(), (void *)RS_LENGTH);
	item_append(_("HQ filter max phases"), format_u32(sound.resampler_hq_phases()), flag_phases(), (void *)RS_PHASES);
	item_append(menu_item_type::SEPARATOR);
}

void menu_audio_effects::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);
}

void menu_audio_effects::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
}

void menu_audio_effects::menu_activated()
{
	// scripts or the other form of the menu could have changed something in the mean time
	reset(reset_options::REMEMBER_POSITION);
}

void menu_audio_effects::menu_deactivated()
{
}


} // namespace ui

