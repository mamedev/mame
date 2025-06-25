// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audiomix.cpp

    Audio mixing/mapping control

*********************************************************************/

#include "emu.h"
#include "ui/audiomix.h"

// frontend
#include "ui/ui.h"

// emu
#include "speaker.h"

// osd
#include "osdepend.h"


namespace ui {

namespace {

enum {
	MT_UNDEFINED, // At startup
	MT_NONE,      // No mapping
	MT_FULL,      // Full mapping to node
	MT_CHANNEL,   // Channel-to-channel mapping
	MT_INTERNAL   // Go back to previous menu or other non-mapping entry
};

enum {
	ITM_GUEST_CHANNEL = 1,
	ITM_NODE,
	ITM_NODE_CHANNEL,
	ITM_DB,
	ITM_REMOVE,
	ITM_ADD_FULL,
	ITM_ADD_CHANNEL
};

} // anonymous namespace

menu_audio_mixer::menu_audio_mixer(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, m_generation(0)
	, m_reset_item(0)
{
	set_heading(_("menu-audiomix", "Audio Mixer"));
	m_reset_selection.m_maptype = MT_UNDEFINED;
	m_reset_selection.m_dev = nullptr;
	m_reset_selection.m_guest_channel = 0;
	m_reset_selection.m_node = 0;
	m_reset_selection.m_node_channel = 0;
}

menu_audio_mixer::~menu_audio_mixer()
{
}

bool menu_audio_mixer::handle(event const *ev)
{
	const auto selection_ref = uintptr_t(get_selection_ref());
	set_process_flags((BIT(selection_ref, 0, 3) == ITM_DB) ? PROCESS_LR_REPEAT : 0);

	if(!ev) {
		if(m_generation != machine().sound().get_osd_info().m_generation) {
			const uint32_t selection_num = selection_ref >> 3;
			if((1 <= selection_num) && (m_selections.size() >= selection_num))
			{
				m_reset_selection = m_selections[selection_num - 1];
				m_reset_item = BIT(selection_ref, 0, 3);
				reset(reset_options::REMEMBER_POSITION);
			}
			else
			{
				m_reset_selection.m_maptype = MT_INTERNAL;
				reset(reset_options::REMEMBER_REF);
			}
			return true;
		}
		return false;
	}

	const bool alt_pressed = machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT);
	const bool ctrl_pressed = machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL);
	const bool shift_pressed = machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT);

	const auto item_ref = uintptr_t(ev->itemref);
	const uint32_t item_num = item_ref >> 3;
	const uint32_t current_item = BIT(item_ref, 0, 3);
	select_entry *const current_selection = ((1 <= item_num) && (m_selections.size() >= item_num)) ? &m_selections[item_num - 1] : nullptr;

	switch(ev->iptkey) {
	case IPT_UI_PREV_GROUP:
		if(!current_selection || (current_selection->m_dev == m_selections.front().m_dev)) {
			return false;
		} else {
			uint32_t i = item_num - 2;
			sound_io_device *prev = nullptr;
			while(0 < i) {
				if(!prev) {
					if(m_selections[i].m_dev != current_selection->m_dev)
						prev = m_selections[i].m_dev;
				} else {
					if(m_selections[i].m_dev != prev) {
						++i;
						break;
					}
				}
				--i;
			}
			set_selection(reinterpret_cast<void *>(((i + 1) << 3) | ((m_selections[i].m_maptype == MT_NONE) ? ITM_ADD_FULL : ITM_GUEST_CHANNEL)));
			return true;
		}
		break;

	case IPT_UI_NEXT_GROUP:
		if(!current_selection || (current_selection->m_dev == m_selections.back().m_dev)) {
			return false;
		} else {
			for(uint32_t i = item_num; m_selections.size() > i; i++) {
				if(m_selections[i].m_dev != current_selection->m_dev) {
					set_selection(reinterpret_cast<void *>(((i + 1) << 3) | ((m_selections[i].m_maptype == MT_NONE) ? ITM_ADD_FULL : ITM_GUEST_CHANNEL)));
					break;
				}
			}
			return true;
		}

	case IPT_UI_SELECT:
		if(!current_selection)
			return false;

		switch(current_item) {
		case ITM_REMOVE:
			return remove_route(current_item - 1, *current_selection);

		case ITM_ADD_FULL:
			return add_full(*current_selection);

		case ITM_ADD_CHANNEL:
			return add_channel(*current_selection);
		}
		break;

	case IPT_UI_CLEAR:
		if(!current_selection)
			return false;

		switch(current_item) {
		case ITM_DB:
			current_selection->m_db = 0.0f;
			return set_route_volume(*ev->item, *current_selection);
		}
		break;

	case IPT_UI_LEFT:
		if(!current_selection)
			return false;

		switch(current_item) {
		case ITM_GUEST_CHANNEL:
			return set_prev_guest_channel(*current_selection);

		case ITM_NODE:
			return set_prev_node(*current_selection);

		case ITM_NODE_CHANNEL:
			return set_prev_node_channel(*current_selection);

		case ITM_DB:
			if(shift_pressed) {
				current_selection->m_db -= 0.1f;
			} else if(ctrl_pressed) {
				current_selection->m_db -= 10.0f;
			} else if(alt_pressed) {
				if(current_selection->m_db > 0.0f)
					current_selection->m_db = 0.0f;
				else
					current_selection->m_db = -96.0f;
			} else {
				current_selection->m_db -= 1.0f;
			}

			return set_route_volume(*ev->item, *current_selection);
		}
		break;

	case IPT_UI_RIGHT:
		if(!current_selection)
			return false;

		switch(current_item) {
		case ITM_GUEST_CHANNEL:
			return set_next_guest_channel(*current_selection);

		case ITM_NODE:
			return set_next_node(*current_selection);

		case ITM_NODE_CHANNEL:
			return set_next_node_channel(*current_selection);

		case ITM_DB:
			if(shift_pressed) {
				current_selection->m_db += 0.1f;
			} else if(ctrl_pressed) {
				current_selection->m_db += 10.0f;
			} else if(alt_pressed) {
				if(current_selection->m_db < 0.0f)
					current_selection->m_db = 0.0f;
				else
					current_selection->m_db = 12.0f;
			} else {
				current_selection->m_db += 1.0f;
			}

			return set_route_volume(*ev->item, *current_selection);
		}
		break;
	}

	return false;
}


