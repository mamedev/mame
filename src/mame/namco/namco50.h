// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_NAMCO_NAMCO50_H
#define MAME_NAMCO_NAMCO50_H

#pragma once

#include "cpu/mb88xx/mb88xx.h"

/* device get info callback */
class namco_50xx_device : public device_t
{
public:
	namco_50xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void reset(int state);
	void chip_select(int state);
	void rw(int state);
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
	uint8_t                   m_rw;
	uint8_t                   m_cmd;
	uint8_t                   m_portO;

	TIMER_CALLBACK_MEMBER( O_w_sync );
	TIMER_CALLBACK_MEMBER( rw_sync );
	TIMER_CALLBACK_MEMBER( write_sync );

	uint8_t K_r();
	uint8_t R0_r();
	uint8_t R2_r();
	void O_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(NAMCO_50XX, namco_50xx_device)

#endif // MAME_NAMCO_NAMCO50_H
