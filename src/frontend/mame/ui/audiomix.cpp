// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/audiomix.cpp

    Audio mixing/mapping control

*********************************************************************/

#include "emu.h"
#include "ui/audiomix.h"

#include "ui/ui.h"

#include "osdepend.h"
#include "speaker.h"

namespace ui {

menu_audio_mixer::menu_audio_mixer(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
{
	set_heading(_("Audio Mixer"));
	m_generation = 0;
	m_current_selection.m_maptype = MT_UNDEFINED;
	m_current_selection.m_dev = nullptr;
	m_current_selection.m_guest_channel = 0;
	m_current_selection.m_node = 0;
	m_current_selection.m_node_channel = 0;
	m_current_group = GRP_NODE;

	set_process_flags(PROCESS_LR_ALWAYS);
}

menu_audio_mixer::~menu_audio_mixer()
{
}

bool menu_audio_mixer::handle(event const *ev)
{
	if(!ev) {
		if(m_generation != machine().sound().get_osd_info().m_generation) {
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
		return false;
	}

	bool const alt_pressed = machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT);
	bool const ctrl_pressed = machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL);
	bool const shift_pressed = machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT);

	switch(ev->iptkey) {
	case IPT_UI_MIXER_ADD_FULL:
		if(m_current_selection.m_maptype == MT_INTERNAL)
			return false;

		if(full_mapping_available(m_current_selection.m_dev, 0)) {
			m_current_selection.m_node = 0;
			machine().sound().config_add_sound_io_connection_default(m_current_selection.m_dev, 0.0);

		} else {
			uint32_t node = find_next_available_node(m_current_selection.m_dev, 0);
			if(node == 0xffffffff)
				return false;
			m_current_selection.m_node = node;
			machine().sound().config_add_sound_io_connection_node(m_current_selection.m_dev, find_node_name(node), 0.0);
		}

		m_current_selection.m_maptype = MT_FULL;
		m_current_selection.m_guest_channel = 0;
		m_current_selection.m_node_channel = 0;
		m_current_selection.m_db = 0.0;
		m_generation --;
		return true;

	case IPT_UI_MIXER_ADD_CHANNEL: {
		if(m_current_selection.m_maptype == MT_INTERNAL)
			return false;

		// Find a possible triplet, any triplet
		const auto &info = machine().sound().get_osd_info();
		u32 guest_channel;
		u32 node_index, node_id;
		u32 node_channel;
		u32 default_osd_id = m_current_selection.m_dev->is_output() ? info.m_default_sink : info.m_default_source;
		for(node_index = default_osd_id == 0 ? 0 : 0xffffffff; node_index != info.m_nodes.size(); node_index++) {
			node_id = node_index == 0xffffffff ? 0 : info.m_nodes[node_index].m_id;
			u32 guest_channel_count = m_current_selection.m_dev->is_output() ? m_current_selection.m_dev->inputs() : m_current_selection.m_dev->outputs();
			u32 node_channel_count = 0;
			if(node_index == 0xffffffff) {
				for(u32 i = 0; i != info.m_nodes.size(); i++)
					if(info.m_nodes[i].m_id == default_osd_id) {
						node_channel_count = m_current_selection.m_dev->is_output() ? info.m_nodes[i].m_sinks : info.m_nodes[i].m_sources;
						break;
					}
			} else
				node_channel_count = m_current_selection.m_dev->is_output() ? info.m_nodes[node_index].m_sinks : info.m_nodes[node_index].m_sources;
			for(guest_channel = 0; guest_channel != guest_channel_count; guest_channel ++)
				for(node_channel = 0; node_channel != node_channel_count; node_channel ++)
					if(channel_mapping_available(m_current_selection.m_dev, guest_channel, node_id, node_channel))
						goto found;
		}
		return false;

	found:
		if(node_id)
			machine().sound().config_add_sound_io_channel_connection_node(m_current_selection.m_dev, guest_channel, info.m_nodes[node_index].name(), node_channel, 0.0);
		else
			machine().sound().config_add_sound_io_channel_connection_default(m_current_selection.m_dev, guest_channel, node_channel, 0.0);
		m_current_selection.m_maptype = MT_CHANNEL;
		m_current_selection.m_guest_channel = guest_channel;
		m_current_selection.m_node = node_id;
		m_current_selection.m_node_channel = node_channel;
		m_current_selection.m_db = 0.0;
		m_generation --;
		return true;
	}

	case IPT_UI_CLEAR: {
		if(m_current_selection.m_maptype == MT_NONE || m_current_selection.m_maptype == MT_INTERNAL)
			return false;

		if(m_current_selection.m_maptype == MT_FULL) {
			if(m_current_selection.m_node == 0)
				machine().sound().config_remove_sound_io_connection_default(m_current_selection.m_dev);
			else
				machine().sound().config_remove_sound_io_connection_node(m_current_selection.m_dev, find_node_name(m_current_selection.m_node));
		} else {
			if(m_current_selection.m_node == 0)
				machine().sound().config_remove_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel);
			else
				machine().sound().config_remove_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, find_node_name(m_current_selection.m_node), m_current_selection.m_node_channel);
		}