bool menu_audio_mixer::add_full(select_entry &current_selection)
{
	if(full_mapping_available(current_selection.m_dev, 0)) {
		m_reset_selection.m_node = 0;
		machine().sound().config_add_sound_io_connection_default(current_selection.m_dev, 0.0);
	} else {
		const uint32_t node = find_next_available_node(current_selection.m_dev, 0);
		if(node == 0xffffffff)
		{
			machine().popmessage(util::string_format(
					current_selection.m_dev->is_output()
						? _("menu-audiomix", "No full routes available for output device %1$s")
						: _("menu-audiomix", "No full routes available for input device %1$s"),
					current_selection.m_dev->tag()));
			return false;
		}
		m_reset_selection.m_node = node;
		machine().sound().config_add_sound_io_connection_node(current_selection.m_dev, find_node_name(node), 0.0);
	}

	m_reset_selection.m_maptype = MT_FULL;
	m_reset_selection.m_dev = current_selection.m_dev;
	m_reset_selection.m_guest_channel = 0;
	m_reset_selection.m_node_channel = 0;
	m_reset_selection.m_db = 0.0;
	m_reset_item = ITM_GUEST_CHANNEL;
	reset(reset_options::REMEMBER_POSITION);
	return true;
}


bool menu_audio_mixer::add_channel(select_entry &current_selection)
{
	// Find a possible triplet, any triplet
	const auto &info = machine().sound().get_osd_info();
	uint32_t guest_channel;
	uint32_t node_index, node_id;
	uint32_t node_channel;
	uint32_t default_osd_id = current_selection.m_dev->is_output() ? info.m_default_sink : info.m_default_source;
	for(node_index = (default_osd_id == 0) ? 0 : 0xffffffff; node_index != info.m_nodes.size(); node_index++) {
		node_id = node_index == 0xffffffff ? 0 : info.m_nodes[node_index].m_id;
		uint32_t guest_channel_count = current_selection.m_dev->is_output() ? current_selection.m_dev->inputs() : current_selection.m_dev->outputs();
		uint32_t node_channel_count = 0;
		if(node_index == 0xffffffff) {
			for(uint32_t i = 0; i != info.m_nodes.size(); i++) {
				if(info.m_nodes[i].m_id == default_osd_id) {
					node_channel_count = current_selection.m_dev->is_output() ? info.m_nodes[i].m_sinks : info.m_nodes[i].m_sources;
					break;
				}
			}
		} else {
			node_channel_count = current_selection.m_dev->is_output() ? info.m_nodes[node_index].m_sinks : info.m_nodes[node_index].m_sources;
		}
		for(guest_channel = 0; guest_channel != guest_channel_count; guest_channel++) {
			for(node_channel = 0; node_channel != node_channel_count; node_channel++) {
				if(channel_mapping_available(current_selection.m_dev, guest_channel, node_id, node_channel))
					goto found;
			}
		}
	}
	machine().popmessage(util::string_format(
			current_selection.m_dev->is_output()
				? _("menu-audiomix", "No channel routes available for output device %1$s")
				: _("menu-audiomix", "No channel routes available for input device %1$s"),
			current_selection.m_dev->tag()));
	return false;

found:
	if(node_id)
		machine().sound().config_add_sound_io_channel_connection_node(current_selection.m_dev, guest_channel, info.m_nodes[node_index].name(), node_channel, 0.0);
	else
		machine().sound().config_add_sound_io_channel_connection_default(current_selection.m_dev, guest_channel, node_channel, 0.0);
	m_reset_selection.m_maptype = MT_CHANNEL;
	m_reset_selection.m_dev = current_selection.m_dev;
	m_reset_selection.m_guest_channel = guest_channel;
	m_reset_selection.m_node = node_id;
	m_reset_selection.m_node_channel = node_channel;
	m_reset_selection.m_db = 0.0;
	m_reset_item = ITM_GUEST_CHANNEL;
	reset(reset_options::REMEMBER_POSITION);
	return true;
}


