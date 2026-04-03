// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
#ifndef MAME_ENSONIQ_VFXCART_H
#define MAME_ENSONIQ_VFXCART_H

#pragma once

#include "machine/x28.h"

class ensoniq_vfx_cartridge:
	public device_t,
	public device_image_interface
{
public:
	using load_cb = delegate<void (ensoniq_vfx_cartridge *)>;
	using unload_cb = delegate<void (ensoniq_vfx_cartridge *)>;

	ensoniq_vfx_cartridge(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock = 0);

	virtual ~ensoniq_vfx_cartridge();

	void setup_load_cb(load_cb cb);
	void setup_unload_cb(unload_cb cb);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// device_image_interface overrides
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	virtual bool        is_readable()                   const noexcept override { return true; }
	virtual bool        is_writeable()                  const noexcept override { return true; }
	virtual bool        is_creatable()                  const noexcept override { return true; }
	virtual bool        is_reset_on_load()              const noexcept override { return false; }
	virtual char const *file_extensions()               const noexcept override { return "rom,cart,eeprom,sc32"; }
	virtual char const *image_type_name()               const noexcept override { return "cartridge"; }
	virtual char const *image_brief_type_name()         const noexcept override { return "cart"; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:

	required_device<x28c256_device> m_eeprom;

	static constexpr uint32_t SIZE = x28c256_device::TOTAL_SIZE_BYTES;
	static constexpr uint32_t MASK = SIZE - 1;

	bool m_is_loaded = false;
	bool m_is_writeable = false;

	load_cb m_load_cb;
	unload_cb m_unload_cb;
	required_ioport m_input_config;
};


// device type declaration
DECLARE_DEVICE_TYPE(ENSONIQ_VFX_CARTRIDGE, ensoniq_vfx_cartridge)

#endif // MAME_ENSONIQ_VFXCART_H
