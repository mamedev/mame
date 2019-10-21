// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_MPU401_H
#define MAME_MACHINE_MPU401_H

#pragma once

#include "cpu/m6800/m6801.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mpu401_device : public device_t
{
public:
	// construction/destruction
	mpu401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_cb() { return write_irq.bind(); }

	// public API - call for reads/writes at I/O 330/331 on PC, C0n0/C0n1 on Apple II, etc.
	DECLARE_READ8_MEMBER(mpu_r);
	DECLARE_WRITE8_MEMBER(mpu_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_WRITE_LINE_MEMBER(midi_rx_w);

	DECLARE_READ8_MEMBER(regs_mode2_r);
	DECLARE_WRITE8_MEMBER(regs_mode2_w);
	DECLARE_READ8_MEMBER(asic_r);
	DECLARE_WRITE8_MEMBER(asic_w);
	DECLARE_READ8_MEMBER(port1_r);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_READ8_MEMBER(port2_r);
	DECLARE_WRITE8_MEMBER(port2_w);

	void mpu401_map(address_map &map);

	required_device<m6801_cpu_device> m_ourcpu;

	devcb_write_line write_irq;

	uint8_t m_port2;
	uint8_t m_command;
	uint8_t m_mpudata;
	uint8_t m_gatearrstat;
	emu_timer *m_timer;
};

// device type definition
DECLARE_DEVICE_TYPE(MPU401, mpu401_device)

#endif // MAME_MACHINE_MPU401_H
