// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_SEGAAI_ROM_H
#define MAME_BUS_SEGAAI_ROM_H

#pragma once

#include "segaai_slot.h"

class segaai_rom_128_device : public device_t,
								public segaai_card_interface
{
public:
	segaai_rom_128_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	segaai_rom_128_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override { };
};


class segaai_rom_256_device : public segaai_rom_128_device
{
public:
	segaai_rom_256_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void install_memory_handlers(address_space *space) override;

private:
	template <int Bank> void bank_w(u8 data);
	void unknown0_w(u8 data);
	void unknown1_w(u8 data);

	memory_bank_array_creator<2> m_rom_bank;
};


DECLARE_DEVICE_TYPE(SEGAAI_ROM_128, segaai_rom_128_device);
DECLARE_DEVICE_TYPE(SEGAAI_ROM_256, segaai_rom_256_device);

#endif