bool menu_audio_mixer::remove_route(uint32_t cursel_index, select_entry &current_selection)
{
	if(current_selection.m_maptype == MT_FULL) {
		if(current_selection.m_node == 0)
			machine().sound().config_remove_sound_io_connection_default(current_selection.m_dev);
		else
			machine().sound().config_remove_sound_io_connection_node(current_selection.m_dev, find_node_name(current_selection.m_node));
	} else {
		if(current_selection.m_node == 0)
			machine().sound().config_remove_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel);
		else
			machine().sound().config_remove_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, find_node_name(current_selection.m_node), current_selection.m_node_channel);
	}

	// If the next item exists and is the same speaker, go there (visually, the cursor stays on the same line)
	// Otherwise if the previous item exists and is the same speaker, go there (visually, the cursor goes up once)
	// Otherwise create a MT_NONE, because one is going to appear at the same place
	if(cursel_index + 1 < m_selections.size() && m_selections[cursel_index+1].m_dev == current_selection.m_dev) {
		m_reset_selection = m_selections[cursel_index+1];
		m_reset_item = ITM_GUEST_CHANNEL;
	} else if(cursel_index != 0 && m_selections[cursel_index-1].m_dev == current_selection.m_dev) {
		m_reset_selection = m_selections[cursel_index-1];
		m_reset_item = ITM_GUEST_CHANNEL;
	} else {
		m_reset_selection.m_maptype = MT_NONE;
		m_reset_selection.m_dev = current_selection.m_dev;
		m_reset_selection.m_guest_channel = 0;
		m_reset_selection.m_node = 0;
		m_reset_selection.m_node_channel = 0;
		m_reset_selection.m_db = 0.0;
		m_reset_item = ITM_ADD_FULL;
	}

	reset(reset_options::REMEMBER_POSITION);
	return true;
}


bool menu_audio_mixer::set_prev_guest_channel(select_entry &current_selection)
{
	if(current_selection.m_maptype != MT_CHANNEL)
		return false;

	uint32_t guest_channel_count = current_selection.m_dev->is_output() ? current_selection.m_dev->inputs() : current_selection.m_dev->outputs();
	if(guest_channel_count == 1)
		return false;

	uint32_t guest_channel = current_selection.m_guest_channel;
	for(;;) {
		if(guest_channel == 0)
			guest_channel = guest_channel_count - 1;
		else
			guest_channel--;
		if(guest_channel == current_selection.m_guest_channel)
			return false;
		if(channel_mapping_available(current_selection.m_dev, guest_channel, current_selection.m_node, current_selection.m_node_channel)) {
			if(current_selection.m_node) {
				const auto node = find_node_name(current_selection.m_node);
				machine().sound().config_remove_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, node, current_selection.m_node_channel);
				machine().sound().config_add_sound_io_channel_connection_node(current_selection.m_dev, guest_channel, node, current_selection.m_node_channel, current_selection.m_db);
			} else {
				machine().sound().config_remove_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel);
				machine().sound().config_add_sound_io_channel_connection_default(current_selection.m_dev, guest_channel, current_selection.m_node_channel, current_selection.m_db);
			}
			m_reset_selection.m_maptype = MT_INTERNAL;
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
	}

	return false;
}


bool menu_audio_mixer::set_next_guest_channel(select_entry &current_selection)
{
	if(current_selection.m_maptype != MT_CHANNEL)
		return false;

	uint32_t guest_channel_count = current_selection.m_dev->is_output() ? current_selection.m_dev->inputs() : current_selection.m_dev->outputs();
	if(guest_channel_count == 1)
		return false;

	uint32_t guest_channel = current_selection.m_guest_channel;
	for(;;) {
		guest_channel++;
		if(guest_channel == guest_channel_count)
			guest_channel = 0;
		if(guest_channel == current_selection.m_guest_channel)
			return false;
		if(channel_mapping_available(current_selection.m_dev, guest_channel, current_selection.m_node, current_selection.m_node_channel)) {
			if(current_selection.m_node) {
				const auto node = find_node_name(current_selection.m_node);
				machine().sound().config_remove_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, node, current_selection.m_node_channel);
				machine().sound().config_add_sound_io_channel_connection_node(current_selection.m_dev, guest_channel, node, current_selection.m_node_channel, current_selection.m_db);
			} else {
				machine().sound().config_remove_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel);
				machine().sound().config_add_sound_io_channel_connection_default(current_selection.m_dev, guest_channel, current_selection.m_node_channel, current_selection.m_db);
			}
			m_reset_selection.m_maptype = MT_INTERNAL;
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
	}

	return false;
}


bool menu_audio_mixer::set_prev_node(select_entry &current_selection)
{
	if(current_selection.m_maptype == MT_FULL) {
		const uint32_t prev_node = current_selection.m_node;
		const uint32_t next_node = find_previous_available_node(current_selection.m_dev, prev_node);
		if(next_node != 0xffffffff) {
			if(prev_node)
				machine().sound().config_remove_sound_io_connection_node(current_selection.m_dev, find_node_name(prev_node));
			else
				machine().sound().config_remove_sound_io_connection_default(current_selection.m_dev);
			if(next_node)
				machine().sound().config_add_sound_io_connection_node(current_selection.m_dev, find_node_name(next_node), current_selection.m_db);
			else
				machine().sound().config_add_sound_io_connection_default(current_selection.m_dev, current_selection.m_db);
			m_reset_selection.m_maptype = MT_INTERNAL;
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
	} else if(current_selection.m_maptype == MT_CHANNEL) {
		const uint32_t prev_node = current_selection.m_node;
		const uint32_t next_node = find_previous_available_channel_node(current_selection.m_dev, current_selection.m_guest_channel, prev_node, current_selection.m_node_channel);
		if(next_node != 0xffffffff) {
			if(prev_node)
				machine().sound().config_remove_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, find_node_name(prev_node), current_selection.m_node_channel);
			else
				machine().sound().config_remove_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel);
			if(next_node)
				machine().sound().config_add_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, find_node_name(next_node), current_selection.m_node_channel, current_selection.m_db);
			else
				machine().sound().config_add_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel, current_selection.m_db);
			m_reset_selection.m_maptype = MT_INTERNAL;
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
	}

	return false;
}


