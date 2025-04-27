// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Audio resampler

#pragma once

#ifndef MAME_EMU_RESAMPLER_H
#define MAME_EMU_RESAMPLER_H

#include "sound.h"

class audio_resampler
{
public:
	using sample_t = sound_stream::sample_t;

	audio_resampler(u32 fs, u32 ft);

	u32 history_size() const { return m_order_per_lane; }

	void apply(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<sample_t> &dest, u64 dest_sample, u32 srcc, float gain, u32 samples) const;
	void apply(const emu::detail::output_buffer_interleaved<s16> &src, std::vector<sample_t> &dest, u64 dest_sample, u32 srcc, float gain, u32 samples) const;
	void apply(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<s16> &dest, u32 destc, int dchannels, u64 dest_sample, u32 srcc, float gain, u32 samples) const;

private:
	u32 m_order_per_lane, m_ftm, m_fsm, m_ft, m_fs, m_delta, m_skip, m_phases, m_phase_shift;

	std::vector<std::vector<float>> m_coefficients;

	static u32 compute_gcd(u32 fs, u32 ft);
};

#endif

