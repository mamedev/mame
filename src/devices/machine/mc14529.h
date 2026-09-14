// license:BSD-3-Clause
// copyright-holders:Tim Lindner
/***************************************************************************

    MC14529 Dual 4-Channel Analog Data Selector

    Emulation of the MC14529 data selector supporting propagation delays
    (tPLH/tPHL) and switching delays (tSW). Each selector (X and Y)
    operates independently in one of three modes:

      MODE_DIGITAL
        Four 1-bit inputs. Input changes apply after tPLH/tPHL propagation
        delays; address updates apply after a tSW switching delay.
        Output via write_line callback.

      MODE_ANALOG
        Four quantized multi-bit analog inputs. Input changes apply after
        tPLH/tPHL propagation delays based on whether the value is rising
        or falling; address updates apply after a tSW switching delay.
        Output via write8 callback.

      MODE_SOUND
        Four live audio inputs routed through device_sound_interface.
        Audio is switched sample-accurately. Supports an optional linear
        crossfade ramp duration to suppress switching clicks.

      Pinout:
        A, B          -> address_w()      (pins 6, 7)
		STROBE X      -> inhibit_x_w()    (pin 1)
		STROBE Y      -> inhibit_y_w()    (pin 15)

		X0-X3         -> x_w() / x_analog_w() / x_sound_input()   (pins 2-5)
		Y0-Y3         -> y_w() / y_analog_w() / y_sound_input()   (pins 11-14)

		Z             -> z_callback() / z_analog_callback() / z_sound_output()   (pin 9)
		W             -> w_callback() / w_analog_callback() / w_sound_output()   (pin 10)

		VDD = pin 16
		VSS = pin 8

    Todo / Unimplemented:
        * Continuous ideal analog switch resistance/slew behavior.
        * Combined 8-channel mode configuration.

***************************************************************************/

#ifndef MAME_MACHINE_MC14529_H
#define MAME_MACHINE_MC14529_H

#pragma once

class mc14529_device : public device_t, public device_sound_interface
{
public:
	enum mode_t : u8
	{
		MODE_DIGITAL = 0, // channel is a single bit
		MODE_ANALOG = 1,  // channel is a quantized multi-bit level
		MODE_SOUND = 2    // channel is a live audio stream (no propagation delay)
	};

	enum : u8
	{
		SEL_X = 0,
		SEL_Y = 1,
		NUM_SELECTORS = 2
	};

	mc14529_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// Sound crossfade (MODE_SOUND only). Zero = instant switch (original,
	// zero-cost behavior). Non-zero = linear ramp over that duration on
	// channel switch or inhibit change, to suppress the switching click.
	attotime m_sound_crossfade[NUM_SELECTORS];

	static constexpr u8 SOUND_SOURCE_SILENCE = 0xff;

	u8   m_sound_current_source[NUM_SELECTORS];   // settled source (channel index or SOUND_SOURCE_SILENCE)
	bool m_sound_fade_active[NUM_SELECTORS];
	sound_stream::sample_t m_sound_fade_from_sample[NUM_SELECTORS]; // snapshot at fade start
	u8   m_sound_fade_target[NUM_SELECTORS];       // channel index or SOUND_SOURCE_SILENCE
	u32  m_sound_fade_samples_total[NUM_SELECTORS];
	u32  m_sound_fade_samples_done[NUM_SELECTORS];
	mc14529_device &set_sound_crossfade(unsigned selector, const attotime &time);

	// configuration
	mc14529_device &set_propagation_delay(const attotime &tplh, const attotime &tphl);
	mc14529_device &set_switching_delay(const attotime &tsw);
	mc14529_device &set_mode(unsigned selector, mode_t mode);
	mc14529_device &set_analog_width(unsigned selector, unsigned bits); // default 8 bits

	// digital-mode outputs
	auto z_callback() { return m_write_z[SEL_X].bind(); }
	auto w_callback() { return m_write_z[SEL_Y].bind(); }
	auto address_changed_callback() { return m_write_address_changed.bind(); }

	// analog-mode outputs (quantized multi-bit value)
	auto z_analog_callback() { return m_write_z_analog[SEL_X].bind(); }
	auto w_analog_callback() { return m_write_z_analog[SEL_Y].bind(); }

	// polling reads of the last-committed digital/analog output (bit or
	// quantized level). Does not disturb any in-flight transition, same
	// as probing a real output pin's voltage. Not meaningful in MODE_SOUND.
	u8 zx_value();
	u8 zy_value();

	// sound-mode routing helpers: pass these as the input-index argument
	// to an upstream device's add_route() call, and these as the
	// output-index argument to this device's own add_route() call
	// (e.g. into a SPEAKER device).
	static constexpr int x_sound_input(unsigned channel) { return SEL_X * NUM_CHANNELS + channel; }
	static constexpr int y_sound_input(unsigned channel) { return SEL_Y * NUM_CHANNELS + channel; }
	static constexpr int x_sound_output() { return SEL_X; }
	static constexpr int w_sound_output() { return SEL_Y; }

	// shared address inputs (2-bit: bit 0 = A0, bit 1 = A1)
	void address_w(int bit, int state);
	void a0_w(int state) { address_w(0, state); }
	void a1_w(int state) { address_w(1, state); }
	int current_address() { return m_address; }

	// per-selector active-high inhibit
	void inhibit_x_w(int state);
	void inhibit_y_w(int state);

