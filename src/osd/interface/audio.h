// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_OSD_INTERFACE_AUDIO_H
#define MAME_OSD_INTERFACE_AUDIO_H

#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>


namespace osd {

struct channel_position {
	// Special positions

	// Position is unknown, placed in the middle
	static constexpr channel_position UNKNOWN() { return channel_position(0.0, 0.0, 0.0); }

	// This channel should only be mapped explicitely through a channel mapping (on request)
	static constexpr channel_position ONREQ() { return channel_position(0.0, 0.0, 10.0); }

	// This channel is a LFE
	static constexpr channel_position LFE() { return channel_position(0.0, 0.0, 11.0); }

	// Standard positions
	static constexpr channel_position FC()       { return channel_position( 0.0, 0.0, 1.0); }
	static constexpr channel_position FL()       { return channel_position(-0.2, 0.0, 1.0); }
	static constexpr channel_position FR()       { return channel_position( 0.2, 0.0, 1.0); }
	static constexpr channel_position RC()       { return channel_position( 0.0, 0.0,-0.5); }
	static constexpr channel_position RL()       { return channel_position(-0.2, 0.0,-0.5); }
	static constexpr channel_position RR()       { return channel_position( 0.2, 0.0,-0.5); }
	static constexpr channel_position HC()       { return channel_position( 0.0, 0.0,-0.1); }
	static constexpr channel_position HL()       { return channel_position(-0.1, 0.0,-0.1); }
	static constexpr channel_position HR()       { return channel_position( 0.1, 0.0,-0.1); }
	static constexpr channel_position BACKREST() { return channel_position( 0.0,-0.2, 0.1); }

	double m_x, m_y, m_z;

	constexpr channel_position() : channel_position(UNKNOWN()) {}
	constexpr channel_position(double x, double y, double z) : m_x(x), m_y(y), m_z(z) {}
	constexpr channel_position(const channel_position &pos) : m_x(pos.m_x), m_y(pos.m_y), m_z(pos.m_z) {}

	constexpr bool operator==(const channel_position &pos) const {
		return (pos.m_x == m_x) && (pos.m_y == m_y) && (pos.m_z == m_z);
	}

	constexpr bool is_lfe() const { return *this == LFE(); }
	constexpr bool is_onreq() const { return *this == ONREQ(); }
	constexpr bool is_unknown() const { return *this == UNKNOWN(); }

	std::string name() const;
};

struct audio_rate_range {
	uint32_t m_default_rate;
	uint32_t m_min_rate;
	uint32_t m_max_rate;

	uint32_t resolve(uint32_t rate) const {
		if(m_max_rate == 0)
			return rate;
		if(rate >= m_min_rate && rate <= m_max_rate)
			return rate;
		if(m_default_rate != 0)
			return m_default_rate;
		if(rate < m_min_rate)
			return m_min_rate;
		return m_max_rate;
	}
};

struct audio_info {
	struct node_info {
		std::string m_name;
		std::string m_display_name;
		uint32_t m_id;
		audio_rate_range m_rate;
		std::vector<std::string> m_port_names;
		std::vector<channel_position> m_port_positions;
		uint32_t m_sinks;
		uint32_t m_sources;

		std::string name() const { return (m_sinks ? "o:" : "i:") + m_name; }

		node_info() = default;
		node_info(const node_info &) = default;
		node_info(node_info &&) = default;
		node_info &operator=(const node_info &) = default;
		node_info &operator=(node_info &&) = default;

		uint32_t resolve_rate(uint32_t rate) const { return m_rate.resolve(rate); }
	};

	struct stream_info {
		uint32_t m_id;
		uint32_t m_node;
		std::vector<float> m_volumes;

		stream_info() = default;
		stream_info(const stream_info &) = default;
		stream_info(stream_info &&) = default;
		stream_info &operator=(const stream_info &) = default;
		stream_info &operator=(stream_info &&) = default;
	};

	uint32_t m_generation;
	uint32_t m_default_sink;
	uint32_t m_default_source;
	std::vector<node_info> m_nodes;
	std::vector<stream_info> m_streams;

	audio_info() = default;
	audio_info(const audio_info &) = default;
	audio_info(audio_info &&) = default;
	audio_info &operator=(const audio_info &) = default;
	audio_info &operator=(audio_info &&) = default;
};

inline void swap(audio_info &a, audio_info &b)
{
	using std::swap;
	swap(a.m_generation, b.m_generation);
	swap(a.m_default_sink, b.m_default_sink);
	swap(a.m_nodes, b.m_nodes);
	swap(a.m_streams, b.m_streams);
}

inline void swap(audio_info::node_info &a, audio_info::node_info &b)
{
	using std::swap;
	swap(a.m_name, b.m_name);
	swap(a.m_id, b.m_id);
	swap(a.m_rate, b.m_rate);
	swap(a.m_port_names, b.m_port_names);
	swap(a.m_port_positions, b.m_port_positions);
	swap(a.m_sinks, b.m_sinks);
	swap(a.m_sources, b.m_sources);
}

inline void swap(audio_info::stream_info &a, audio_info::stream_info &b)
{
	using std::swap;
	swap(a.m_id, b.m_id);
	swap(a.m_node, b.m_node);
	swap(a.m_volumes, b.m_volumes);
}

inline float db_to_linear(float db) { return (db <= -96.0F) ? 0.0F : std::pow(10.0F, db / 20.0F); }
inline float linear_to_db(float linear) { return (linear <= (1.0F / 65536.0F)) ? -96.0F : (20.0F * std::log10(linear)); }

} // namespace osd

#endif // MAME_OSD_INTERFACE_AUDIO_H
