// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation (Audio)

**********************************************************************/

#ifndef MAME_MACHINE_SPG2XX_AUDIO_H
#define MAME_MACHINE_SPG2XX_AUDIO_H

#pragma once

#include "sound/imaadpcm.h"
#include "cpu/unsp/unsp.h"

class spg2xx_audio_device : public device_t, public device_sound_interface
{
public:
	spg2xx_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	spg2xx_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto space_read_callback() { return m_space_read_cb.bind(); }
	auto write_irq_callback() { return m_irq_cb.bind(); }
	auto channel_irq_callback() { return m_ch_irq_cb.bind(); }

	uint16_t audio_r(offs_t offset);
	virtual void audio_w(offs_t offset, uint16_t data);
	uint16_t audio_ctrl_r(offs_t offset);
	void audio_ctrl_w(offs_t offset, uint16_t data);
	uint16_t audio_phase_r(offs_t offset);
	void audio_phase_w(offs_t offset, uint16_t data);

protected:
	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	TIMER_CALLBACK_MEMBER(irq_tick);
	TIMER_CALLBACK_MEMBER(audio_beat_tick);
	void audio_rampdown_tick(const uint32_t channel);
	bool audio_envelope_tick(const uint32_t channel);
	inline uint32_t get_rampdown_frame_count(const uint32_t channel);
	inline uint32_t get_envclk_frame_count(const uint32_t channel);

	// Audio getters
	bool get_channel_enable(const offs_t channel) const { return m_audio_ctrl_regs[AUDIO_CHANNEL_ENABLE] & (1 << channel); }
	bool get_channel_status(const offs_t channel) const { return m_audio_ctrl_regs[AUDIO_CHANNEL_STATUS] & (1 << channel); }
	bool get_manual_envelope_enable(const offs_t channel) const { return m_audio_ctrl_regs[AUDIO_CHANNEL_ENV_MODE] & (1 << channel); }
	bool get_auto_envelope_enable(const offs_t channel) const { return !get_manual_envelope_enable(channel); }
	uint32_t get_envelope_clock(const offs_t channel) const;
	uint16_t get_vol_sel() const { return (m_audio_ctrl_regs[AUDIO_CONTROL] & AUDIO_CONTROL_VOLSEL_MASK) >> AUDIO_CONTROL_VOLSEL_SHIFT; }