	// per-selector active-high inhibit
	bool inhibited_x() const { return m_inhibit[SEL_X]; }
	bool inhibited_y() const { return m_inhibit[SEL_Y]; }

	// digital-mode data inputs, by channel number (0-3); ignored unless
	// that selector is in MODE_DIGITAL
	void x_w(int channel, int state);
	void y_w(int channel, int state);

	void x0_w(int state) { x_w(0, state); }
	void x1_w(int state) { x_w(1, state); }
	void x2_w(int state) { x_w(2, state); }
	void x3_w(int state) { x_w(3, state); }
	void y0_w(int state) { y_w(0, state); }
	void y1_w(int state) { y_w(1, state); }
	void y2_w(int state) { y_w(2, state); }
	void y3_w(int state) { y_w(3, state); }

	// analog-mode data inputs (quantized multi-bit value), by channel
	// number (0-3); ignored unless that selector is in MODE_ANALOG
	void x_analog_w(int channel, u8 value);
	void y_analog_w(int channel, u8 value);

	void x0_analog_w(u8 value) { x_analog_w(0, value); }
	void x1_analog_w(u8 value) { x_analog_w(1, value); }
	void x2_analog_w(u8 value) { x_analog_w(2, value); }
	void x3_analog_w(u8 value) { x_analog_w(3, value); }
	void y0_analog_w(u8 value) { y_analog_w(0, value); }
	void y1_analog_w(u8 value) { y_analog_w(1, value); }
	void y2_analog_w(u8 value) { y_analog_w(2, value); }
	void y3_analog_w(u8 value) { y_analog_w(3, value); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_sound_interface
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned NUM_CHANNELS = 4;

	// fixed-size pool of in-flight transitions per selector (MODE_DIGITAL /
	// MODE_ANALOG only); sized with generous headroom relative to real
	// host timing (transitions would have to arrive faster than one every
	// ~tPHL, i.e. faster than every ~200 ns of emulated time, to exhaust
	// this)
	static constexpr unsigned MAX_PENDING = 4;

 	void switch_selector(unsigned selector);
	void queue_switch_update(unsigned new_address); // channel/inhibit change: may incur tPLH/tPHL
	void update_sound_target(unsigned address);
	void queue_value_update(unsigned selector, unsigned channel, u8 value);
	TIMER_CALLBACK_MEMBER(delay_expired);
	inline void update_selector_stream(sound_stream &stream, unsigned selector);
	inline void finalize_sample_capture(sound_stream &stream, unsigned selector);

	devcb_write_line m_write_z[NUM_SELECTORS];        // MODE_DIGITAL output
	devcb_write8 m_write_z_analog[NUM_SELECTORS];     // MODE_ANALOG output
	devcb_write8 m_write_address_changed;             // All address changed events

	sound_stream *m_stream; // MODE_SOUND input/output

	attotime m_tplh;   // rising-swing propagation delay
	attotime m_tphl;   // falling-swing propagation delay
	attotime m_tsw;    // switching propagation delay

	mode_t m_mode[NUM_SELECTORS];
	u8 m_width_mask[NUM_SELECTORS]; // e.g. 0x3f for a 6-bit quantized level

	u8 m_channel[NUM_SELECTORS][NUM_CHANNELS];       // MODE_DIGITAL: X0-X3/Y0-Y3 bit values
	u8 m_channel_value[NUM_SELECTORS][NUM_CHANNELS]; // MODE_ANALOG: X0-X3/Y0-Y3 quantized values

	u8 m_address;   // 2-bit shared address (bit0=A0, bit1=A1)
	u8 m_inhibit[NUM_SELECTORS]; // active high

	u8 m_current_output[NUM_SELECTORS]; // last output actually driven (MODE_DIGITAL/MODE_ANALOG)
	u8 m_last_scheduled[NUM_SELECTORS][NUM_CHANNELS]; // target value of most recently queued (or committed) transition

	// used when SOUND channel is inhibited
	sound_stream::sample_t m_last_stream_sampel[NUM_SELECTORS];

	// fixed pool of pending transitions per selector
	emu_timer *m_pending_timer[MAX_PENDING];
	u8         m_pending_value[NUM_SELECTORS][NUM_CHANNELS];
	bool       m_pending_active[MAX_PENDING];
	unsigned   m_pending_next; // round-robin allocation index
	unsigned   m_last_queued_address;

	static constexpr unsigned VALUE_WIDTH   = 8;
	static constexpr unsigned SWITCH_WIDTH  = 1;
	static constexpr unsigned SLOT_BITS     = std::bit_width(MAX_PENDING - 1U);
	static constexpr unsigned CHANNEL_BITS  = std::bit_width(NUM_CHANNELS - 1U);
	static constexpr unsigned SELECTOR_BITS = std::bit_width(NUM_SELECTORS - 1U);

	union PackedData {
		int32_t raw;
		struct {
			uint32_t value    : VALUE_WIDTH;
			uint32_t channel  : CHANNEL_BITS;
			uint32_t sw       : SWITCH_WIDTH;
			uint32_t selector : SELECTOR_BITS;
			uint32_t slot     : SLOT_BITS;
			uint32_t padding  : (32 - (VALUE_WIDTH + CHANNEL_BITS + SWITCH_WIDTH + SELECTOR_BITS + SLOT_BITS));
		} bits;
	};

	int32_t pack(unsigned slot, unsigned selector, unsigned channel, bool sw, uint8_t value);
};

DECLARE_DEVICE_TYPE(MC14529, mc14529_device)

#endif // MAME_MACHINE_MC14529_H
