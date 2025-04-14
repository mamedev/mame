// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Jonathan Gevaryahu
// thanks-to:Zonn Moore
#ifndef MAME_SOUND_HC55516_H
#define MAME_SOUND_HC55516_H

#pragma once

class cvsd_device_base : public device_t, public device_sound_interface
{
public:
	enum
	{
		RISING=true,
		FALLING=false
	};

	// A clock state change callback. Using this is a bad idea due to lack of synchronization to other devices.
	// TODO: remove this.
	auto clock_state_cb() { return m_clock_state_push_cb.bind(); }

	// Digital in pull callback function, for use if a clock is specified and we need to pull in the digital
	// in pin state, otherwise unused. TODO: this is not hooked up yet, and should be.
	auto digin_cb() { return m_digin_pull_cb.bind(); }

	// Digital out push callback function. TODO: this is not hooked up or implemented yet, although it
	// is only really relevant for devices which use the CVSD chips in encode mode.
	auto digout_cb() { return m_digout_push_cb.bind(); }

	// Clock pull, really only relevant of something manually polls the clock (and clock is specified),
	// which is a very bad design pattern and will cause synchronization/missed clock transition issues.
	// This function WILL ASSERT if it is called and the clock hz is NOT specified!
	// TODO: remove all use of this, and remove it.
	int clock_r();

	// Clock push; this function WILL ASSERT if it is called and the clock hz IS specified!
	void mclock_w(int state);

	// Digital in push to the pin, as a pseudo 'buffer' implemented within the cvsd device itself.
	// This is not technically accurate to hardware, and in the future should be deprecated in favor of
	// digin_cb once the latter is implemented.
	void digin_w(int state);

	// DEC/ENC decode/encode select push. This is not implemented yet, and relies on an input audio stream.
	// TODO: implement this beyond a do-nothing stub
	void dec_encq_w(int state);

	// Digital out pull. TODO: this is not hooked up or implemented yet, although it is only really
	// relevant for devices which use the CVSD chips in encode mode.
	int digout_r();

	// Audio In pin, an analog value of the audio waveform being pushed to the chip.
	// TODO: this is not hooked up or implemented yet, and this should really be handled as an
	// input stream from a separate DAC device, not a value push function at all.
	//void audio_in_w(sound_stream::sample_t data);

	// sets the buffered digit (0 or 1), common to all chips. TODO: replace all use of this with
	// digin_cb once implemented
	void digit_w(int digit);

	// sets the clock state (0 or 1, clocked on the rising edge), common to all chips
	void clock_w(int state);

	// returns whether the clock is currently LO or HI, common to all chips.
	// TODO: get rid of all use of this, then get rid of it.
	virtual int clock_state_r();

protected:
	cvsd_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool active_clock_edge, uint8_t shiftreg_mask);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	//virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// callbacks
	devcb_write_line m_clock_state_push_cb; // TODO: get rid of this, if you use it you should feel bad
	devcb_read_line m_digin_pull_cb;
	devcb_write_line m_digout_push_cb;

	// const state defined by constructor
	const bool m_active_clock_edge;
	const uint8_t m_shiftreg_mask; // it may be desirable to allow this to be changed by the user under some circumstances

	// internal state
	sound_stream *m_stream;
	bool m_last_clock_state;
	bool m_buffered_bit;
	uint8_t m_shiftreg;
	uint32_t m_samples_generated;

	// specific internal handler overrides, overridden by each chip
	virtual void process_bit(bool bit, bool clock_state);

	// TODO: get rid of these
	inline bool is_external_oscillator();
	inline bool is_clock_changed(bool clock_state);
	inline bool is_active_clock_transition(bool clock_state);
	inline bool current_clock_state();
};


class hc55516_device : public cvsd_device_base
{
public:
	hc55516_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// /FZ (partial reset) pull callback, ok to leave unconnected (we assume it is pulled high)
	auto fzq_cb() { return m_fzq_pull_cb.bind(); }

	// AGC callback function, called to push the state if the AGC pin changes, ok to leave unconnected
	auto agc_cb() { return m_agc_push_cb.bind(); }

	void fzq_w(int state); // /FZ (partial reset) push
	int agc_r(); // AGC pull
	// TODO: These are only relevant for encode mode, which isn't done yet!
	//void aptq_w(int state); // /APT (silence encoder output) push
	//void dec_encq_w(int state); // DEC/ENC decode/encode select push

protected:
	// overridable type for subclass
	hc55516_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t sylmask, int32_t sylshift, int32_t syladd, int32_t intshift);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// callbacks
	devcb_write_line m_agc_push_cb;
	devcb_read_line m_fzq_pull_cb;

	// const coefficients defined by constructor
	const uint32_t m_sylmask;
	const int32_t m_sylshift;
	const int32_t m_syladd;
	const int32_t m_intshift;

	// internal state
	int32_t m_sylfilter;
	int32_t m_intfilter;
	int16_t m_next_sample;
	bool m_agc;
	bool m_buffered_fzq;

	// internal handlers
	virtual void process_bit(bool bit, bool clock_state) override;
};


class hc55532_device : public hc55516_device
{
public:
	hc55532_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;
};


class mc3417_device : public cvsd_device_base
{
public:
	mc3417_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// override for clock_w
	//virtual void clock_w(int state) override;

protected:
	// overridable type for subclass
	mc3417_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t shiftreg_mask);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// const coefficients defined by constructor; should these be adjustable by the user or externally defined,
	// as they are implemented using a set of two small lowpass filters outside the chip?
	const double m_charge;
	const double m_decay;
	const double m_leak;

	// internal state
	double m_sylfilter;
	double m_intfilter;
	sound_stream::sample_t m_curr_sample;
	sound_stream::sample_t m_next_sample;

	// internal handlers
	virtual void process_bit(bool bit, bool clock_state) override;
};


class mc3418_device : public mc3417_device
{
public:
	mc3418_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(HC55516, hc55516_device)
DECLARE_DEVICE_TYPE(HC55532, hc55532_device)
//DECLARE_DEVICE_TYPE(HC55536, hc55536_device)
//DECLARE_DEVICE_TYPE(HC55564, hc55564_device)
DECLARE_DEVICE_TYPE(MC3417,  mc3417_device)
//DECLARE_DEVICE_TYPE(MC34115, mc34115_device)
DECLARE_DEVICE_TYPE(MC3418,  mc3418_device)

#endif // MAME_SOUND_HC55516_H