bool menu_audio_mixer::set_next_node(select_entry &current_selection)
{
	if(current_selection.m_maptype == MT_FULL) {
		const uint32_t prev_node = current_selection.m_node;
		const uint32_t next_node = find_next_available_node(current_selection.m_dev, prev_node);
		if(next_node != 0xffffffff) {
			current_selection.m_node = next_node;
			if(prev_node)
				machine().sound().config_remove_sound_io_connection_node(current_selection.m_dev, find_node_name(prev_node));
			else
				machine().sound().config_remove_sound_io_connection_default(current_selection.m_dev);
			if(next_node)
				machine().sound().config_add_sound_io_connection_node(current_selection.m_dev, find_node_name(next_node), current_selection.m_db);
			else
				machine().sound().config_add_sound_io_connection_default(current_selection.m_dev, current_selection.m_db);
			m_reset_selection.m_maptype = MT_INTERNAL;
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
	} else if(current_selection.m_maptype == MT_CHANNEL) {
		const uint32_t prev_node = current_selection.m_node;
		const uint32_t next_node = find_next_available_channel_node(current_selection.m_dev, current_selection.m_guest_channel, prev_node, current_selection.m_node_channel);
		if(next_node != 0xffffffff) {
			current_selection.m_node = next_node;
			if(prev_node)
				machine().sound().config_remove_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, find_node_name(prev_node), current_selection.m_node_channel);
			else
				machine().sound().config_remove_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel);
			if(next_node)
				machine().sound().config_add_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, find_node_name(next_node), current_selection.m_node_channel, current_selection.m_db);
			else
				machine().sound().config_add_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel, current_selection.m_db);
			m_reset_selection.m_maptype = MT_INTERNAL;
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
	}

	return false;
}


bool menu_audio_mixer::set_prev_node_channel(select_entry &current_selection)
{
	if(current_selection.m_maptype != MT_CHANNEL)
		return false;

	uint32_t node_channel_count = find_node_channel_count(current_selection.m_node, current_selection.m_dev->is_output());
	if(node_channel_count == 1)
		return false;

	uint32_t node_channel = current_selection.m_node_channel;
	for(;;) {
		if(node_channel == 0)
			node_channel = node_channel_count - 1;
		else
			node_channel--;
		if(node_channel == current_selection.m_node_channel)
			return false;
		if(channel_mapping_available(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node, node_channel)) {
			if(current_selection.m_node) {
				const auto node = find_node_name(current_selection.m_node);
				machine().sound().config_remove_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, node, current_selection.m_node_channel);
				machine().sound().config_add_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, node, node_channel, current_selection.m_db);
			} else {
				machine().sound().config_remove_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel);
				machine().sound().config_add_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, node_channel, current_selection.m_db);
			}
			m_reset_selection.m_maptype = MT_INTERNAL;
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
	}

	return false;
}


bool menu_audio_mixer::set_next_node_channel(select_entry &current_selection)
{
	if(current_selection.m_maptype != MT_CHANNEL)
		return false;

	uint32_t node_channel_count = find_node_channel_count(current_selection.m_node, current_selection.m_dev->is_output());
	if(node_channel_count == 1)
		return false;

	uint32_t node_channel = current_selection.m_node_channel;
	for(;;) {
		node_channel++;
		if(node_channel == node_channel_count)
			node_channel = 0;
		if(node_channel == current_selection.m_node_channel)
			return false;
		if(channel_mapping_available(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node, node_channel)) {
			if(current_selection.m_node) {
				const auto node = find_node_name(current_selection.m_node);
				machine().sound().config_remove_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, node, current_selection.m_node_channel);
				machine().sound().config_add_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, node, node_channel, current_selection.m_db);
			} else {
				machine().sound().config_remove_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel);
				machine().sound().config_add_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, node_channel, current_selection.m_db);
			}
			m_reset_selection.m_maptype = MT_INTERNAL;
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
	}

	return false;
}


bool menu_audio_mixer::set_route_volume(menu_item &item, select_entry &current_selection)
{
	current_selection.m_db = floorf(current_selection.m_db * 10.0f + 0.5f) / 10.0f;
	current_selection.m_db = std::clamp(current_selection.m_db, -96.0f, 12.0f);

	if(current_selection.m_maptype == MT_FULL) {
		if(current_selection.m_node == 0)
			machine().sound().config_set_volume_sound_io_connection_default(current_selection.m_dev, current_selection.m_db);
		else
			machine().sound().config_set_volume_sound_io_connection_node(current_selection.m_dev, find_node_name(current_selection.m_node), current_selection.m_db);
	} else {
		if(current_selection.m_node == 0)
			machine().sound().config_set_volume_sound_io_channel_connection_default(current_selection.m_dev, current_selection.m_guest_channel, current_selection.m_node_channel, current_selection.m_db);
		else
			machine().sound().config_set_volume_sound_io_channel_connection_node(current_selection.m_dev, current_selection.m_guest_channel, find_node_name(current_selection.m_node), current_selection.m_node_channel, current_selection.m_db);
	}

	item.set_subtext(util::string_format(_("menu-audiomix", "%1$g dB"), current_selection.m_db));
	item.set_flags(((current_selection.m_db > -96.0f) ? FLAG_LEFT_ARROW : 0) | ((current_selection.m_db < 12.0f) ? FLAG_RIGHT_ARROW : 0));
	return true;
}


