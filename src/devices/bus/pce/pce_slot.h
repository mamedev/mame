// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_PCE_PCE_SLOT_H
#define MAME_BUS_PCE_PCE_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	PCE_STD = 0,
	PCE_CDSYS3J,
	PCE_CDSYS3U,
	PCE_POPULOUS,
	PCE_SF2,
	PCE_TENNOKOE
};


// ======================> device_pce_cart_interface

class device_pce_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_pce_cart_interface();

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) { return 0xff; }
	virtual void write_cart(offs_t offset, uint8_t data) {}

	void rom_alloc(uint32_t size, const char *tag);
	void ram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }

	void rom_map_setup(uint32_t size);

protected:
	device_pce_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;

	uint8_t rom_bank_map[8];    // 128K chunks of rom
};


// ======================> pce_cart_slot_device

class pce_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_pce_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	pce_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt, const char *interface)
		: pce_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_intf(interface);
	}

	pce_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~pce_cart_slot_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return m_interface; }
	virtual const char *file_extensions() const noexcept override { return "pce,bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	static int get_cart_type(const uint8_t *ROM, uint32_t len);

	void set_intf(const char * interface) { m_interface = interface; }

	// reading and writing
	uint8_t read_cart(offs_t offset);
	void write_cart(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;

	const char *m_interface;
	int m_type;
	device_pce_cart_interface *m_cart;
};



// device type definition
DECLARE_DEVICE_TYPE(PCE_CART_SLOT, pce_cart_slot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define PCESLOT_ROM_REGION_TAG ":cart:rom"

#endif // MAME_BUS_PCE_PCE_SLOT_H
