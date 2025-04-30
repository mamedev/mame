// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_OSD_INTERFACE_AUDIO_H
#define MAME_OSD_INTERFACE_AUDIO_H

#pragma once

#include <array>
#include <cmath>
#include <string>
#include <vector>


namespace osd {

struct audio_rate_range {
	uint32_t m_default_rate;
	uint32_t m_min_rate;
	uint32_t m_max_rate;
};

struct audio_info {
	struct node_info {
		std::string m_name;
		uint32_t m_id;
		audio_rate_range m_rate;
		std::vector<std::string> m_port_names;
		std::vector<std::array<double, 3> > m_port_positions;
		uint32_t m_sinks;
		uint32_t m_sources;

		std::string name() const { return (m_sinks ? "o:" : "i:") + m_name; }
	};

	struct stream_info {
		uint32_t m_id;
		uint32_t m_node;
		std::vector<float> m_volumes;
	};

	uint32_t m_generation;
	uint32_t m_default_sink;
	uint32_t m_default_source;
	std::vector<node_info> m_nodes;
	std::vector<stream_info> m_streams;
};

inline float db_to_linear(float db) { return (db <= -96.0F) ? 0.0F : std::pow(10.0F, db / 20.0F); }
inline float linear_to_db(float linear) { return (linear <= (1.0F / 65536.0F)) ? -96.0F : (20.0F * std::log10(linear)); }

} // namespace osd

#endif // MAME_OSD_INTERFACE_AUDIO_H