//-------------------------------------------------
//  menu_audio_mixer_populate - populate the audio_mixer
//  menu
//-------------------------------------------------

void menu_audio_mixer::populate()
{
	const auto &mapping = machine().sound().get_mappings();
	const auto &info = machine().sound().get_osd_info();
	m_generation = info.m_generation;

	const auto find_node =
			[&info](uint32_t node_id) -> const osd::audio_info::node_info * {
				for(const auto &node : info.m_nodes)
					if(node.m_id == node_id)
						return &node;
				// Never happens
				return nullptr;
			};

	// Rebuild the selections list
	m_selections.clear();
	for(const auto &omap : mapping) {
		for(const auto &nmap : omap.m_node_mappings)
			m_selections.emplace_back(select_entry{ MT_FULL, omap.m_dev, 0, nmap.m_is_system_default ? 0 : nmap.m_node, 0, nmap.m_db });
		for(const auto &cmap : omap.m_channel_mappings)
			m_selections.emplace_back(select_entry{ MT_CHANNEL, omap.m_dev, cmap.m_guest_channel, cmap.m_is_system_default ? 0 : cmap.m_node, cmap.m_node_channel, cmap.m_db });
		if(omap.m_node_mappings.empty() && omap.m_channel_mappings.empty())
			m_selections.emplace_back(select_entry{ MT_NONE, omap.m_dev, 0, 0, 0, 0 });
	}

	// If there's nothing, get out of there
	if(m_selections.empty()) {
		item_append(_("menu-audiomix", "[mappings not initialized]"), FLAG_DISABLE, nullptr);
		item_append(menu_item_type::SEPARATOR);
		return;
	}

	// Find the line of the current selection, if any.
	// Otherwise default to the first line
	uint32_t cursel_line = 0xffffffff;
	for(uint32_t i = 0; i != m_selections.size(); i++) {
		if(m_reset_selection == m_selections[i]) {
			cursel_line = i;
			break;
		}
	}

	if(cursel_line == 0xffffffff) {
		for(uint32_t i = 0; i != m_selections.size(); i++) {
			if(m_reset_selection.m_dev == m_selections[i].m_dev) {
				if (m_selections[i].m_maptype != MT_NONE)
					m_reset_item = ITM_GUEST_CHANNEL;
				else
					m_reset_item = ITM_ADD_FULL;
				cursel_line = i;
				break;
			}
		}
	}

	if(cursel_line == 0xffffffff)
		cursel_line = 0;

	if(!m_reset_item)
		m_reset_item = (m_selections[0].m_maptype == MT_NONE) ? ITM_ADD_FULL : ITM_GUEST_CHANNEL;

	else if(m_reset_selection.m_maptype == MT_INTERNAL)
		cursel_line = 0xffffffff;

	if((cursel_line < m_selections.size()) && (m_selections[cursel_line].m_maptype == MT_FULL)) {
		if(m_reset_item == ITM_NODE_CHANNEL)
			m_reset_item = ITM_NODE;
	}

	// (Re)build the menu
	uint32_t cursel = 0;

	auto const add_routes = [this, &cursel] ()
			{
				item_append(
						_("menu-audiomix", "Add new full route"),
						0,
						reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_ADD_FULL));
				item_append(
						_("menu-audiomix", "Add new channel route"),
						0,
						reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_ADD_CHANNEL));
			};

	for(const auto &omap : mapping) {
		item_append(omap.m_dev->tag(), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
		bool first = true;
		for(const auto &nmap : omap.m_node_mappings) {
			const auto &node = find_node(nmap.m_node);

			if(first) {
				first = false;
				add_routes();
			}
			item_append(menu_item_type::SEPARATOR);

			item_append(
					omap.m_dev->is_output() ? _("menu-audiomix", "Output") : _("menu-audiomix", "Input"),
					_("menu-audiomix", "[all]"),
					0,
					reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_GUEST_CHANNEL));
			item_append(
					_("menu-audiomix", "Device"),
					nmap.m_is_system_default ?
						_("menu-audiomix", "[default]") :
						(omap.m_dev->is_output() || !node->m_sinks) ?
							node->m_display_name :
							util::string_format(_("menu-audiomix", "Monitor of %1$s"), node->m_display_name),
					FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW,
					reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_NODE));
			item_append(
					_("menu-audiomix", "Volume"),
					util::string_format(_("menu-audiomix", "%1$g dB"), nmap.m_db),
					((nmap.m_db > -96.0f) ? FLAG_LEFT_ARROW : 0) | ((nmap.m_db < 12.0f) ? FLAG_RIGHT_ARROW : 0),
					reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_DB));
			item_append(
					_("menu-audiomix", "Remove this route"),
					0,
					reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_REMOVE));

			++cursel;
		}
		for(const auto &cmap : omap.m_channel_mappings) {
			const auto &node = find_node(cmap.m_node);

			if(first) {
				first = false;
				add_routes();
			}
			item_append(menu_item_type::SEPARATOR);

			item_append(
					omap.m_dev->is_output() ? _("menu-audiomix", "Output") : _("menu-audiomix", "Input"),
					omap.m_dev->get_position(cmap.m_guest_channel).name(),
					FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW,
					reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_GUEST_CHANNEL));
			item_append(
					_("menu-audiomix", "Device"),
					cmap.m_is_system_default ?
						_("menu-audiomix", "[default]") :
						(omap.m_dev->is_output() || !node->m_sinks) ?
							node->m_display_name :
							util::string_format(_("menu-audiomix", "Monitor of %1$s"), node->m_display_name),
					FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW,
					reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_NODE));
			item_append(
					_("menu-audiomix", "Channel"),
					node->m_port_names[cmap.m_node_channel],
					FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW,
					reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_NODE_CHANNEL));
			item_append(
					_("menu-audiomix", "Volume"),
					util::string_format(_("menu-audiomix", "%1$g dB"), cmap.m_db),
					((cmap.m_db > -96.0f) ? FLAG_LEFT_ARROW : 0) | ((cmap.m_db < 12.0f) ? FLAG_RIGHT_ARROW : 0),
					reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_DB));
			item_append(
					_("menu-audiomix", "Remove this route"),
					0,
					reinterpret_cast<void *>(((cursel + 1) << 3) | ITM_REMOVE));

			++cursel;
		}
		if(omap.m_node_mappings.empty() && omap.m_channel_mappings.empty()) {
			add_routes();
			++cursel;
		}
	}

	item_append(menu_item_type::SEPARATOR);

	if(cursel_line != 0xffffffff)
		set_selection(reinterpret_cast<void *>(((cursel_line + 1) << 3) | m_reset_item));
}