	// Audio Mode getters
	uint16_t get_wave_addr_high(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_WADDR_HIGH_MASK; }
	uint16_t get_loop_addr_high(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_LADDR_HIGH_MASK) >> AUDIO_LADDR_HIGH_SHIFT; }
	uint16_t get_tone_mode(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_TONE_MODE_MASK) >> AUDIO_TONE_MODE_SHIFT; }
	virtual uint16_t get_16bit_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_16M_MASK) ? 1 : 0; }
	virtual uint16_t get_adpcm_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_ADPCM_MASK) ? 1 : 0; }

	// Audio Pan getters
	uint16_t get_volume(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_PAN_VOL] & AUDIO_VOLUME_MASK; }
	uint16_t get_pan(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_PAN_VOL] & AUDIO_PAN_MASK) >> AUDIO_PAN_SHIFT; }

	// Audio Envelope0 Data getters
	uint16_t get_envelope_inc(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE0] & AUDIO_ENVELOPE_INC_MASK; }
	uint16_t get_envelope_sign_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE0] & AUDIO_ENVELOPE_SIGN_MASK) ? 1 : 0; }
	uint16_t get_envelope_target(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE0] & AUDIO_ENVELOPE_TARGET_MASK) >> AUDIO_ENVELOPE_TARGET_SHIFT; }
	uint16_t get_repeat_period_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE0] & AUDIO_ENVELOPE_REPEAT_PERIOD_MASK) ? 1 : 0; }

	// Audio Envelope Data getters
	uint16_t get_edd(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] & AUDIO_EDD_MASK; }
	uint16_t get_envelope_count(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] & AUDIO_ENVELOPE_COUNT_MASK) >> AUDIO_ENVELOPE_COUNT_SHIFT; }
	void set_edd(const offs_t channel, uint8_t edd) { m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] = (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] & ~AUDIO_EDD_MASK) | edd; }
	void set_envelope_count(const offs_t channel, uint16_t count) { m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] = get_edd(channel) | (count << AUDIO_ENVELOPE_COUNT_SHIFT); }

	// Audio Envelope1 Data getters
	uint16_t get_envelope_load(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] & AUDIO_ENVELOPE_LOAD_MASK; }
	uint16_t get_envelope_repeat_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] & AUDIO_ENVELOPE_RPT_MASK) ? 1 : 0; }
	uint16_t get_envelope_repeat_count(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] & AUDIO_ENVELOPE_RPCNT_MASK) >> AUDIO_ENVELOPE_RPCNT_SHIFT; }
	inline void set_envelope_repeat_count(const offs_t channel, const uint16_t count) { m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] = (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] & ~AUDIO_ENVELOPE_RPCNT_MASK) | ((count << AUDIO_ENVELOPE_RPCNT_SHIFT) & AUDIO_ENVELOPE_RPCNT_MASK); }

	// Audio Envelope Address getters
	uint16_t get_envelope_addr_high(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_ADDR_HIGH] & AUDIO_EADDR_HIGH_MASK; }
	uint16_t get_audio_irq_enable_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_ADDR_HIGH] & AUDIO_IRQ_EN_MASK) ? 1 : 0; }
	uint16_t get_audio_irq_addr(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_ADDR_HIGH] & AUDIO_IRQ_ADDR_MASK) >> AUDIO_IRQ_ADDR_SHIFT; }

	// Audio Envelope Loop getters
	uint16_t get_envelope_eaoffset(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_LOOP_CTRL] & AUDIO_EAOFFSET_MASK; }
	uint16_t get_rampdown_offset(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_LOOP_CTRL] & AUDIO_RAMPDOWN_OFFSET_MASK) >> AUDIO_RAMPDOWN_OFFSET_SHIFT; }
	void set_envelope_eaoffset(const offs_t channel, uint16_t eaoffset) { m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_LOOP_CTRL] = (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_LOOP_CTRL] & ~AUDIO_RAMPDOWN_OFFSET_MASK) | (eaoffset & AUDIO_EAOFFSET_MASK); }

	// Audio ADPCM getters
	uint16_t get_point_number(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ADPCM_SEL] & AUDIO_POINT_NUMBER_MASK) >> AUDIO_POINT_NUMBER_SHIFT; }
	uint16_t get_adpcm36_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ADPCM_SEL] & AUDIO_ADPCM36_MASK) ? 1 : 0; }

	// Audio high-word getters
	uint16_t get_phase_high(const offs_t channel) const { return m_audio_phase_regs[(channel << 4) | AUDIO_PHASE_HIGH] & AUDIO_PHASE_HIGH_MASK; }
	uint16_t get_phase_accum_high(const offs_t channel) const { return m_audio_phase_regs[(channel << 4) | AUDIO_PHASE_ACCUM_HIGH] & AUDIO_PHASE_ACCUM_HIGH_MASK; }
	uint16_t get_target_phase_high(const offs_t channel) const { return m_audio_phase_regs[(channel << 4) | AUDIO_TARGET_PHASE_HIGH] & AUDIO_TARGET_PHASE_HIGH_MASK; }
	uint16_t get_rampdown_clock(const offs_t channel) const { return m_audio_phase_regs[(channel << 4) | AUDIO_RAMP_DOWN_CLOCK] & AUDIO_RAMP_DOWN_CLOCK_MASK; }

	// Audio ADPCM getters
	uint16_t get_phase_offset(const offs_t channel) const { return m_audio_phase_regs[(channel << 4) | AUDIO_PHASE_CTRL] & AUDIO_PHASE_OFFSET_MASK; }
	uint16_t get_phase_sign_bit(const offs_t channel) const { return (m_audio_phase_regs[(channel << 4) | AUDIO_PHASE_CTRL] & AUDIO_PHASE_SIGN_MASK) >> AUDIO_PHASE_SIGN_SHIFT; }
	uint16_t get_phase_time_step(const offs_t channel) const { return (m_audio_phase_regs[(channel << 4) | AUDIO_PHASE_CTRL] & AUDIO_PHASE_TIME_STEP_MASK) >> AUDIO_PHASE_TIME_STEP_SHIFT; }

	// Audio combined getters
	virtual uint32_t get_phase(const offs_t channel) const { return ((uint32_t)get_phase_high(channel) << 16) | m_audio_phase_regs[(channel << 4) | AUDIO_PHASE]; }
	uint32_t get_phase_accum(const offs_t channel) const { return ((uint32_t)get_phase_accum_high(channel) << 16) | m_audio_phase_regs[(channel << 4) | AUDIO_PHASE_ACCUM]; }
	uint32_t get_target_phase(const offs_t channel) const { return ((uint32_t)get_target_phase_high(channel) << 16) | m_audio_phase_regs[(channel << 4) | AUDIO_TARGET_PHASE]; }

	uint32_t get_wave_addr(const offs_t channel) const { return ((uint32_t)get_wave_addr_high(channel) << 16) | m_audio_regs[(channel << 4) | AUDIO_WAVE_ADDR]; }
	uint32_t get_loop_addr(const offs_t channel) const { return ((uint32_t)get_loop_addr_high(channel) << 16) | m_audio_regs[(channel << 4) | AUDIO_LOOP_ADDR]; }
	uint32_t get_envelope_addr(const offs_t channel) const { return ((uint32_t)get_envelope_addr_high(channel) << 16) | m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_ADDR]; }
	void inc_wave_addr(const offs_t channel) { set_wave_addr(channel, get_wave_addr(channel) + 1); }
	void set_wave_addr(const offs_t channel, uint32_t addr)
	{
		m_audio_regs[(channel << 4) | AUDIO_MODE] &= ~AUDIO_WADDR_HIGH_MASK;
		m_audio_regs[(channel << 4) | AUDIO_MODE] |= (addr >> 16) & AUDIO_WADDR_HIGH_MASK;
		m_audio_regs[(channel << 4) | AUDIO_WAVE_ADDR] = addr & 0xffff;
	}

	enum // at audio write offset 0x000 in spg2xx
	{
		AUDIO_WAVE_ADDR             = 0x000,

		AUDIO_MODE                  = 0x001,
		AUDIO_WADDR_HIGH_MASK       = 0x003f,
		AUDIO_LADDR_HIGH_MASK       = 0x0fc0,
		AUDIO_LADDR_HIGH_SHIFT      = 6,
		AUDIO_TONE_MODE_MASK        = 0x3000,
		AUDIO_TONE_MODE_SHIFT       = 12,
		AUDIO_TONE_MODE_SW          = 0,
		AUDIO_TONE_MODE_HW_ONESHOT  = 1,
		AUDIO_TONE_MODE_HW_LOOP     = 2,
		AUDIO_16M_MASK              = 0x4000,
		AUDIO_ADPCM_MASK            = 0x8000,

		AUDIO_LOOP_ADDR             = 0x002,

		AUDIO_PAN_VOL               = 0x003,
		AUDIO_PAN_VOL_MASK          = 0x7f7f,
		AUDIO_VOLUME_MASK           = 0x007f,
		AUDIO_PAN_MASK              = 0x7f00,
		AUDIO_PAN_SHIFT             = 8,

		AUDIO_ENVELOPE0             = 0x004,
		AUDIO_ENVELOPE_INC_MASK     = 0x007f,
		AUDIO_ENVELOPE_SIGN_MASK    = 0x0080,
		AUDIO_ENVELOPE_TARGET_MASK  = 0x7f00,
		AUDIO_ENVELOPE_TARGET_SHIFT = 8,
		AUDIO_ENVELOPE_REPEAT_PERIOD_MASK = 0x8000,

		AUDIO_ENVELOPE_DATA         = 0x005,
		AUDIO_ENVELOPE_DATA_MASK    = 0xff7f,
		AUDIO_EDD_MASK              = 0x007f,
		AUDIO_ENVELOPE_COUNT_MASK   = 0xff00,
		AUDIO_ENVELOPE_COUNT_SHIFT  = 8,

		AUDIO_ENVELOPE1             = 0x006,
		AUDIO_ENVELOPE_LOAD_MASK    = 0x00ff,
		AUDIO_ENVELOPE_RPT_MASK     = 0x0100,
		AUDIO_ENVELOPE_RPCNT_MASK   = 0xfe00,
		AUDIO_ENVELOPE_RPCNT_SHIFT  = 9,

		AUDIO_ENVELOPE_ADDR_HIGH    = 0x007,
		AUDIO_EADDR_HIGH_MASK       = 0x003f,
		AUDIO_IRQ_EN_MASK           = 0x0040,
		AUDIO_IRQ_ADDR_MASK         = 0xff80,
		AUDIO_IRQ_ADDR_SHIFT        = 7,

		AUDIO_ENVELOPE_ADDR         = 0x008,
		AUDIO_WAVE_DATA_PREV        = 0x009,

		AUDIO_ENVELOPE_LOOP_CTRL    = 0x00a,
		AUDIO_EAOFFSET_MASK         = 0x01ff,
		AUDIO_RAMPDOWN_OFFSET_MASK  = 0xfe00,
		AUDIO_RAMPDOWN_OFFSET_SHIFT = 9,

		AUDIO_WAVE_DATA             = 0x00b,

		AUDIO_ADPCM_SEL             = 0x00d,
		AUDIO_ADPCM_SEL_MASK        = 0xfe00,
		AUDIO_POINT_NUMBER_MASK     = 0x7e00,
		AUDIO_POINT_NUMBER_SHIFT    = 9,
		AUDIO_ADPCM36_MASK          = 0x8000,
	};

	enum // at audio write offset 0x200 in spg2xx
	{
		AUDIO_PHASE_HIGH            = 0x000,
		AUDIO_PHASE_HIGH_MASK       = 0x0007,

		AUDIO_PHASE_ACCUM_HIGH      = 0x001,
		AUDIO_PHASE_ACCUM_HIGH_MASK = 0x0007,

		AUDIO_TARGET_PHASE_HIGH     = 0x002,
		AUDIO_TARGET_PHASE_HIGH_MASK= 0x0007,

		AUDIO_RAMP_DOWN_CLOCK       = 0x003,
		AUDIO_RAMP_DOWN_CLOCK_MASK  = 0x0007,

		AUDIO_PHASE                 = 0x004,
		AUDIO_PHASE_ACCUM           = 0x005,
		AUDIO_TARGET_PHASE          = 0x006,

		AUDIO_PHASE_CTRL            = 0x007,
		AUDIO_PHASE_OFFSET_MASK     = 0x0fff,
		AUDIO_PHASE_SIGN_MASK       = 0x1000,
		AUDIO_PHASE_SIGN_SHIFT      = 12,
		AUDIO_PHASE_TIME_STEP_MASK  = 0xe000,
		AUDIO_PHASE_TIME_STEP_SHIFT = 13,

		AUDIO_CHAN_OFFSET_MASK      = 0xf0f,
	};

	enum // at audio write offset 0x400 in spg2xx
	{

		AUDIO_CHANNEL_ENABLE            = 0x000,
		AUDIO_CHANNEL_ENABLE_MASK       = 0xffff,

		AUDIO_MAIN_VOLUME               = 0x001,
		AUDIO_MAIN_VOLUME_MASK          = 0x007f,

		AUDIO_CHANNEL_FIQ_ENABLE        = 0x002,
		AUDIO_CHANNEL_FIQ_ENABLE_MASK   = 0xffff,

		AUDIO_CHANNEL_FIQ_STATUS        = 0x003,
		AUDIO_CHANNEL_FIQ_STATUS_MASK   = 0xffff,

		AUDIO_BEAT_BASE_COUNT           = 0x004,
		AUDIO_BEAT_BASE_COUNT_MASK      = 0x07ff,

		AUDIO_BEAT_COUNT                = 0x005,
		AUDIO_BEAT_COUNT_MASK           = 0x3fff,
		AUDIO_BIS_MASK                  = 0x4000,
		AUDIO_BIE_MASK                  = 0x8000,

		AUDIO_ENVCLK0                   = 0x006,

		AUDIO_ENVCLK0_HIGH              = 0x007,
		AUDIO_ENVCLK0_HIGH_MASK         = 0xffff,

		AUDIO_ENVCLK1                   = 0x008,

		AUDIO_ENVCLK1_HIGH              = 0x009,
		AUDIO_ENVCLK1_HIGH_MASK         = 0xffff,

		AUDIO_ENV_RAMP_DOWN             = 0x00a,
		AUDIO_ENV_RAMP_DOWN_MASK        = 0xffff,

		AUDIO_CHANNEL_STOP              = 0x00b,
		AUDIO_CHANNEL_STOP_MASK         = 0xffff,

		AUDIO_CHANNEL_ZERO_CROSS        = 0x00c,
		AUDIO_CHANNEL_ZERO_CROSS_MASK   = 0xffff,

		AUDIO_CONTROL                   = 0x00d,
		AUDIO_CONTROL_MASK              = 0x9fe8,
		AUDIO_CONTROL_SATURATE_MASK     = 0x8000,
		AUDIO_CONTROL_SOFTCH_MASK       = 0x1000,
		AUDIO_CONTROL_COMPEN_MASK       = 0x0800,
		AUDIO_CONTROL_NOHIGH_MASK       = 0x0400,
		AUDIO_CONTROL_NOINT_MASK        = 0x0200,
		AUDIO_CONTROL_EQEN_MASK         = 0x0100,
		AUDIO_CONTROL_VOLSEL_MASK       = 0x00c0,
		AUDIO_CONTROL_VOLSEL_SHIFT      = 6,
		AUDIO_CONTROL_FOF_MASK          = 0x0020,
		AUDIO_CONTROL_INIT_MASK         = 0x0008,

		AUDIO_COMPRESS_CTRL             = 0x00e,
		AUDIO_COMPRESS_CTRL_PEAK_MASK   = 0x8000,
		AUDIO_COMPRESS_CTRL_THRESHOLD_MASK  = 0x7f00,
		AUDIO_COMPRESS_CTRL_THRESHOLD_SHIFT = 8,
		AUDIO_COMPRESS_CTRL_ATTSCALE_MASK   = 0x00c0,
		AUDIO_COMPRESS_CTRL_ATTSCALE_SHIFT  = 6,
		AUDIO_COMPRESS_CTRL_RELSCALE_MASK   = 0x0030,
		AUDIO_COMPRESS_CTRL_RELSCALE_SHIFT  = 4,
		AUDIO_COMPRESS_CTRL_DISZC_MASK      = 0x0008,
		AUDIO_COMPRESS_CTRL_RATIO_MASK      = 0x0007,

		AUDIO_CHANNEL_STATUS            = 0x00f,
		AUDIO_CHANNEL_STATUS_MASK       = 0xffff,

		AUDIO_WAVE_IN_L                 = 0x010,

		AUDIO_WAVE_IN_R                 = 0x011,
		AUDIO_SOFTIRQ_MASK              = 0x8000,
		AUDIO_SOFTIRQ_EN_MASK           = 0x4000,
		AUDIO_SOFT_PHASE_HIGH_MASK      = 0x0070,
		AUDIO_SOFT_PHASE_HIGH_SHIFT     = 4,
		AUDIO_FIFO_IRQ_THRESHOLD_MASK   = 0x000f,

		AUDIO_WAVE_OUT_L                = 0x012,
		AUDIO_WAVE_OUT_R                = 0x013,

		AUDIO_CHANNEL_REPEAT            = 0x014,
		AUDIO_CHANNEL_REPEAT_MASK       = 0xffff,

		AUDIO_CHANNEL_ENV_MODE          = 0x015,
		AUDIO_CHANNEL_ENV_MODE_MASK     = 0xffff,

		AUDIO_CHANNEL_TONE_RELEASE      = 0x016,
		AUDIO_CHANNEL_TONE_RELEASE_MASK = 0xffff,

		AUDIO_CHANNEL_ENV_IRQ           = 0x017,
		AUDIO_CHANNEL_ENV_IRQ_MASK      = 0xffff,

		AUDIO_CHANNEL_PITCH_BEND        = 0x018,
		AUDIO_CHANNEL_PITCH_BEND_MASK   = 0xffff,

		AUDIO_SOFT_PHASE                = 0x019,

		AUDIO_ATTACK_RELEASE            = 0x01a,
		AUDIO_RELEASE_TIME_MASK         = 0x00ff,
		AUDIO_ATTACK_TIME_MASK          = 0xff00,
		AUDIO_ATTACK_TIME_SHIFT         = 8,

		AUDIO_EQ_CUTOFF10               = 0x01b,
		AUDIO_EQ_CUTOFF10_MASK          = 0x7f7f,

		AUDIO_EQ_CUTOFF32               = 0x01c,
		AUDIO_EQ_CUTOFF32_MASK          = 0x7f7f,

		AUDIO_EQ_GAIN10                 = 0x01d,
		AUDIO_EQ_GAIN10_MASK            = 0x7f7f,

		AUDIO_EQ_GAIN32                 = 0x01e,
		AUDIO_EQ_GAIN32_MASK            = 0x7f7f
	};

	struct adpcm36_state
	{
		uint16_t m_remaining;
		uint16_t m_header;
		int16_t m_prevsamp[2];
	};

	void check_irqs(const uint16_t changed);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	uint16_t read_space(offs_t offset);

	void start_channel(const uint32_t channel);
	void stop_channel(const uint32_t channel);
	bool advance_channel(const uint32_t channel);
	bool fetch_sample(const uint32_t channel);
	void loop_channel(const uint32_t channel);
	uint16_t decode_adpcm36_nybble(const uint32_t channel, const uint8_t data);
	void read_adpcm36_header(const uint32_t channel);

	bool m_debug_samples;
	bool m_debug_rates;

	uint16_t m_audio_regs[0x200];
	uint16_t m_audio_phase_regs[0x200];
	uint16_t m_audio_ctrl_regs[0x400];
	uint8_t m_sample_shift[16];
	uint32_t m_sample_count[16];
	double m_channel_rate[16];
	double m_channel_rate_accum[16];
	uint32_t m_rampdown_frame[16];
	uint32_t m_envclk_frame[16];
	uint32_t m_envelope_addr[16];
	int m_channel_debug;
	uint16_t m_audio_curr_beat_base_count;

	emu_timer *m_audio_beat;
	emu_timer *m_channel_irq[16];

	sound_stream *m_stream;
	ima_adpcm_state m_adpcm[16];
	adpcm36_state m_adpcm36_state[16];

	static const uint32_t s_rampdown_frame_counts[8];
	static const uint32_t s_envclk_frame_counts[16];

