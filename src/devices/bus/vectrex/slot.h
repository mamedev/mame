// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_VECTREX_SLOT_H
#define MAME_BUS_VECTREX_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	VECTREX_STD = 0,
	VECTREX_64K,
	VECTREX_SRAM
};

// 3D setup
enum
{
	VEC3D_NONE = 0,
	VEC3D_MINEST,
	VEC3D_CCOAST,
	VEC3D_NARROW
};

// ======================> device_vectrex_cart_interface

class device_vectrex_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_vectrex_cart_interface();

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) { return 0xff; }
	virtual void write_ram(offs_t offset, uint8_t data) { }
	virtual void write_bank(uint8_t data) { }

	void rom_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

protected:
	device_vectrex_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
};


// ======================> vectrex_cart_slot_device

class vectrex_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_vectrex_cart_interface>
{
public:
	// construction/destruction
	vectrex_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~vectrex_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override { }

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "vectrex_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,gam,vec"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	int get_vec3d() { return m_vec3d; }

	// reading and writing
	uint8_t read_rom(offs_t offset);
	void write_ram(offs_t offset, uint8_t data);
	void write_bank(uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type, m_vec3d;
	device_vectrex_cart_interface *m_cart;
};



// device type definition
DECLARE_DEVICE_TYPE(VECTREX_CART_SLOT, vectrex_cart_slot_device)

#endif // MAME_BUS_VECTREX_SLOT_H