//-------------------------------------------------
//  menu_activated - handle menu gaining focus
//-------------------------------------------------

void menu_audio_mixer::menu_activated()
{
	// scripts or the other form of the menu could have changed something in the mean time
	const auto selection_ref = uintptr_t(get_selection_ref());
	const uint32_t selection_num = selection_ref >> 3;
	if((1 <= selection_num) && (m_selections.size() >= selection_num))
	{
		m_reset_selection = m_selections[selection_num - 1];
		m_reset_item = BIT(selection_ref, 0, 3);
		reset(reset_options::REMEMBER_POSITION);
	}
	else
	{
		m_reset_selection.m_maptype = MT_INTERNAL;
		reset(reset_options::REMEMBER_REF);
	}
}


//-------------------------------------------------
//  menu_deactivated - handle menu losing focus
//-------------------------------------------------

void menu_audio_mixer::menu_deactivated()
{
}

uint32_t menu_audio_mixer::find_node_index(uint32_t node) const
{
	const auto &info = machine().sound().get_osd_info();
	for(uint32_t i = 0; i != info.m_nodes.size(); i++)
		if(info.m_nodes[i].m_id == node)
			return i;
	// Can't happen in theory
	return 0xffffffff;
}

std::string menu_audio_mixer::find_node_name(uint32_t node) const
{
	const auto &info = machine().sound().get_osd_info();
	for(uint32_t i = 0; i != info.m_nodes.size(); i++)
		if(info.m_nodes[i].m_id == node)
			return info.m_nodes[i].name();
	// Can't happen in theory
	return "";
}

uint32_t menu_audio_mixer::find_node_channel_count(uint32_t node, bool is_output) const
{
	const auto &info = machine().sound().get_osd_info();
	if(!node)
		node = info.m_default_sink;
	for(uint32_t i = 0; i != info.m_nodes.size(); i++)
		if(info.m_nodes[i].m_id == node)
			return is_output ? info.m_nodes[i].m_sinks : info.m_nodes[i].m_sources;
	// Can't happen in theory
	return 0;
}

uint32_t menu_audio_mixer::find_next_sink_node_index(uint32_t index) const
{
	if(index == 0xffffffff)
		return index;

	const auto &info = machine().sound().get_osd_info();
	for(uint32_t idx = index + 1; idx != info.m_nodes.size(); idx++)
		if(info.m_nodes[idx].m_sinks)
			return idx;
	return 0xffffffff;
}

uint32_t menu_audio_mixer::find_next_source_node_index(uint32_t index) const
{
	if(index == 0xffffffff)
		return index;

	const auto &info = machine().sound().get_osd_info();
	for(uint32_t idx = index + 1; idx != info.m_nodes.size(); idx++)
		if(info.m_nodes[idx].m_sources)
			return idx;
	return 0xffffffff;
}

uint32_t menu_audio_mixer::find_previous_sink_node_index(uint32_t index) const
{
	if(index == 0xffffffff)
		return index;

	const auto &info = machine().sound().get_osd_info();
	for(uint32_t idx = index - 1; idx != 0xffffffff; idx--)
		if(info.m_nodes[idx].m_sinks)
			return idx;
	return 0xffffffff;
}

