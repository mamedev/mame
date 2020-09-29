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
	// this base device implements common things across all the CVSD chips

	auto clock_state_cb() { return m_clock_state_push_cb.bind(); } // A clock state change callback. Using this is a bad idea due to lack of synchronization to other devices. TODO: remove this.
	auto digin_cb() { return m_digin_pull_cb.bind(); } // Digital in pull callback function, for use if a clock is specified and we need to pull in the digital pin state, otherwise unused. TODO: this is not hooked up yet, and should be.
	auto digout_cb() { return m_digout_push_cb.bind(); } // Digital out push callback function. TODO: this is not hooked up or implemented yet, although it is only really relevant for devices which use the CVSD chips in encode mode.

	READ_LINE_MEMBER( clock_r ); // Clock pull, really only relevant of something manually polls the clock (and clock is specified), which is a very bad design pattern and will cause synchronization/missed clock transition issues. This function WILL ASSERT if it is called and the clock hz is NOT specified! TODO: remove all use of this, and remove it.
	WRITE_LINE_MEMBER( mclock_w ); // Clock push; this function WILL ASSERT if it is called and the clock hz IS specified!
	WRITE_LINE_MEMBER( digin_w ); // Digital in push to the pin, as a pseudo 'buffer' implemented within the cvsd device itself. This is not technically accurate to hardware, and in the future should be deprecated in favor of digin_cb once the latter is implemented.
	WRITE_LINE_MEMBER( dec_encq_w ); //DEC/ENC decode/encode select push. This is not implemented yet, and relies on an input audio stream. TODO: implement this beyond a do-nothing stub
	READ_LINE_MEMBER( digout_r ); // Digital out pull. TODO: this is not hooked up or implemented yet, although it is only really relevant for devices which use the CVSD chips in encode mode.
	//void audio_in_w(stream_buffer::sample_t data); // Audio In pin, an analog value of the audio waveform being pushed to the chip. TODO: this is not hooked up or implemented yet, and this should really be handled as an input stream from a separate DAC device, not a value push function at all.
	void digit_w(int digit); /* sets the buffered digit (0 or 1), common to all chips. TODO: replace all use of this with digin_cb once implemented */
	void clock_w(int state); /* sets the clock state (0 or 1, clocked on the rising edge), common to all chips */
	virtual int clock_state_r(); /* returns whether the clock is currently LO or HI, common to all chips. TODO: get rid of all use of this, then get rid of it. */

protected:
	cvsd_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	cvsd_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	//virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// callbacks
	devcb_write_line m_clock_state_push_cb; ///TODO: get rid of this, if you use it you should feel bad
	devcb_read_line m_digin_pull_cb;
	devcb_write_line m_digout_push_cb;

	// internal state
	sound_stream *m_stream;
	bool      m_active_clock_edge;
	uint8_t   m_shiftreg_mask;
	bool      m_last_clock_state;
	bool      m_buffered_bit;
	uint8_t   m_shiftreg;
	stream_buffer::sample_t m_curr_sample;
	stream_buffer::sample_t m_next_sample;
	uint32_t  m_samples_generated;

	// specific internal handler overrides, overridden by each chip
	virtual void process_bit(bool bit, bool clock_state);

	///TODO: get rid of these
	inline bool is_external_oscillator();
	inline bool is_clock_changed(bool clock_state);
	inline bool is_active_clock_transition(bool clock_state);
	inline bool current_clock_state();

};


class hc55516_device : public cvsd_device_base
{
public:
	hc55516_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto fzq_cb() { return m_fzq_pull_cb.bind(); } 	// /FZ (partial reset) pull callback, ok to leave unconnected (we assume it is pulled high)
	auto agc_cb() { return m_agc_push_cb.bind(); } 	// AGC callback function, called to push the state if the AGC pin changes, ok to leave unconnected

	WRITE_LINE_MEMBER( fzq_w );	// /FZ (partial reset) push
	READ_LINE_MEMBER( agc_r ); // AGC pull

/* only relevant for encode mode, which isn't done yet!
	// /APT (silence encoder output) push
	WRITE_LINE_MEMBER( aptq_w );
	// DEC/ENC decode/encode select push
	WRITE_LINE_MEMBER( dec_encq_w );

*/

protected:
	// overridable type for subclass
	hc55516_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// callbacks
	devcb_write_line m_agc_push_cb;
	devcb_read_line m_fzq_pull_cb;

	// coefficients
	int32_t m_sylmask;
	int32_t m_sylshift;
	int32_t m_syladd;
	int32_t m_intshift;

	// internal state
	int32_t m_sylfilter;
	int32_t m_intfilter;
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
	virtual void device_start() override;
	virtual void device_reset() override;
};


class mc3417_device : public cvsd_device_base
{
public:
	mc3417_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// override for clock_w
	//virtual void clock_w(int state) override;

protected:
	// overridable type for subclass
	mc3417_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	double m_sylfilter_d;
	double m_intfilter_d;
	double m_charge;
	double m_decay;
	double m_leak;

	// internal handlers
	virtual void process_bit(bool bit, bool clock_state) override;
};


class mc3418_device : public mc3417_device
{
public:
	mc3418_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	// device-level overrides
	virtual void device_start() override;
};


DECLARE_DEVICE_TYPE(HC55516, hc55516_device)
DECLARE_DEVICE_TYPE(HC55532, hc55532_device)
//DECLARE_DEVICE_TYPE(HC55536, hc55536_device)
//DECLARE_DEVICE_TYPE(HC55564, hc55564_device)
DECLARE_DEVICE_TYPE(MC3417,  mc3417_device)
//DECLARE_DEVICE_TYPE(MC34115, mc34115_device)
DECLARE_DEVICE_TYPE(MC3418,  mc3418_device)

#endif // MAME_SOUND_HC55516_H
