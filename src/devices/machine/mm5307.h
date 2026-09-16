// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor MM5307 Baud Rate Generator/Programmable Divider

****************************************************************************
                                 ___    ___
              EXTERNAL FREQ   1 |*  \__/   | 14  ϕOUT
                         NC   2 |          | 13  RESET
                     OUTPUT   3 |          | 12  Vgg
                        Vss   4 |  MM5307  | 11  A
             EXTERNAL CLOCK   5 |          | 10  B
                    CRYSTAL   6 |          |  9  C
                    CRYSTAL   7 |__________|  8  D

***************************************************************************/

#ifndef MAME_MACHINE_MM5307_H
#define MAME_MACHINE_MM5307_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mm5307_device

class mm5307_device : public device_t
{
public:
	// configuration
	void set_ext_freq(u32 freq) { m_ext_freq = freq; }
	void set_ext_freq(const XTAL &freq) { m_ext_freq = freq.value(); }
	auto output_cb() { return m_output_cb.bind(); }

	// frequency control
	void control_w(u8 data);

protected:
	// base class constructor
	mm5307_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const std::array<u16, 16> &divisors_x2);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// timed update callback
	TIMER_CALLBACK_MEMBER(periodic_update);

	// internal divisors
	const std::array<u16, 16> &m_divisors_x2;

	// callbacks
	devcb_write_line m_output_cb;

	// external input frequency
	u32 m_ext_freq;

	// internal state
	u8 m_freq_control;
	u8 m_phase;

	// timer
	emu_timer *m_periodic_timer;
};

// ======================> mm5307aa_device

class mm5307aa_device : public mm5307_device
{
public:
	// construction/destruction
	mm5307aa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	static const std::array<u16, 16> s_divisors_x2;
};

// ======================> mm5307ab_device

class mm5307ab_device : public mm5307_device
{
public:
	// construction/destruction
	mm5307ab_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	static const std::array<u16, 16> s_divisors_x2;
};

// device type declarations
DECLARE_DEVICE_TYPE(MM5307AA, mm5307aa_device)
DECLARE_DEVICE_TYPE(MM5307AB, mm5307ab_device)

#endif // MAME_MACHINE_MM5307_H
