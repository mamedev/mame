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
	mpu401_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_device<m6801_cpu_device> m_ourcpu;

	template<class _write> void set_irqf(_write wr)
	{
		write_irq.set_callback(wr);
	}

	devcb_write_line write_irq;

	DECLARE_READ8_MEMBER(regs_mode2_r);
	DECLARE_WRITE8_MEMBER(regs_mode2_w);
	DECLARE_READ8_MEMBER(asic_r);
	DECLARE_WRITE8_MEMBER(asic_w);
	DECLARE_READ8_MEMBER(port1_r);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_READ8_MEMBER(port2_r);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_WRITE_LINE_MEMBER(midi_rx_w);

	// public API - call for reads/writes at I/O 330/331 on PC, C0n0/C0n1 on Apple II, etc.
	DECLARE_READ8_MEMBER(mpu_r);
	DECLARE_WRITE8_MEMBER(mpu_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	UINT8 m_port2;
	UINT8 m_command;
	UINT8 m_mpudata;
	UINT8 m_gatearrstat;
	emu_timer *m_timer;
};

// device type definition
extern const device_type MPU401;

#endif  /* __MPU401_H__ */