uint32_t menu_audio_mixer::find_previous_source_node_index(uint32_t index) const
{
	if(index == 0xffffffff)
		return index;

	const auto &info = machine().sound().get_osd_info();
	for(uint32_t idx = index - 1; idx != 0xffffffff; idx--)
		if(info.m_nodes[idx].m_sources)
			return idx;
	return 0xffffffff;
}

uint32_t menu_audio_mixer::find_first_sink_node_index() const
{
	const auto &info = machine().sound().get_osd_info();
	for(uint32_t index = 0; index != info.m_nodes.size(); index ++)
		if(info.m_nodes[index].m_sinks)
			return index;
	return 0xffffffff;
}

uint32_t menu_audio_mixer::find_first_source_node_index() const
{
	const auto &info = machine().sound().get_osd_info();
	for(uint32_t index = 0; index != info.m_nodes.size(); index ++)
		if(info.m_nodes[index].m_sources)
			return index;
	return 0xffffffff;
}

uint32_t menu_audio_mixer::find_last_sink_node_index() const
{
	const auto &info = machine().sound().get_osd_info();
	for(uint32_t index = info.m_nodes.size() - 1; index != 0xffffffff; index --)
		if(info.m_nodes[index].m_sinks)
			return index;
	return 0xffffffff;
}

uint32_t menu_audio_mixer::find_last_source_node_index() const
{
	const auto &info = machine().sound().get_osd_info();
	for(uint32_t index = info.m_nodes.size() - 1; index != 0xffffffff; index --)
		if(info.m_nodes[index].m_sources)
			return index;
	return 0xffffffff;
}

bool menu_audio_mixer::full_mapping_available(sound_io_device *dev, uint32_t node) const
{
	if(dev->is_output() && !node && machine().sound().get_osd_info().m_default_sink == 0)
		return false;
	if(!dev->is_output() && !node && machine().sound().get_osd_info().m_default_source == 0)
		return false;

	const auto &mapping = machine().sound().get_mappings();
	for(const auto &omap : mapping)
		if(omap.m_dev == dev) {
			for(const auto &nmap : omap.m_node_mappings)
				if((node != 0 && nmap.m_node == node && !nmap.m_is_system_default) || (node == 0 && nmap.m_is_system_default))
					return false;
			return true;
		}
	return true;
}

bool menu_audio_mixer::channel_mapping_available(sound_io_device *dev, uint32_t guest_channel, uint32_t node, uint32_t node_channel) const
{
	if(dev->is_output() && !node && machine().sound().get_osd_info().m_default_sink == 0)
		return false;
	if(!dev->is_output() && !node && machine().sound().get_osd_info().m_default_source == 0)
		return false;

	const auto &mapping = machine().sound().get_mappings();
	for(const auto &omap : mapping)
		if(omap.m_dev == dev) {
			for(const auto &cmap : omap.m_channel_mappings)
				if(cmap.m_guest_channel == guest_channel &&
				   ((node != 0 && cmap.m_node == node && !cmap.m_is_system_default) || (node == 0 && cmap.m_is_system_default))
				   && cmap.m_node_channel == node_channel)
					return false;
			return true;
		}
	return true;
}

uint32_t menu_audio_mixer::find_next_available_node(sound_io_device *dev, uint32_t node) const
{
	const auto &info = machine().sound().get_osd_info();

	if(dev->is_output()) {
		if(node == 0) {
			uint32_t index = find_first_sink_node_index();
			while(index != 0xffffffff && !full_mapping_available(dev, info.m_nodes[index].m_id))
				index = find_next_sink_node_index(index);
			return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
		}

		uint32_t index = find_node_index(node);
		while(index != 0xffffffff) {
			index = find_next_sink_node_index(index);
			if(index != 0xffffffff && full_mapping_available(dev, info.m_nodes[index].m_id))
				return info.m_nodes[index].m_id;
		}

		if(info.m_default_sink != 0 && full_mapping_available(dev, 0))
			return 0;

		index = find_first_sink_node_index();
		while(index != 0xffffffff && !full_mapping_available(dev, info.m_nodes[index].m_id))
			index = find_next_sink_node_index(index);
		return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
	} else {
		if(node == 0) {
			uint32_t index = find_first_source_node_index();
			while(index != 0xffffffff && !full_mapping_available(dev, info.m_nodes[index].m_id))
				index = find_next_source_node_index(index);
			return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
		}

		uint32_t index = find_node_index(node);
		while(index != 0xffffffff) {
			index = find_next_source_node_index(index);
			if(index != 0xffffffff && full_mapping_available(dev, info.m_nodes[index].m_id))
				return info.m_nodes[index].m_id;
		}

		if(info.m_default_source != 0 && full_mapping_available(dev, 0))
			return 0;

		index = find_first_source_node_index();
		while(index != 0xffffffff && !full_mapping_available(dev, info.m_nodes[index].m_id))
			index = find_next_source_node_index(index);
		return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
	}
}

