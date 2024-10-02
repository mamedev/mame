// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Philips MEA 8000 emulation.

**********************************************************************/

#ifndef MAME_SOUND_MEA8000_H
#define MAME_SOUND_MEA8000_H

#pragma once

/* define to use double instead of int (slow but useful for debugging) */
#undef MEA8000_FLOAT_MODE

class mea8000_device : public device_t, public device_sound_interface
{
public:
	mea8000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto req() { return m_write_req.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	/* filter coefficients from frequencies */
	static constexpr unsigned TABLE_LEN = 3600;

	/* noise generator table */
	static constexpr unsigned NOISE_LEN = 8192;

	/* finite machine state controlling frames */
	enum class mea8000_state : u8
	{
		STOPPED,    /* nothing to do, timer disabled */
		WAIT_FIRST, /* received pitch, wait for first full frame, timer disabled */
		STARTED,    /* playing a frame, timer on */
		SLOWING     /* repeating last frame with decreasing amplitude, timer on */
	};

	struct filter_t
	{
#ifdef MEA8000_FLOAT_MODE
		double fm, last_fm;         /* frequency, in Hz */
		double bw, last_bw;         /* bandwidth, in Hz */
		double output, last_output; /* filter state */
#else
		uint16_t fm = 0, last_fm = 0;
		uint16_t bw = 0, last_bw = 0;
		int32_t  output = 0, last_output = 0;
#endif
	};

	int accept_byte();
	void update_req();
	void init_tables();
#ifndef MEA8000_FLOAT_MODE /* uint16_t version */
	int interp(uint16_t org, uint16_t dst);
	int filter_step(int i, int input);
	int noise_gen();
	int freq_gen();
	int compute_sample();
#else /* float version */
	double interp(double org, double dst);
	double filter_step(int i, double input);
	double noise_gen();
	double freq_gen();
	double compute_sample();
#endif

	void shift_frame();
	void decode_frame();
	void start_frame();
	void stop_frame();

	TIMER_CALLBACK_MEMBER(timer_expire);

	devcb_write8 m_write_req;

	/* state */
	mea8000_state m_state = mea8000_state::STOPPED; /* current state */

	uint8_t m_buf[4] = { 0, 0, 0, 0 }; /* store 4 consecutive data to form a frame info */
	uint8_t m_bufpos = 0; /* new byte to write in frame info buffer */

	uint8_t m_cont = 0; /* if no data 0=stop 1=repeat last frame */
	uint8_t m_roe = 0;  /* enable req output, now unimplemented */

	uint16_t m_framelength = 0;  /* in samples */
	uint16_t m_framepos = 0;     /* in samples */
	uint16_t m_framelog = 0;     /* log2 of framelength */

	int16_t m_lastsample = 0, m_sample = 0; /* output samples are interpolated */

	uint32_t m_phi = 0; /* absolute phase for frequency / noise generator */

	filter_t m_f[4]; /* filters */

	uint16_t m_last_ampl = 0, m_ampl = 0;    /* amplitude * 1000 */
	uint16_t m_last_pitch = 0, m_pitch = 0;  /* pitch of sawtooth signal, in Hz */
	uint8_t  m_noise = 0;

	emu_timer *m_timer = nullptr;
	sound_stream * m_stream = nullptr;
	int32_t m_output = 0;

	int m_cos_table[TABLE_LEN];  /* fm => cos coefficient */
	int m_exp_table[TABLE_LEN];  /* bw => exp coefficient */
	int m_exp2_table[TABLE_LEN]; /* bw => 2*exp coefficient */
	int m_noise_table[NOISE_LEN];

};

DECLARE_DEVICE_TYPE(MEA8000, mea8000_device)

#endif // MAME_SOUND_MEA8000_H
