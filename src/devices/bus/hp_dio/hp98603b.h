// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#ifndef MAME_BUS_HPDIO_98603B_H
#define MAME_BUS_HPDIO_98603B_H

#pragma once

#include "hp_dio.h"

class dio16_98603b_device :
		public device_t,
		public device_dio16_card_interface
{
public:
	// construction/destruction
	dio16_98603b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ16_MEMBER(rom_r);
	DECLARE_WRITE16_MEMBER(rom_w);

protected:
	dio16_98603b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	uint8_t *m_rom;
};

DECLARE_DEVICE_TYPE(HPDIO_98603B, dio16_98603b_device)

#endif // MAME_BUS_HPDIO_98603B_H