uint32_t menu_audio_mixer::find_previous_available_node(sound_io_device *dev, uint32_t node) const
{
	const auto &info = machine().sound().get_osd_info();

	if(dev->is_output()) {
		if(node == 0) {
			uint32_t index = find_last_sink_node_index();
			while(index != 0xffffffff && !full_mapping_available(dev, info.m_nodes[index].m_id))
				index = find_previous_sink_node_index(index);
			return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
		}

		uint32_t index = find_node_index(node);
		while(index != 0xffffffff) {
			index = find_previous_sink_node_index(index);
			if(index != 0xffffffff && full_mapping_available(dev, info.m_nodes[index].m_id))
				return info.m_nodes[index].m_id;
		}

		if(info.m_default_sink != 0 && full_mapping_available(dev, 0))
			return 0;

		index = find_last_sink_node_index();
		while(index != 0xffffffff && !full_mapping_available(dev, info.m_nodes[index].m_id))
			index = find_previous_sink_node_index(index);
		return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;

	} else {
		if(node == 0) {
			uint32_t index = find_last_source_node_index();
			while(index != 0xffffffff && !full_mapping_available(dev, info.m_nodes[index].m_id))
				index = find_previous_source_node_index(index);
			return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
		}

		uint32_t index = find_node_index(node);
		while(index != 0xffffffff) {
			index = find_previous_source_node_index(index);
			if(index != 0xffffffff && full_mapping_available(dev, info.m_nodes[index].m_id))
				return info.m_nodes[index].m_id;
		}

		if(info.m_default_source != 0 && full_mapping_available(dev, 0))
			return 0;

		index = find_last_source_node_index();
		while(index != 0xffffffff && !full_mapping_available(dev, info.m_nodes[index].m_id))
			index = find_previous_source_node_index(index);
		return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
	}
}

uint32_t menu_audio_mixer::find_next_available_channel_node(sound_io_device *dev, uint32_t guest_channel, uint32_t node, uint32_t node_channel) const
{
	const auto &info = machine().sound().get_osd_info();

	if(dev->is_output()) {
		if(node == 0) {
			uint32_t index = find_first_sink_node_index();
			while(index != 0xffffffff && !channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
				index = find_next_sink_node_index(index);
			return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
		}

		uint32_t index = find_node_index(node);
		while(index != 0xffffffff) {
			index = find_next_sink_node_index(index);
			if(index != 0xffffffff && channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
				return info.m_nodes[index].m_id;
		}

		if(dev->is_output() && info.m_default_sink != 0 && channel_mapping_available(dev, guest_channel, 0, node_channel))
			return 0;
		if(!dev->is_output() && info.m_default_source != 0 && channel_mapping_available(dev, guest_channel, 0, node_channel))
			return 0;

		index = find_first_sink_node_index();
		while(index != 0xffffffff && !channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
			index = find_next_sink_node_index(index);
		return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;

	} else {
		if(node == 0) {
			uint32_t index = find_first_source_node_index();
			while(index != 0xffffffff && !channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
				index = find_next_source_node_index(index);
			return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
		}

		uint32_t index = find_node_index(node);
		while(index != 0xffffffff) {
			index = find_next_source_node_index(index);
			if(index != 0xffffffff && channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
				return info.m_nodes[index].m_id;
		}

		if(dev->is_output() && info.m_default_source != 0 && channel_mapping_available(dev, guest_channel, 0, node_channel))
			return 0;
		if(!dev->is_output() && info.m_default_source != 0 && channel_mapping_available(dev, guest_channel, 0, node_channel))
			return 0;

		index = find_first_source_node_index();
		while(index != 0xffffffff && !channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
			index = find_next_source_node_index(index);
		return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
	}
}

uint32_t menu_audio_mixer::find_previous_available_channel_node(sound_io_device *dev, uint32_t guest_channel, uint32_t node, uint32_t node_channel) const
{
	const auto &info = machine().sound().get_osd_info();

	if(dev->is_output()) {
		if(node == 0) {
			uint32_t index = find_last_sink_node_index();
			while(index != 0xffffffff && !channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
				index = find_previous_sink_node_index(index);
			return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
		}

		uint32_t index = find_node_index(node);
		while(index != 0xffffffff) {
			index = find_previous_sink_node_index(index);
			if(index != 0xffffffff && channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
				return info.m_nodes[index].m_id;
		}

		if(dev->is_output() && info.m_default_sink != 0 && channel_mapping_available(dev, guest_channel, 0, node_channel))
			return 0;
		if(!dev->is_output() && info.m_default_source != 0 && channel_mapping_available(dev, guest_channel, 0, node_channel))
			return 0;

		index = find_last_sink_node_index();
		while(index != 0xffffffff && !channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
			index = find_previous_sink_node_index(index);
		return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;

	} else {
		if(node == 0) {
			uint32_t index = find_last_source_node_index();
			while(index != 0xffffffff && !channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
				index = find_previous_source_node_index(index);
			return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
		}

		uint32_t index = find_node_index(node);
		while(index != 0xffffffff) {
			index = find_previous_source_node_index(index);
			if(index != 0xffffffff && channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
				return info.m_nodes[index].m_id;
		}

		if(dev->is_output() && info.m_default_source != 0 && channel_mapping_available(dev, guest_channel, 0, node_channel))
			return 0;
		if(!dev->is_output() && info.m_default_source != 0 && channel_mapping_available(dev, guest_channel, 0, node_channel))
			return 0;

		index = find_last_source_node_index();
		while(index != 0xffffffff && !channel_mapping_available(dev, guest_channel, info.m_nodes[index].m_id, node_channel))
			index = find_previous_source_node_index(index);
		return index == 0xffffffff ? 0xffffffff : info.m_nodes[index].m_id;
	}
}

} // namespace ui

