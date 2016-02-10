// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_SLOT_CARTRIDGE_H
#define __MSX_SLOT_CARTRIDGE_H

#include "slot.h"
#include "bus/msx_cart/cartridge.h"


extern const device_type MSX_SLOT_CARTRIDGE;
extern const device_type MSX_SLOT_YAMAHA_EXPANSION;


#define MCFG_MSX_SLOT_CARTRIDGE_ADD(_tag, _devcb) \
	MCFG_DEVICE_ADD(_tag, MSX_SLOT_CARTRIDGE, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(msx_cart, NULL, false) \
	devcb = &msx_slot_cartridge_device::set_irq_handler(*device, DEVCB_##_devcb);


#define MCFG_MSX_SLOT_YAMAHA_EXPANSION_ADD(_tag, _devcb, _default) \
	MCFG_DEVICE_ADD(_tag, MSX_SLOT_YAMAHA_EXPANSION, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(msx_yamaha_60pin, _default, false) \
	devcb = &msx_slot_cartridge_device::set_irq_handler(*device, DEVCB_##_devcb);


class msx_slot_cartridge_device : public device_t
								, public device_image_interface
								, public device_slot_interface
								, public msx_internal_slot_interface
{
public:
	// construction/destruction
	msx_slot_cartridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<msx_slot_cartridge_device &>(device).m_irq_handler.set_callback(object); }

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override { update_names(MSX_SLOT_CARTRIDGE, "cartridge", "cart"); }

	// image-level overrides
	virtual bool call_load() override;
	virtual void call_unload() override;
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) override;
	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return false; }
	virtual bool is_creatable() const override { return false; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return true; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }
	virtual const char *image_interface() const override { return "msx_cart"; }
	virtual const char *file_extensions() const override { return "mx1,bin,rom"; }

	// slot interface overrides
	virtual std::string get_default_card_software() override;

	// msx_internal_slot-level overrides
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_WRITE_LINE_MEMBER(irq_out);

protected:
	devcb_write_line m_irq_handler;
	msx_cart_interface *m_cartridge;

	int get_cart_type(UINT8 *rom, UINT32 length);
};


class msx_slot_yamaha_expansion_device : public msx_slot_cartridge_device
{
public:
	// construction/destruction
	msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start() override;
	virtual void device_config_complete() override { update_names(MSX_SLOT_YAMAHA_EXPANSION, "cartridge60pin", "cart60p"); }

	virtual const char *image_interface() const override { return "msx_yamaha_60pin"; }
};


#endif
