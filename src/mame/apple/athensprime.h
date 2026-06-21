// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_ATHENSPRIME_H
#define MAME_APPLE_ATHENSPRIME_H

#pragma once

#include "machine/i2chle.h"

class athensprime_device : public device_t, public i2c_hle_interface
{
public:
	athensprime_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto pclock_changed() { return m_write_pclock.bind(); }
	auto sclock_changed() { return m_write_sclock.bind(); }

protected:
	// device_r overrides
	virtual void device_start() override ATTR_COLD;
	// i2c_hle_interface overrides
	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;
	virtual const char *get_tag() override { return tag(); }

private:
	enum
	{
		REG_ID = 0,
		REG_D2,
		REG_N2,
		REG_P2,
		REG_VENDOR,
		REG_D1,
		REG_N1,
		REG_P1,

		NUM_REGS
	};

	devcb_write32 m_write_pclock, m_write_sclock;
	u32 m_pclock, m_sclock;
	u8 m_regs[NUM_REGS];

	void update_pclock();
	void update_sclock();
};

DECLARE_DEVICE_TYPE(APPLE_ATHENSPRIME, athensprime_device)

#endif // MAME_APPLE_ATHENSPRIME_H
