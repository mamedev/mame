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


#define MCFG_MSX_SLOT_CARTRIDGE_ADD(_tag, _devcb) \
	MCFG_DEVICE_ADD(_tag, MSX_SLOT_CARTRIDGE, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(msx_cart, nullptr, false) \
	downcast<msx_slot_cartridge_device &>(*device).set_irq_handler(DEVCB_##_devcb);


#define MCFG_MSX_SLOT_YAMAHA_EXPANSION_ADD(_tag, _devcb, _default) \
	MCFG_DEVICE_ADD(_tag, MSX_SLOT_YAMAHA_EXPANSION, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(msx_yamaha_60pin, _default, false) \
	downcast<msx_slot_cartridge_device &>(*device).set_irq_handler(DEVCB_##_devcb);


class msx_slot_cartridge_device : public device_t
								, public device_image_interface
								, public device_slot_interface
								, public msx_internal_slot_interface
{
public:
	// construction/destruction
	msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template <class Object> devcb_base &set_irq_handler(Object &&cb) { return m_irq_handler.set_callback(std::forward<Object>(cb)); }

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }
	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return false; }
	virtual bool is_creatable() const override { return false; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return true; }
	virtual const char *image_interface() const override { return "msx_cart"; }
	virtual const char *file_extensions() const override { return "mx1,bin,rom"; }
	virtual const char *custom_instance_name() const override { return "cartridge"; }
	virtual const char *custom_brief_instance_name() const override { return "cart"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// msx_internal_slot-level overrides
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_WRITE_LINE_MEMBER(irq_out);

protected:
	msx_slot_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	devcb_write_line m_irq_handler;
	msx_cart_interface *m_cartridge;

	static int get_cart_type(const uint8_t *rom, uint32_t length);
};


class msx_slot_yamaha_expansion_device : public msx_slot_cartridge_device
{
public:
	// construction/destruction
	msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const char *image_interface() const override { return "msx_yamaha_60pin"; }
	virtual const char *custom_instance_name() const override { return "cartridge60pin"; }
	virtual const char *custom_brief_instance_name() const override { return "cart60p"; }

protected:
	virtual void device_start() override;
};


#endif // MAME_BUS_MSX_SLOT_CARTRIDGE_H
