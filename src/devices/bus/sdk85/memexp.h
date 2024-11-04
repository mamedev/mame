// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_SDK85_MEMEXP_H
#define MAME_BUS_SDK85_MEMEXP_H

#pragma once

#include "imagedev/cartrom.h"

// ======================> device_sdk85_romexp_card_interface

class device_sdk85_romexp_card_interface : public device_interface
{
	friend class sdk85_romexp_device;

protected:
	device_sdk85_romexp_card_interface(const machine_config &mconfig, device_t &device);

	virtual u8 read_memory(offs_t offset) = 0;
	virtual void write_memory(offs_t offset, u8 data) = 0;
	virtual u8 read_io(offs_t offset) = 0;
	virtual void write_io(offs_t offset, u8 data) = 0;

	virtual u8 *get_rom_base(u32 size);
};

// ======================> sdk85_romexp_device

class sdk85_romexp_device : public device_t,
								public device_rom_image_interface,
								public device_single_card_slot_interface<device_sdk85_romexp_card_interface>
{
public:
	// construction/destruction
	sdk85_romexp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T>
	sdk85_romexp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&opts, const char *dflt)
		: sdk85_romexp_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	static void rom_options(device_slot_interface &device);

	u8 memory_r(offs_t offset);
	void memory_w(offs_t offset, u8 data);
	u8 io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override { }

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "sdk85_rom"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

private:
	device_sdk85_romexp_card_interface *m_dev;
};

// device type declaration
DECLARE_DEVICE_TYPE(SDK85_ROMEXP, sdk85_romexp_device)

#endif // MAME_BUS_SDK85_MEMEXP_H
