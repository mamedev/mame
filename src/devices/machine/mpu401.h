// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __MPU401_H__
#define __MPU401_H__

#include "emu.h"
#include "cpu/m6800/m6800.h"

#define MCFG_MPU401_ADD(_tag, _irqf ) \
	MCFG_DEVICE_ADD(_tag, MPU401, 0) \
	MCFG_IRQ_FUNC(_irqf)

#define MCFG_IRQ_FUNC(_irqf) \
	downcast<mpu401_device *>(device)->set_irqf(DEVCB_##_irqf);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mpu401_device : public device_t
{
public:
	// construction/destruction
	mpu401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_device<m6801_cpu_device> m_ourcpu;

	template<class _write> void set_irqf(_write wr)
	{
		write_irq.set_callback(wr);
	}

	devcb_write_line write_irq;

	uint8_t regs_mode2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void regs_mode2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t asic_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void asic_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void midi_rx_w(int state);

	// public API - call for reads/writes at I/O 330/331 on PC, C0n0/C0n1 on Apple II, etc.
	uint8_t mpu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mpu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint8_t m_port2;
	uint8_t m_command;
	uint8_t m_mpudata;
	uint8_t m_gatearrstat;
	emu_timer *m_timer;
};

// device type definition
extern const device_type MPU401;

#endif  /* __MPU401_H__ */
