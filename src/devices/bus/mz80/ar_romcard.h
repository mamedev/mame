// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_MZ80_AR_ROMCARD_H
#define MAME_BUS_MZ80_AR_ROMCARD_H

#pragma once

#include "mz80_exp.h"
#include "imagedev/cartrom.h"

// ======================> ar_romcard_device

class ar_romcard_device : public device_t, public device_mz80_exp_interface, public device_rom_image_interface
{
public:
	// construction/destruction
	ar_romcard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_image_interface implementation
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }
	std::pair<std::error_condition, std::string> call_load() override;

	// device_mz80_exp_interface implementation
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	u8 reset_address_r();
	void reset_address_w(u8 data);
	u8 rom_data_r();
	void increment_address_w(u8 data);

	std::unique_ptr<u8 []> m_rom;
	u16 m_rom_address;
};

// device type declaration
DECLARE_DEVICE_TYPE(AR_ROMCARD, ar_romcard_device)

#endif // MAME_BUS_MZ80_AR_ROMCARD_H
