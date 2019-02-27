// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    kc.h

    KC85_2/3/4/5 expansion slot emulation

*********************************************************************/

#ifndef MAME_BUS_KC_KC_H
#define MAME_BUS_KC_KC_H

#pragma once

#include "softlist_dev.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> device_kcexp_interface

class device_kcexp_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_kcexp_interface();

	// reading and writing
	virtual uint8_t module_id_r() { return 0xff; }
	virtual void control_w(uint8_t data) { }
	virtual void read(offs_t offset, uint8_t &data) { }
	virtual void write(offs_t offset, uint8_t data) { }
	virtual void io_read(offs_t offset, uint8_t &data) { }
	virtual void io_write(offs_t offset, uint8_t data) { }
	virtual uint8_t* get_cart_base() { return nullptr; }
	virtual DECLARE_WRITE_LINE_MEMBER( mei_w ) { }

protected:
	device_kcexp_interface(const machine_config &mconfig, device_t &device);
};

// ======================> kcexp_slot_device

class kcexp_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	kcexp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: kcexp_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	kcexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~kcexp_slot_device();

	auto irq() { return m_out_irq_cb.bind(); }
	auto nmi() { return m_out_nmi_cb.bind(); }
	auto halt() { return m_out_halt_cb.bind(); }

	// inline configuration
	void set_next_slot(const char *next_module_tag) { m_next_slot_tag = next_module_tag; }

	// reading and writing
	virtual uint8_t module_id_r();
	virtual void control_w(uint8_t data);
	virtual void read(offs_t offset, uint8_t &data);
	virtual void write(offs_t offset, uint8_t data);
	virtual void io_read(offs_t offset, uint8_t &data);
	virtual void io_write(offs_t offset, uint8_t data);
	virtual DECLARE_WRITE_LINE_MEMBER( mei_w );
	virtual DECLARE_WRITE_LINE_MEMBER( meo_w );

	devcb_write_line                m_out_irq_cb;
	devcb_write_line                m_out_nmi_cb;
	devcb_write_line                m_out_halt_cb;

protected:
	kcexp_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	device_kcexp_interface*     m_cart;

	const char*                 m_next_slot_tag;
	kcexp_slot_device*          m_next_slot;
};

// ======================> kccart_slot_device

class kccart_slot_device : public kcexp_slot_device,
							public device_image_interface
{
public:
	// construction/destruction
	template <typename T>
	kccart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: kccart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	kccart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~kccart_slot_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "kc_cart"; }
	virtual const char *file_extensions() const override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;
};

// device type definition
DECLARE_DEVICE_TYPE(KCEXP_SLOT,  kcexp_slot_device)
DECLARE_DEVICE_TYPE(KCCART_SLOT, kccart_slot_device)

#endif // MAME_BUS_KC_KC_H
