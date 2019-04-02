// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_ASTROCADE_SLOT_H
#define MAME_BUS_ASTROCADE_SLOT_H

#pragma once

#include "softlist_dev.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

#define ASTROCADESLOT_ROM_REGION_TAG ":cart:rom"

/* PCB */
enum
{
	ASTROCADE_STD = 0,
	ASTROCADE_256K,
	ASTROCADE_512K,
	ASTROCADE_CASS
};


// ======================> device_astrocade_cart_interface

class device_astrocade_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_astrocade_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) { return 0xff; }

	void rom_alloc(uint32_t size, const char *tag);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

protected:
	device_astrocade_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
};


// ======================> astrocade_cart_slot_device

class astrocade_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	astrocade_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: astrocade_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	astrocade_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~astrocade_cart_slot_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override {}
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	int get_type() { return m_type; }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "astrocde_cart"; }
	virtual const char *file_extensions() const override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);

protected:
	// device-level overrides
	virtual void device_start() override;

	int m_type;
	device_astrocade_cart_interface*       m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(ASTROCADE_CART_SLOT, astrocade_cart_slot_device)

#endif // MAME_BUS_ASTROCADE_SLOT_H
