// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_MACHINE_NAMCO53_H
#define MAME_MACHINE_NAMCO53_H

#pragma once

#include "cpu/mb88xx/mb88xx.h"

class namco_53xx_device : public device_t
{
public:
	namco_53xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto input_callback() { return m_in[N].bind(); }

	auto k_port_callback() { return m_k.bind(); }
	auto p_port_callback() { return m_p.bind(); }

	DECLARE_WRITE_LINE_MEMBER(read_request);
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	TIMER_CALLBACK_MEMBER( irq_clear );

private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	uint8_t           m_portO;
	devcb_read8    m_k;
	devcb_read8    m_in[4];
	devcb_write8   m_p;
	emu_timer *m_irq_cleared_timer;

	DECLARE_READ8_MEMBER( K_r );
	DECLARE_READ8_MEMBER( R0_r );
	DECLARE_READ8_MEMBER( R1_r );
	DECLARE_READ8_MEMBER( R2_r );
	DECLARE_READ8_MEMBER( R3_r );
	DECLARE_WRITE8_MEMBER( O_w );
	DECLARE_WRITE8_MEMBER( P_w );
};

DECLARE_DEVICE_TYPE(NAMCO_53XX, namco_53xx_device)


#endif // MAME_MACHINE_NAMCO53_H
