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
	uint8_t mpu_r(offs_t offset);
	void mpu_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(serial_tick);

private:
	void midi_rx_w(int state);

	uint8_t asic_r(offs_t offset);
	void asic_w(offs_t offset, uint8_t data);
	uint8_t port1_r();
	void port1_w(uint8_t data);
	uint8_t port2_r();
	void port2_w(uint8_t data);

	void mpu401_map(address_map &map) ATTR_COLD;

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
