// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/audiomix.h

    Audio mixing/mapping control

***************************************************************************/

#ifndef MAME_FRONTEND_UI_AUDIOMIX_H
#define MAME_FRONTEND_UI_AUDIOMIX_H

#pragma once

#include "ui/menu.h"

#include <string>
#include <vector>


namespace ui {

class menu_audio_mixer : public menu
{
public:
	menu_audio_mixer(mame_ui_manager &mui, render_container &container);
	virtual ~menu_audio_mixer() override;

protected:
	virtual void menu_activated() override;
	virtual void menu_deactivated() override;

private:
	struct select_entry {
		uint32_t m_maptype;
		sound_io_device *m_dev;
		uint32_t m_guest_channel;
		uint32_t m_node;
		uint32_t m_node_channel;
		float m_db;

		bool operator==(const select_entry &sel) const {
			return
					(sel.m_maptype == m_maptype) &&
					(sel.m_dev == m_dev) &&
					(sel.m_guest_channel == m_guest_channel) &&
					(sel.m_node == m_node) &&
					(sel.m_node_channel == m_node_channel);
		}
	};

	uint32_t m_generation;
	select_entry m_reset_selection;
	uint32_t m_reset_item;
	std::vector<select_entry> m_selections;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	bool add_full(select_entry &current_selection);
	bool add_channel(select_entry &current_selection);
	bool remove_route(uint32_t cursel_index, select_entry &current_selection);
	bool set_prev_guest_channel(select_entry &current_selection);
	bool set_next_guest_channel(select_entry &current_selection);
	bool set_prev_node(select_entry &current_selection);
	bool set_next_node(select_entry &current_selection);
	bool set_prev_node_channel(select_entry &current_selection);
	bool set_next_node_channel(select_entry &current_selection);
	bool set_route_volume(menu_item &item, select_entry &current_selection);

	uint32_t find_node_index(uint32_t node) const;
	std::string find_node_name(uint32_t node) const;
	uint32_t find_node_channel_count(uint32_t node, bool is_output) const;

	uint32_t find_next_sink_node_index(uint32_t index) const;
	uint32_t find_next_source_node_index(uint32_t index) const;
	uint32_t find_previous_sink_node_index(uint32_t index) const;
	uint32_t find_previous_source_node_index(uint32_t index) const;

	uint32_t find_first_sink_node_index() const;
	uint32_t find_first_source_node_index() const;
	uint32_t find_last_sink_node_index() const;
	uint32_t find_last_source_node_index() const;

	bool full_mapping_available(sound_io_device *dev, uint32_t node) const;
	bool channel_mapping_available(sound_io_device *dev, uint32_t guest_channel, uint32_t node, uint32_t node_channel) const;

	uint32_t find_next_available_node(sound_io_device *dev, uint32_t node) const;
	uint32_t find_previous_available_node(sound_io_device *dev, uint32_t node) const;
	uint32_t find_next_available_channel_node(sound_io_device *dev, uint32_t guest_channel, uint32_t node, uint32_t node_channel) const;
	uint32_t find_previous_available_channel_node(sound_io_device *dev, uint32_t guest_channel, uint32_t node, uint32_t node_channel) const;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_AUDIOMIX_H
