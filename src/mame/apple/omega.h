// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_OMEGA_H
#define MAME_APPLE_OMEGA_H

#pragma once

class omega_device : public device_t
{
public:
	omega_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto pclock_changed() { return m_write_pclock.bind(); }

	// Omega uses the same 3-wire serial interface as DFAC, and is intended
	// to share the bus with DFAC.
	void data_write(int state) { m_data = state; }
	void clock_write(int state);
	void latch_write(int state);

protected:
	// device_r overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write32 m_write_pclock;
	bool m_data, m_clock, m_latch;
	u8 m_latch_byte, m_N, m_D, m_P;
	u32 m_bit;
};

DECLARE_DEVICE_TYPE(APPLE_OMEGA, omega_device)

#endif // MAME_APPLE_OMEGA_H
