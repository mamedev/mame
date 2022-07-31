// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SHARED_EXIDYSOUND_H
#define MAME_SHARED_EXIDYSOUND_H

#pragma once

#include "machine/6532riot.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "sound/flt_biquad.h"
#include "sound/hc55516.h"
#include "sound/tms5220.h"


class exidy_sound_device : public device_t,
									public device_sound_interface
{
	/* 6840 variables */
	struct sh6840_timer_channel
	{
		uint8_t   cr = 0;
		uint8_t   state = 0;
		uint8_t   leftovers = 0;
		uint16_t  timer = 0;
		uint32_t  clocks = 0;
		union
		{
#ifdef LSB_FIRST
			struct { uint8_t l, h; } b;
#else
			struct { uint8_t h, l; } b;
#endif
			uint16_t w;
		} counter;
	};

public:
	exidy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~exidy_sound_device() {}

	uint8_t sh6840_r(offs_t offset);
	void sh6840_w(offs_t offset, uint8_t data);
	void sfxctrl_w(offs_t offset, uint8_t data);

protected:
	exidy_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void common_sh_start();
	void common_sh_reset();

	void sh6840_register_state_globals();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	virtual s32 generate_music_sample();

	static inline void sh6840_apply_clock(sh6840_timer_channel *t, int clocks);

	/* sound streaming variables */
	sound_stream *m_stream;
	double m_freq_to_step;

private:
	// internal state
	sh6840_timer_channel m_sh6840_timer[3];
	int16_t m_sh6840_volume[3];
	uint8_t m_sh6840_MSB_latch;
	uint8_t m_sh6840_LSB_latch;
	uint8_t m_sh6840_LFSR_oldxor;
	uint32_t m_sh6840_LFSR_0;
	uint32_t m_sh6840_LFSR_1;
	uint32_t m_sh6840_LFSR_2;
	uint32_t m_sh6840_LFSR_3;
	uint32_t m_sh6840_clocks_per_sample;
	uint32_t m_sh6840_clock_count;

	uint8_t m_sfxctrl;

	inline int sh6840_update_noise(int clocks);
};

DECLARE_DEVICE_TYPE(EXIDY, exidy_sound_device)

class exidy_sh8253_sound_device : public exidy_sound_device
{
	struct sh8253_timer_channel
	{
		uint8_t   clstate = 0;
		uint8_t   enable = 0;
		uint16_t  count = 0;
		uint32_t  step = 0;
		uint32_t  fraction = 0;
	};

protected:
	exidy_sh8253_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual s32 generate_music_sample() override;

	void r6532_porta_w(uint8_t data);
	uint8_t r6532_porta_r();
	void r6532_portb_w(uint8_t data);
	uint8_t r6532_portb_r();

	void sh8253_w(offs_t offset, uint8_t data);

	void sh8253_register_state_globals();

	/* 8253 variables */
	sh8253_timer_channel m_sh8253_timer[3];

	/* 6532 variables */
	required_device<riot6532_device> m_riot;

	/* 5220/CVSD variables */
	optional_device<mc3417_device> m_cvsd;
	optional_device<filter_biquad_device> m_cvsd_filter;
	optional_device<filter_biquad_device> m_cvsd_filter2;
	optional_device<cpu_device> m_cvsdcpu;
	optional_device<tms5220_device> m_tms;
	required_device<pia6821_device> m_pia;
};

class venture_sound_device : public exidy_sh8253_sound_device
{
public:
	venture_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration access
	auto pa_callback() { return m_pa_callback.bind(); }
	auto pb_callback() { return m_pb_callback.bind(); }
	auto ca2_callback() { return m_ca2_callback.bind(); }
	auto cb2_callback() { return m_cb2_callback.bind(); }

	// external access
	void pa_w(uint8_t data);
	void pb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(ca_w);
	DECLARE_WRITE_LINE_MEMBER(cb_w);

protected:
	venture_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void venture_audio_map(address_map &map);

private:
	void filter_w(uint8_t data);

	void pia_pa_w(uint8_t data);
	void pia_pb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_cb2_w);

	void venture_audio(machine_config &config);

	devcb_write8 m_pa_callback;
	devcb_write8 m_pb_callback;
	devcb_write_line m_ca2_callback;
	devcb_write_line m_cb2_callback;
};

DECLARE_DEVICE_TYPE(EXIDY_VENTURE, venture_sound_device)

class mtrap_sound_device : public venture_sound_device
{
public:
	mtrap_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<timer_device> m_cvsd_timer;
	TIMER_DEVICE_CALLBACK_MEMBER(cvsd_timer);
	void voiceio_w(offs_t offset, uint8_t data);
	uint8_t voiceio_r(offs_t offset);
	bool m_cvsd_clk;

	void cvsd_map(address_map &map);
	void cvsd_iomap(address_map &map);
};

DECLARE_DEVICE_TYPE(EXIDY_MTRAP, mtrap_sound_device)

class victory_sound_device : public exidy_sh8253_sound_device
{
public:
	victory_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// external access
	uint8_t response_r();
	uint8_t status_r();
	void command_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_WRITE_LINE_MEMBER(irq_clear_w);
	DECLARE_WRITE_LINE_MEMBER(main_ack_w);

	void victory_audio_map(address_map &map);

	// internal state
	uint8_t m_victory_sound_response_ack_clk; /* 7474 @ F4 */

	TIMER_CALLBACK_MEMBER( delayed_command_w );

	int m_pia_ca1 = 0;
	int m_pia_cb1 = 0;
};

DECLARE_DEVICE_TYPE(EXIDY_VICTORY, victory_sound_device)

#endif // MAME_SHARED_EXIDYSOUND_H
