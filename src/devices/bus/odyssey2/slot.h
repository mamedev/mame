// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_ODYSSEY2_SLOT_H
#define MAME_BUS_ODYSSEY2_SLOT_H

#pragma once

#include "softlist_dev.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	O2_STD = 0,
	O2_ROM12,
	O2_ROM16,
	O2_CHESS,
	O2_VOICE
};


// ======================> device_o2_cart_interface

class device_o2_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_o2_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom04) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_rom0c) { return 0xff; }
	virtual void write_bank(int bank) { }

	virtual DECLARE_WRITE8_MEMBER(io_write) { }
	virtual DECLARE_READ_LINE_MEMBER(t0_read) { return 0; }

	void rom_alloc(uint32_t size, const char *tag);
	void ram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }

protected:
	device_o2_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
};


// ======================> o2_cart_slot_device

class o2_cart_slot_device : public device_t,
								public device_image_interface,
								public device_single_card_slot_interface<device_o2_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	o2_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: o2_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	o2_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~o2_cart_slot_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override { }

	virtual iodevice_t image_type() const noexcept override { return IO_CARTSLOT; }
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "odyssey_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }

	// reading and writing
	DECLARE_READ8_MEMBER(read_rom04);
	DECLARE_READ8_MEMBER(read_rom0c);
	DECLARE_WRITE8_MEMBER(io_write);
	DECLARE_READ_LINE_MEMBER(t0_read) { if (m_cart) return m_cart->t0_read(); else return 0; }

	void write_bank(int bank)   { if (m_cart) m_cart->write_bank(bank); }

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	int m_type;
	device_o2_cart_interface* m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(O2_CART_SLOT, o2_cart_slot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define O2SLOT_ROM_REGION_TAG ":cart:rom"

void o2_cart(device_slot_interface &device);

#endif // MAME_BUS_ODYSSEY2_SLOT_H
