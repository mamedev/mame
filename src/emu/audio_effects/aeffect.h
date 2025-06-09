// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#pragma once

#ifndef MAME_EMU_AUDIO_EFFECTS_AEFFECT_H
#define MAME_EMU_AUDIO_EFFECTS_AEFFECT_H

class audio_effect
{
public:
	using sample_t = sound_stream::sample_t;

	enum {
		FILTER,
		COMPRESSOR,
		REVERB,
		EQ,
		COUNT
	};

	static const char *const effect_names[COUNT];

	static audio_effect *create(int type, u32 sample_rate, audio_effect *def = nullptr);

	audio_effect(u32 sample_rate, audio_effect *def) : m_default(def), m_sample_rate(sample_rate) {}
	virtual ~audio_effect() = default;

	void copy(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) const;

	virtual int type() const = 0;
	virtual u32 history_size() const;
	virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) = 0;
	virtual void config_load(util::xml::data_node const *ef_node) = 0;
	virtual void config_save(util::xml::data_node *ef_node) const = 0;
	virtual void default_changed() = 0;

protected:
	audio_effect *m_default;
	u32 m_sample_rate;
};

#endif
