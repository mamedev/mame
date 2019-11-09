// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_CARTRIDGE_H
#define MAME_BUS_MSX_SLOT_CARTRIDGE_H

#pragma once

#include "slot.h"
#include "bus/msx_cart/cartridge.h"
#include "softlist_dev.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_CARTRIDGE,        msx_slot_cartridge_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_YAMAHA_EXPANSION, msx_slot_yamaha_expansion_device)


class msx_slot_cartridge_device : public device_t
								, public device_image_interface
								, public device_slot_interface
								, public msx_internal_slot_interface
{
public:
	// construction/destruction
	msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual iodevice_t image_type() const noexcept override { return IO_CARTSLOT; }
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "msx_cart"; }
	virtual const char *file_extensions() const noexcept override { return "mx1,bin,rom"; }
	virtual const char *custom_instance_name() const noexcept override { return "cartridge"; }
	virtual const char *custom_brief_instance_name() const noexcept override { return "cart"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// msx_internal_slot-level overrides
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	DECLARE_WRITE_LINE_MEMBER(irq_out);

protected:
	msx_slot_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	devcb_write_line m_irq_handler;
	msx_cart_interface *m_cartridge;

	static int get_cart_type(const uint8_t *rom, uint32_t length);
};


class msx_slot_yamaha_expansion_device : public msx_slot_cartridge_device
{
public:
	// construction/destruction
	msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const char *image_interface() const noexcept override { return "msx_yamaha_60pin"; }
	virtual const char *custom_instance_name() const noexcept override { return "cartridge60pin"; }
	virtual const char *custom_brief_instance_name() const noexcept override { return "cart60p"; }

protected:
	virtual void device_start() override;
};


#endif // MAME_BUS_MSX_SLOT_CARTRIDGE_H
