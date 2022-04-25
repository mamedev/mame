// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_MACHINE_NAMCO50_H
#define MAME_MACHINE_NAMCO50_H

#pragma once

#include "cpu/mb88xx/mb88xx.h"

/* device get info callback */
class namco_50xx_device : public device_t
{
public:
	namco_50xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	WRITE_LINE_MEMBER( reset );
	WRITE_LINE_MEMBER( chip_select );
	WRITE_LINE_MEMBER( rw );
	void write(uint8_t data);
	uint8_t read();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

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

#endif // MAME_MACHINE_NAMCO50_H