private:
	devcb_read16 m_space_read_cb;
	devcb_write_line m_irq_cb;
	devcb_write_line m_ch_irq_cb;
};

class spg110_audio_device : public spg2xx_audio_device
{
public:
	spg110_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void audio_w(offs_t offset, uint16_t data) override;

	// these either come from somewhere else on spg110 or are hardcoded
	virtual uint16_t get_16bit_bit(const offs_t channel) const override { return 1; }
	virtual uint16_t get_adpcm_bit(const offs_t channel) const override { return 0; }

	virtual uint32_t get_phase(const offs_t channel) const override { return m_audio_regs[(channel << 4) | 0xe]; }
};

class sunplus_gcm394_audio_device : public spg2xx_audio_device
{
public:
	sunplus_gcm394_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t control_r(offs_t offset);
	void control_w(offs_t offset, uint16_t data);

	virtual void device_start() override ATTR_COLD;

private:
	uint16_t control_group16_r(uint8_t group, uint8_t offset);
	void control_group16_w(uint8_t group, uint8_t offset, uint16_t data);

	uint16_t m_control[2][0x20];
};

DECLARE_DEVICE_TYPE(SPG2XX_AUDIO, spg2xx_audio_device)
DECLARE_DEVICE_TYPE(SPG110_AUDIO, spg110_audio_device)
DECLARE_DEVICE_TYPE(SUNPLUS_GCM394_AUDIO, sunplus_gcm394_audio_device)

#endif // MAME_MACHINE_SPG2XX_AUDIO_H
