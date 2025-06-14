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

	virtual ~audio_resampler() = default;

	virtual u32 history_size() const = 0;

	virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<sample_t> &dest, u64 dest_sample, u32 srcc, float gain, u32 samples) const = 0;
	virtual void apply(const emu::detail::output_buffer_interleaved<s16> &src, sound_stream &dest, u32 srcc, u32 destc, float gain) const = 0;
	virtual void apply_copy(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<s16> &dest, u32 destc, int dchannels, u64 dest_sample, u32 srcc, float gain, u32 samples) const = 0;
	virtual void apply_add(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<s16> &dest, u32 destc, int dchannels, u64 dest_sample, u32 srcc, float gain, u32 samples) const = 0;
};

class audio_resampler_hq : public audio_resampler
{
public:
	audio_resampler_hq(u32 fs, u32 ft, float latency, u32 max_order_per_lane, u32 max_lanes);
	virtual ~audio_resampler_hq() = default;

	virtual u32 history_size() const override;

	virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<sample_t> &dest, u64 dest_sample, u32 srcc, float gain, u32 samples) const override;
	virtual void apply(const emu::detail::output_buffer_interleaved<s16> &src, sound_stream &dest, u32 srcc, u32 destc, float gain) const override;
	virtual void apply_copy(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<s16> &dest, u32 destc, int dchannels, u64 dest_sample, u32 srcc, float gain, u32 samples) const override;
	virtual void apply_add(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<s16> &dest, u32 destc, int dchannels, u64 dest_sample, u32 srcc, float gain, u32 samples) const override;

private:
	u32 m_order_per_lane, m_ftm, m_fsm, m_ft, m_fs, m_delta, m_skip, m_phases, m_phase_shift;

	std::vector<std::vector<float>> m_coefficients;

	static u32 compute_gcd(u32 fs, u32 ft);
};

class audio_resampler_lofi : public audio_resampler
{
public:
	audio_resampler_lofi(u32 fs, u32 ft);
	virtual ~audio_resampler_lofi() = default;

	virtual u32 history_size() const override;

	virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<sample_t> &dest, u64 dest_sample, u32 srcc, float gain, u32 samples) const override;
	virtual void apply(const emu::detail::output_buffer_interleaved<s16> &src, sound_stream &dest, u32 srcc, u32 destc, float gain) const override;
	virtual void apply_copy(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<s16> &dest, u32 destc, int dchannels, u64 dest_sample, u32 srcc, float gain, u32 samples) const override;
	virtual void apply_add(const emu::detail::output_buffer_flat<sample_t> &src, std::vector<s16> &dest, u32 destc, int dchannels, u64 dest_sample, u32 srcc, float gain, u32 samples) const override;

private:
	static const std::array<std::array<float, 0x1001>, 2> interpolation_table;
	u32 m_source_divide, m_fs, m_ft;
	u32 m_step;
};

#endif

