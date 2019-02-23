// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_VBOY_SLOT_H
#define MAME_BUS_VBOY_SLOT_H

#pragma once

#include "softlist_dev.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	VBOY_STD = 0,
	VBOY_EEPROM
};


// ======================> device_vboy_cart_interface

class device_vboy_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_vboy_cart_interface();

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_cart) { return 0xffffffff; }
	virtual DECLARE_READ32_MEMBER(read_eeprom) { return 0xffffffff; }
	virtual DECLARE_WRITE32_MEMBER(write_eeprom) { }

	void rom_alloc(uint32_t size, const char *tag);
	void eeprom_alloc(uint32_t size);
	uint32_t* get_rom_base() { return m_rom; }
	uint32_t* get_eeprom_base() { return &m_eeprom[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_eeprom_size() { return m_eeprom.size(); }

	void save_eeprom() { device().save_item(NAME(m_eeprom)); }

protected:
	device_vboy_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint32_t *m_rom;
	uint32_t m_rom_size;
	uint32_t m_rom_mask;
	std::vector<uint32_t> m_eeprom;
};


// ======================> vboy_cart_slot_device

class vboy_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	vboy_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: vboy_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	vboy_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~vboy_cart_slot_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	int get_type() { return m_type; }

	void save_eeprom()  { if (m_cart && m_cart->get_eeprom_size()) m_cart->save_eeprom(); }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 1; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "vboy_cart"; }
	virtual const char *file_extensions() const override { return "vb,bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_cart);
	virtual DECLARE_READ32_MEMBER(read_eeprom);
	virtual DECLARE_WRITE32_MEMBER(write_eeprom);

protected:
	// device-level overrides
	virtual void device_start() override;

	int m_type;
	device_vboy_cart_interface* m_cart;
};


// device type definition
DECLARE_DEVICE_TYPE(VBOY_CART_SLOT, vboy_cart_slot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define VBOYSLOT_ROM_REGION_TAG ":cart:rom"

#endif // MAME_BUS_VBOY_SLOT_H