		// Find where the selection was
		uint32_t cursel_index = 0;
		for(uint32_t i = 0; i != m_selections.size(); i++)
			if(m_selections[i] == m_current_selection) {
				cursel_index = i;
				break;
			}

		// If the next item exists and is the same speaker, go there (visually, the cursor stays on the same line)
		// Otherwise if the previous item exists and is the same speaker, go there (visually, the cursor goes up once)
		// Otherwise create a MT_NONE, because one is going to appear at the same place

		if(cursel_index + 1 < m_selections.size() && m_selections[cursel_index+1].m_dev == m_current_selection.m_dev)
			m_current_selection = m_selections[cursel_index+1];
		else if(cursel_index != 0 && m_selections[cursel_index-1].m_dev == m_current_selection.m_dev)
			m_current_selection = m_selections[cursel_index-1];
		else {
			m_current_selection.m_maptype = MT_NONE;
			m_current_selection.m_guest_channel = 0;
			m_current_selection.m_node = 0;
			m_current_selection.m_node_channel = 0;
			m_current_selection.m_db = 0.0;
		}

		m_generation --;
		return true;
	}

	case IPT_UI_UP:
	case IPT_UI_DOWN:
		if(!ev->itemref) {
			m_current_selection.m_maptype = MT_INTERNAL;
			m_generation --;
			return true;
		}

		m_current_selection = *(select_entry *)(ev->itemref);
		if(m_current_selection.m_maptype == MT_FULL) {
			if(m_current_group == GRP_GUEST_CHANNEL || m_current_group == GRP_NODE_CHANNEL)
				m_current_group = GRP_NODE;
		}
		m_generation --;
		return true;

	case IPT_UI_NEXT_GROUP:
		if(m_current_selection.m_maptype == MT_NONE || m_current_selection.m_maptype == MT_INTERNAL)
			return false;

		if(m_current_selection.m_maptype == MT_FULL) {
			if(m_current_group == GRP_NODE)
				m_current_group = GRP_DB;
			else
				m_current_group = GRP_NODE;

		} else if(m_current_selection.m_maptype == MT_CHANNEL) {
			if(m_current_group == GRP_NODE)
				m_current_group = GRP_NODE_CHANNEL;
			else if(m_current_group == GRP_NODE_CHANNEL)
				m_current_group = GRP_DB;
			else if(m_current_group == GRP_DB)
				m_current_group = GRP_GUEST_CHANNEL;
			else
				m_current_group = GRP_NODE;
		}
		m_generation --;
		return true;

	case IPT_UI_PREV_GROUP:
		if(m_current_selection.m_maptype == MT_NONE || m_current_selection.m_maptype == MT_INTERNAL)
			return false;

		if(m_current_selection.m_maptype == MT_FULL) {
			if(m_current_group == GRP_NODE)
				m_current_group = GRP_DB;
			else
				m_current_group = GRP_NODE;

		} else if(m_current_selection.m_maptype == MT_CHANNEL) {
			if(m_current_group == GRP_NODE)
				m_current_group = GRP_GUEST_CHANNEL;
			else if(m_current_group == GRP_GUEST_CHANNEL)
				m_current_group = GRP_DB;
			else if(m_current_group == GRP_DB)
				m_current_group = GRP_NODE_CHANNEL;
			else
				m_current_group = GRP_NODE;
		}
		m_generation --;
		return true;

	case IPT_UI_LEFT: {
		if(m_current_selection.m_maptype == MT_NONE || m_current_selection.m_maptype == MT_INTERNAL)
			return false;

		switch(m_current_group) {
		case GRP_NODE: {
			if(m_current_selection.m_maptype == MT_FULL) {
				uint32_t prev_node = m_current_selection.m_node;
				uint32_t next_node = find_previous_available_node(m_current_selection.m_dev, prev_node);
				if(next_node != 0xffffffff) {
					m_current_selection.m_node = next_node;
					if(prev_node)
						machine().sound().config_remove_sound_io_connection_node(m_current_selection.m_dev, find_node_name(prev_node));
					else
						machine().sound().config_remove_sound_io_connection_default(m_current_selection.m_dev);
					if(next_node)
						machine().sound().config_add_sound_io_connection_node(m_current_selection.m_dev, find_node_name(next_node), m_current_selection.m_db);
					else
						machine().sound().config_add_sound_io_connection_default(m_current_selection.m_dev, m_current_selection.m_db);
					m_generation --;
					return true;
				}
			} else if(m_current_selection.m_maptype == MT_CHANNEL) {
				uint32_t prev_node = m_current_selection.m_node;
				uint32_t next_node = find_previous_available_channel_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, prev_node, m_current_selection.m_node_channel);
				if(next_node != 0xffffffff) {
					m_current_selection.m_node = next_node;
					if(prev_node)
						machine().sound().config_remove_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, find_node_name(prev_node), m_current_selection.m_node_channel);
					else
						machine().sound().config_remove_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel);
					if(next_node)
						machine().sound().config_add_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, find_node_name(next_node), m_current_selection.m_node_channel, m_current_selection.m_db);
					else
						machine().sound().config_add_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel, m_current_selection.m_db);
					m_generation --;
					return true;
				}
			}
			break;
		}

		case GRP_DB: {
			if(shift_pressed)
				m_current_selection.m_db -= 0.1f;
			else if(ctrl_pressed)
				m_current_selection.m_db -= 10.0f;
			else if(alt_pressed) {
				if(m_current_selection.m_db > 0.0f)
					m_current_selection.m_db = 0.0f;
				else
					m_current_selection.m_db = -96.0f;
			}
			else
				m_current_selection.m_db -= 1.0f;

			m_current_selection.m_db = floorf(m_current_selection.m_db * 10.0f + 0.5f) / 10.0f;
			m_current_selection.m_db = std::clamp(m_current_selection.m_db, -96.0f, 12.0f);

			if(m_current_selection.m_maptype == MT_FULL) {
				if(m_current_selection.m_node == 0)
					machine().sound().config_set_volume_sound_io_connection_default(m_current_selection.m_dev, m_current_selection.m_db);
				else
					machine().sound().config_set_volume_sound_io_connection_node(m_current_selection.m_dev, find_node_name(m_current_selection.m_node), m_current_selection.m_db);
			} else {
				if(m_current_selection.m_node == 0)
					machine().sound().config_set_volume_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel, m_current_selection.m_db);
				else
					machine().sound().config_set_volume_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, find_node_name(m_current_selection.m_node), m_current_selection.m_node_channel, m_current_selection.m_db);
			}
			m_generation --;
			return true;
		}

		case GRP_GUEST_CHANNEL: {
			if(m_current_selection.m_maptype != MT_CHANNEL)
				return false;

			u32 guest_channel_count = m_current_selection.m_dev->is_output() ? m_current_selection.m_dev->inputs() : m_current_selection.m_dev->outputs();
			if(guest_channel_count == 1)
				return false;
			u32 guest_channel = m_current_selection.m_guest_channel;
			for(;;) {
				if(guest_channel == 0)
					guest_channel = guest_channel_count - 1;
				else
					guest_channel --;
				if(guest_channel == m_current_selection.m_guest_channel)
					return false;
				if(channel_mapping_available(m_current_selection.m_dev, guest_channel, m_current_selection.m_node, m_current_selection.m_node_channel)) {
					if(m_current_selection.m_node) {
						std::string node = find_node_name(m_current_selection.m_node);
						machine().sound().config_remove_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, node, m_current_selection.m_node_channel);
						machine().sound().config_add_sound_io_channel_connection_node(m_current_selection.m_dev, guest_channel, node, m_current_selection.m_node_channel, m_current_selection.m_db);
					} else {
						machine().sound().config_remove_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel);
						machine().sound().config_add_sound_io_channel_connection_default(m_current_selection.m_dev, guest_channel, m_current_selection.m_node_channel, m_current_selection.m_db);
					}
					m_current_selection.m_guest_channel = guest_channel;
					m_generation --;
					return true;
				}
			}
			break;
		}

		case GRP_NODE_CHANNEL: {
			if(m_current_selection.m_maptype != MT_CHANNEL)
				return false;

			u32 node_channel_count = find_node_channel_count(m_current_selection.m_node, m_current_selection.m_dev->is_output());
			if(node_channel_count == 1)
				return false;
			u32 node_channel = m_current_selection.m_node_channel;
			for(;;) {
				if(node_channel == 0)
					node_channel = node_channel_count - 1;
				else
					node_channel --;
				if(node_channel == m_current_selection.m_node_channel)
					return false;
				if(channel_mapping_available(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node, node_channel)) {
					if(m_current_selection.m_node) {
						std::string node = find_node_name(m_current_selection.m_node);
						machine().sound().config_remove_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, node, m_current_selection.m_node_channel);
						machine().sound().config_add_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, node, node_channel, m_current_selection.m_db);
					} else {
						machine().sound().config_remove_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel);
						machine().sound().config_add_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, node_channel, m_current_selection.m_db);
					}
					m_current_selection.m_node_channel = node_channel;
					m_generation --;
					return true;
				}
			}
			break;
		}
		}
		break;
	}

	case IPT_UI_RIGHT: {
		if(m_current_selection.m_maptype == MT_NONE || m_current_selection.m_maptype == MT_INTERNAL)
			return false;

		switch(m_current_group) {
		case GRP_NODE: {
			if(m_current_selection.m_maptype == MT_FULL) {
				uint32_t prev_node = m_current_selection.m_node;
				uint32_t next_node = find_next_available_node(m_current_selection.m_dev, prev_node);
				if(next_node != 0xffffffff) {
					m_current_selection.m_node = next_node;
					if(prev_node)
						machine().sound().config_remove_sound_io_connection_node(m_current_selection.m_dev, find_node_name(prev_node));
					else
						machine().sound().config_remove_sound_io_connection_default(m_current_selection.m_dev);
					if(next_node)
						machine().sound().config_add_sound_io_connection_node(m_current_selection.m_dev, find_node_name(next_node), m_current_selection.m_db);
					else
						machine().sound().config_add_sound_io_connection_default(m_current_selection.m_dev, m_current_selection.m_db);
					m_generation --;
					return true;
				}
			} else if(m_current_selection.m_maptype == MT_CHANNEL) {
				uint32_t prev_node = m_current_selection.m_node;
				uint32_t next_node = find_next_available_channel_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, prev_node, m_current_selection.m_node_channel);
				if(next_node != 0xffffffff) {
					m_current_selection.m_node = next_node;
					if(prev_node)
						machine().sound().config_remove_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, find_node_name(prev_node), m_current_selection.m_node_channel);
					else
						machine().sound().config_remove_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel);
					if(next_node)
						machine().sound().config_add_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, find_node_name(next_node), m_current_selection.m_node_channel, m_current_selection.m_db);
					else
						machine().sound().config_add_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel, m_current_selection.m_db);
					m_generation --;
					return true;
				}
			}
			break;
		}

		case GRP_DB: {
			if(shift_pressed)
				m_current_selection.m_db += 0.1f;
			else if(ctrl_pressed)
				m_current_selection.m_db += 10.0f;
			else if(alt_pressed) {
				if(m_current_selection.m_db < 0.0f)
					m_current_selection.m_db = 0.0f;
				else
					m_current_selection.m_db = 12.0f;
			}
			else
				m_current_selection.m_db += 1.0f;

			m_current_selection.m_db = floorf(m_current_selection.m_db * 10.0f + 0.5f) / 10.0f;
			m_current_selection.m_db = std::clamp(m_current_selection.m_db, -96.0f, 12.0f);

			if(m_current_selection.m_maptype == MT_FULL) {
				if(m_current_selection.m_node == 0)
					machine().sound().config_set_volume_sound_io_connection_default(m_current_selection.m_dev, m_current_selection.m_db);
				else
					machine().sound().config_set_volume_sound_io_connection_node(m_current_selection.m_dev, find_node_name(m_current_selection.m_node), m_current_selection.m_db);
			} else {
				if(m_current_selection.m_node == 0)
					machine().sound().config_set_volume_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel, m_current_selection.m_db);
				else
					machine().sound().config_set_volume_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, find_node_name(m_current_selection.m_node), m_current_selection.m_node_channel, m_current_selection.m_db);
			}
			m_generation --;
			return true;
		}

		case GRP_GUEST_CHANNEL: {
			if(m_current_selection.m_maptype != MT_CHANNEL)
				return false;

			u32 guest_channel_count = m_current_selection.m_dev->is_output() ? m_current_selection.m_dev->inputs() : m_current_selection.m_dev->outputs();
			if(guest_channel_count == 1)
				return false;
			u32 guest_channel = m_current_selection.m_guest_channel;
			for(;;) {
				guest_channel ++;
				if(guest_channel == guest_channel_count)
					guest_channel = 0;
				if(guest_channel == m_current_selection.m_guest_channel)
					return false;
				if(channel_mapping_available(m_current_selection.m_dev, guest_channel, m_current_selection.m_node, m_current_selection.m_node_channel)) {
					if(m_current_selection.m_node) {
						std::string node = find_node_name(m_current_selection.m_node);
						machine().sound().config_remove_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, node, m_current_selection.m_node_channel);
						machine().sound().config_add_sound_io_channel_connection_node(m_current_selection.m_dev, guest_channel, node, m_current_selection.m_node_channel, m_current_selection.m_db);
					} else {
						machine().sound().config_remove_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel);
						machine().sound().config_add_sound_io_channel_connection_default(m_current_selection.m_dev, guest_channel, m_current_selection.m_node_channel, m_current_selection.m_db);
					}
					m_current_selection.m_guest_channel = guest_channel;
					m_generation --;
					return true;
				}
			}
			break;
		}

		case GRP_NODE_CHANNEL: {
			if(m_current_selection.m_maptype != MT_CHANNEL)
				return false;

			u32 node_channel_count = find_node_channel_count(m_current_selection.m_node, m_current_selection.m_dev->is_output());
			if(node_channel_count == 1)
				return false;
			u32 node_channel = m_current_selection.m_node_channel;
			for(;;) {
				node_channel ++;
				if(node_channel == node_channel_count)
					node_channel = 0;
				if(node_channel == m_current_selection.m_node_channel)
					return false;
				if(channel_mapping_available(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node, node_channel)) {
					if(m_current_selection.m_node) {
						std::string node = find_node_name(m_current_selection.m_node);
						machine().sound().config_remove_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, node, m_current_selection.m_node_channel);
						machine().sound().config_add_sound_io_channel_connection_node(m_current_selection.m_dev, m_current_selection.m_guest_channel, node, node_channel, m_current_selection.m_db);
					} else {
						machine().sound().config_remove_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, m_current_selection.m_node_channel);
						machine().sound().config_add_sound_io_channel_connection_default(m_current_selection.m_dev, m_current_selection.m_guest_channel, node_channel, m_current_selection.m_db);
					}
					m_current_selection.m_node_channel = node_channel;
					m_generation --;
					return true;
				}
			}
			break;
		}
		}
		break;
	}
	}

	return false;
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

	auto find_node = [&info](u32 node_id) -> const osd::audio_info::node_info * {
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
			m_selections.emplace_back(select_entry { MT_FULL, omap.m_dev, 0, nmap.m_is_system_default ? 0 : nmap.m_node, 0, nmap.m_db });
		for(const auto &cmap : omap.m_channel_mappings)
			m_selections.emplace_back(select_entry { MT_CHANNEL, omap.m_dev, cmap.m_guest_channel, cmap.m_is_system_default ? 0 : cmap.m_node, cmap.m_node_channel, cmap.m_db });
		if(omap.m_node_mappings.empty() && omap.m_channel_mappings.empty())
			m_selections.emplace_back(select_entry { MT_NONE, omap.m_dev, 0, 0, 0, 0 });
	}

	// If there's nothing, get out of there
	if(m_selections.empty())
		return;

	// Find the line of the current selection, if any.
	// Otherwise default to the first line

	u32 cursel_line = 0xffffffff;

	for(u32 i = 0; i != m_selections.size(); i++) {
		if(m_current_selection == m_selections[i]) {
			cursel_line = i;
			break;
		}
	}

	if(cursel_line == 0xffffffff)
		for(u32 i = 0; i != m_selections.size(); i++) {
			if(m_current_selection.m_dev == m_selections[i].m_dev) {
				cursel_line = i;
				break;
			}
		}

	if(cursel_line == 0xffffffff)
		cursel_line = 0;

	if(m_current_selection.m_maptype == MT_INTERNAL)
		cursel_line = 0xffffffff;
	else
		m_current_selection = m_selections[cursel_line];

	if(m_current_selection.m_maptype == MT_FULL) {
		if(m_current_group == GRP_GUEST_CHANNEL || m_current_group == GRP_NODE_CHANNEL)
			m_current_group = GRP_NODE;
	}

	// (Re)build the menu
	u32 curline = 0;
	for(const auto &omap : mapping) {
		item_append(omap.m_dev->tag(), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
		for(const auto &nmap : omap.m_node_mappings) {
			const auto &node = find_node(nmap.m_node);
			std::string lnode = nmap.m_is_system_default ? "[default]" : node->m_display_name;
			if(!omap.m_dev->is_output() && node->m_sinks)
				lnode = util::string_format("Monitor of %s", lnode);
			if(curline == cursel_line && m_current_group == GRP_NODE)
				lnode = u8"\u25c4" + lnode + u8"\u25ba";

			std::string line = (omap.m_dev->is_output() ? "> " : "< ") + lnode;

			std::string db = util::string_format("%g dB", nmap.m_db);
			if(curline == cursel_line && m_current_group == GRP_DB)
				db = u8"\u25c4" + db + u8"\u25ba";

			item_append(line, db, 0, m_selections.data() + curline);
			curline ++;
		}
		for(const auto &cmap : omap.m_channel_mappings) {
			const auto &node = find_node(cmap.m_node);
			std::string guest_channel = omap.m_dev->get_position_name(cmap.m_guest_channel);
			if(curline == cursel_line && m_current_group == GRP_GUEST_CHANNEL)
				guest_channel = u8"\u25c4" + guest_channel + u8"\u25ba";

			std::string lnode = cmap.m_is_system_default ? "[default]" : node->m_display_name;
			if(!omap.m_dev->is_output() && node->m_sinks)
				lnode = util::string_format("Monitor of %s", lnode);
			if(curline == cursel_line && m_current_group == GRP_NODE)
				lnode = u8"\u25c4" + lnode + u8"\u25ba";

			std::string lnode_channel = node->m_port_names[cmap.m_node_channel];
			if(curline == cursel_line && m_current_group == GRP_NODE_CHANNEL)
				lnode_channel = u8"\u25c4" + lnode_channel + u8"\u25ba";

			std::string line = guest_channel + " > " + lnode + ":" + lnode_channel;

			std::string db = util::string_format("%g dB", cmap.m_db);
			if(curline == cursel_line && m_current_group == GRP_DB)
				db = u8"\u25c4" + db + u8"\u25ba";

			item_append(line, db, 0, m_selections.data() + curline);
			curline ++;
		}
		if(omap.m_node_mappings.empty() && omap.m_channel_mappings.empty()) {
			item_append("[no mapping]", 0, m_selections.data() + curline);
			curline ++;
		}
	}
	item_append(menu_item_type::SEPARATOR);
	item_append(util::string_format("%s: add a full mapping", ui().get_general_input_setting(IPT_UI_MIXER_ADD_FULL)), FLAG_DISABLE, nullptr);
	item_append(util::string_format("%s: add a channel mapping", ui().get_general_input_setting(IPT_UI_MIXER_ADD_CHANNEL)), FLAG_DISABLE, nullptr);
	item_append(util::string_format("%s: remove a mapping", ui().get_general_input_setting(IPT_UI_CLEAR)), FLAG_DISABLE, nullptr);
	item_append(menu_item_type::SEPARATOR);

	if(cursel_line != 0xffffffff)
		set_selection(m_selections.data() + cursel_line);
}


//-------------------------------------------------
//  recompute_metrics - recompute metrics
//-------------------------------------------------

void menu_audio_mixer::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	//	set_custom_space(0.0f, 2.0f * line_height() + 2.0f * tb_border());
}


//-------------------------------------------------
//  menu_audio_mixer_custom_render - perform our special
//  rendering
//-------------------------------------------------

void menu_audio_mixer::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
}


//-------------------------------------------------
//  menu_activated - handle menu gaining focus
//-------------------------------------------------

void menu_audio_mixer::menu_activated()
{
	// scripts or the other form of the menu could have changed something in the mean time
	reset(reset_options::REMEMBER_POSITION);
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

