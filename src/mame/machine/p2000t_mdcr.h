// license:BSD-3-Clause
// copyright-holders:Erwin Jansen
/**********************************************************************

    Philips P2000t  Mini Digital Cassette Recorder Emulation

**********************************************************************

                    +12V      1      8       !WCD
            OV (signal)       2      9       !REV
            OV (power)        3      A       !FWD
                    GND       4      B       RDC
                   !WDA       6      C       !RDA
                   !BET       7      D       !CIP
                                     E       !WEN

**********************************************************************/

#ifndef MAME_MACHINE_P2000T_MDCR_H
#define MAME_MACHINE_P2000T_MDCR_H

#pragma once

#include "imagedev/cassette.h"

/// \brief Models a MCR220 Micro Cassette Recorder
///
/// Detailed documentation on the device can be found in this repository:
/// https://github.com/p2000t/documentation/tree/master/hardware
///
/// This device was built in the P2000t and was used as tape storage.
/// It used mini-cassettes that were also used in dictation devices.
/// The P2000t completely controls this device without user intervention.
class mdcr_device : public device_t
{
public:
	mdcr_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	/// \brief The read clock, switches state when a bit is available.
	///
	/// This is the read clock. The read clock is a flip flop that switches
	/// whenever a new bit is available on rda. The original flips every 167us
	/// This system flips at 154 / 176 usec, which is within the tolerance for
	/// the rom and system diagnostics.
	///
	/// Note that rdc & rda are flipped when the tape is moving in reverse.
	DECLARE_READ_LINE_MEMBER(rdc);

	/// The current active data bit.
	DECLARE_READ_LINE_MEMBER(rda);

	/// False indicates we have reached end/beginning of tape
	DECLARE_READ_LINE_MEMBER(bet);

	/// False if a cassette is in place.
	DECLARE_READ_LINE_MEMBER(cip);

	/// False when the cassette is write enabled.
	DECLARE_READ_LINE_MEMBER(wen);

	/// True if we should activate the reverse motor.
	DECLARE_WRITE_LINE_MEMBER(rev);

	/// True if we should activate the forward motor.
	/// Note: A quick pulse (<20usec) will reset the phase decoder.
	DECLARE_WRITE_LINE_MEMBER(fwd);

	/// The bit to write to tape. Make sure to set wda after wdc.
	DECLARE_WRITE_LINE_MEMBER(wda);

	/// True if the current wda should be written to tape.
	DECLARE_WRITE_LINE_MEMBER(wdc);

protected:
	virtual void device_start() override;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	virtual void device_add_mconfig(machine_config &config) override;

private:
	/// \brief A Phase Decoder used in a Philips MDCR220 Mini Cassette Recorder
	///
	/// A phase decoder is capable of converting a signal stream into a
	/// a series of bits that go together with a clock signal. This phase
	/// decoder is conform to what you would find in an Philips MDCR220
	///
	/// Signals are converted into bits whenever the line signal
	/// changes from low to high and vice versa on a clock signal.
	///
	/// A transition on a clock boundary from low to high is a 1.
	/// A transition on a clock boundary from high to low is a 0
	/// An intermediate transition halfway between the clock boundary
	/// can occur when there are consecutive 0s or 1s. See the example
	/// below where the clock is marked by a |
	///
	///          1    0    1    1    0    0
	///   RDA:  _|----|____|--__|----|__--|__--
	///   RDC:  _|-___|-___|-___|-___|-___|-___
	///          ^                      ^
	///          |-- clock signal       |-- intermediate transition.
	///
	/// This particular phase decoder expects a signal of
	/// 1010 1010 which is used to derive the clock T.
	/// after a reset.
	class phase_decoder
	{
		using time_in_seconds = double;

	public:
		/// Creates a phase decoder with the given tolerance.
		phase_decoder(double tolerance = 0.15);

		/// Pulls the bit out of the queue.
		bool pull_bit();

		/// Returns true if a new bit can be read, you can now pull the bit.
		bool signal(bool state, double delay);

		/// Reset the clock state, the system will now need to resynchronize on 0xAA.
		void reset();

	private:
		// add a bit and reset the current clock.
		void add_bit(bool bit);

		// tries to sync up the signal and calculate the clockperiod.
		bool sync_signal(bool state);

		// y * (1 - tolerance) < x < y * (1 + tolerance)
		bool within_tolerance(double x, double y);

		double m_tolerance;

		static constexpr int SYNCBITS    = 7;
		static constexpr int QUEUE_DELAY = 2;

	public:
		// Needed for save state.
		bool m_last_signal{ false };
		int m_needs_sync{ SYNCBITS };
		uint8_t m_bit_queue{ 0 };
		uint8_t m_bit_place{ 0 };
		time_in_seconds m_current_clock{ 0 };
		time_in_seconds m_clock_period{ 0 };
	};


	void write_bit(bool bit);
	void rewind();
	void forward();
	void stop();
	bool tape_start_or_end();

	bool m_fwd{ false };
	bool m_rev{ false };
	bool m_rdc{ false };
	bool m_rda{ false };
	bool m_wda{ false };

	bool m_recording{ false };
	double m_fwd_pulse_time{ 0 };
	double m_last_tape_time{ 0 };
	double m_save_tape_time{ 0 };

	required_device<cassette_image_device> m_cassette;
	phase_decoder m_phase_decoder;

	// timers
	emu_timer *m_read_timer;
};


DECLARE_DEVICE_TYPE(MDCR, mdcr_device)

#endif // MAME_MACHINE_P2000T_MDCR_H
