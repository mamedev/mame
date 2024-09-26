// license:BSD-3-Clause
// copyright-holders:Mike Harris, Aaron Giles
#ifndef MAME_NAMCO_NAMCO51_H
#define MAME_NAMCO_NAMCO51_H

#pragma once

#include "cpu/mb88xx/mb88xx.h"


class namco_51xx_device : public device_t
{
public:
	namco_51xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto input_callback() { return m_in[N].bind(); }
	auto output_callback() { return m_out.bind(); }
	auto lockout_callback() { return m_lockout.bind(); }

	void reset(int state);
	void vblank(int state);
	void rw(int state);
	void chip_select(int state);
	void write(uint8_t data);
	uint8_t read();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	uint8_t m_portO;
	uint8_t m_rw;
	devcb_read8::array<4> m_in;
	devcb_write8 m_out;
	devcb_write_line m_lockout;

	uint8_t K_r();
	uint8_t R0_r();
	uint8_t R1_r();
	uint8_t R2_r();
	uint8_t R3_r();
	void O_w(uint8_t data);
	void P_w(uint8_t data);

	TIMER_CALLBACK_MEMBER( rw_sync );
	TIMER_CALLBACK_MEMBER( write_sync );
	TIMER_CALLBACK_MEMBER( O_w_sync );
};

DECLARE_DEVICE_TYPE(NAMCO_51XX, namco_51xx_device)

#endif // MAME_NAMCO_51XX
