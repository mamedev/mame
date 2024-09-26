// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SCV_SLOT_H
#define MAME_BUS_SCV_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	SCV_8K = 0,
	SCV_16K,
	SCV_32K,
	SCV_32K_RAM,
	SCV_64K,
	SCV_128K,
	SCV_128K_RAM
};


// ======================> device_scv_cart_interface

class device_scv_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_scv_cart_interface();

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) { return 0xff; }
	virtual void write_cart(offs_t offset, uint8_t data) { }
	virtual void write_bank(uint8_t data) { }

	void rom_alloc(uint32_t size);
	void ram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }

	void save_ram() { device().save_item(NAME(m_ram)); }

protected:
	device_scv_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
};


// ======================> scv_cart_slot_device

class scv_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_scv_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	scv_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: scv_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	scv_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~scv_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override { }

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "scv_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	static int get_cart_type(const uint8_t *ROM, uint32_t len);

	void save_ram() { if (m_cart && m_cart->get_ram_size()) m_cart->save_ram(); }

	// reading and writing
	uint8_t read_cart(offs_t offset);
	void write_cart(offs_t offset, uint8_t data);
	void write_bank(uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_scv_cart_interface *m_cart;
};



// device type definition
DECLARE_DEVICE_TYPE(SCV_CART_SLOT, scv_cart_slot_device)

#endif // MAME_BUS_SCV_SLOT_H
