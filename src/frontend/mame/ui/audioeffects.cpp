// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audioeffects.cpp

    Audio effects control

*********************************************************************/

#include "emu.h"
#include "ui/audioeffects.h"
#include "audio_effects/aeffect.h"

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

bool menu_audio_effects::handle(event const *ev)
{
	if(ev && (ev->iptkey == IPT_UI_SELECT)) {
		u16 chain = (uintptr_t(ev->itemref)) >> 16;
		u16 entry = (uintptr_t(ev->itemref)) & 0xffff;
		audio_effect *eff = chain == 0xffff ? machine().sound().default_effect_chain()[entry] : machine().sound().effect_chain(chain)[entry];
		switch(eff->type()) {
		case audio_effect::FILTER:
			menu::stack_push<menu_audio_effect_filter>(ui(), container(), chain, entry, eff);
			break;

		case audio_effect::EQ:
			menu::stack_push<menu_audio_effect_eq>(ui(), container(), chain, entry, eff);
			break;
		}
		return true;
	}

	return false;
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

