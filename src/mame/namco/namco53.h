// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_NAMCO_NAMCO53_H
#define MAME_NAMCO_NAMCO53_H

#pragma once

#include "cpu/mb88xx/mb88xx.h"

class namco_53xx_device : public device_t
{
public:
	namco_53xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto input_callback() { return m_in[N].bind(); }

	auto k_port_callback() { return m_k.bind(); }
	auto p_port_callback() { return m_p.bind(); }

	void reset(int state);
	void chip_select(int state);
	uint8_t read();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	uint8_t        m_portO;
	devcb_read8    m_k;
	devcb_read8::array<4> m_in;
	devcb_write8   m_p;

	uint8_t K_r();
	uint8_t R0_r();
	uint8_t R1_r();
	uint8_t R2_r();
	uint8_t R3_r();
	void O_w(uint8_t data);
	void P_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(NAMCO_53XX, namco_53xx_device)


#endif // MAME_NAMCO_NAMCO53_H
