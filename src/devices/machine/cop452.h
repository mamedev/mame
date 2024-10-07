// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/***************************************************************************

    cop452.h

    Frequency generator & counter

****************************************************************************
                                ____   ____
                        ZO   1 |*   \_/    | 14   ZI
                        OA   2 |           | 13   DO
                        INB  3 |           | 12   DI
                        ENB  4 |  COP452   | 11   SK
                        OB   5 |           | 10  CS/
                        Vcc  6 |           |  9  GND
                        CKO  7 |___________|  8  CKI

***************************************************************************/

#ifndef MAME_MACHINE_COP452_H
#define MAME_MACHINE_COP452_H

#pragma once

class cop452_device : public device_t
{
public:
	cop452_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// SPI I/O
	void cs_w(int state);
	void sk_w(int state);
	void di_w(int state);
	int do_r();

	// Signal outputs
	auto oa_w() { return m_out_handlers[0].bind(); }
	auto ob_w() { return m_out_handlers[1].bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(timer_tick);

private:
	// Index 0 is for "A" side, 1 is for "B" side
	devcb_write_line::array<2> m_out_handlers;

	// Timers
	emu_timer *m_timers[2];

	// State
	uint8_t m_mode;
	bool m_clk_div_4;
	bool m_cs;
	bool m_sk;
	bool m_di;
	bool m_out[2];
	bool m_regA_b16;
	uint16_t m_reg[2];
	uint16_t m_cnt[2];
	// 0    : idle
	// 1..5 : shifting instruction in
	// 6..21: shifting register in
	// >= 22: done
	unsigned m_spi_state;
	uint8_t m_sr;

	// Operating modes
	enum : uint8_t {
		MODE_DUAL_FREQ,     // 0000 Dual frequency
		MODE_TRIG_PULSE,    // 0001 Triggered pulse
		MODE_NUMBER_PULSES, // 0010 Number of pulses
		MODE_DUTY_CYCLE,    // 0011 Duty cycle
		MODE_FREQ_COUNT,    // 0100 Frequency and count
		MODE_DUAL_COUNT,    // 0101 Dual count
		MODE_WAVE_MEAS,     // 0110 Waveform measurement
		MODE_TRIG_COUNT,    // 0111 Triggered pulse and count
		MODE_WHITE_NOISE,   // 1000 White noise & frequency
		MODE_GATED_WHITE,   // 1001 Gated white noise
		MODE_RESET = 15     // 1111 Reset
	};

	attotime counts_to_attotime(unsigned counts) const;
	void set_timer(unsigned idx);
	void set_output(unsigned idx, bool state);
	void toggle_output(unsigned idx);
	void toggle_n_reload(unsigned idx);
};

// device type definition
DECLARE_DEVICE_TYPE(COP452, cop452_device)

#endif // MAME_MACHINE_COP452_H
