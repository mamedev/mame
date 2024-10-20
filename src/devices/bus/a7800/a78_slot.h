// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_A7800_A78_SLOT_H
#define MAME_BUS_A7800_A78_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

/* PCB */
enum
{
	A78_TYPE0 = 0,      // standard 8K/16K/32K games, no bankswitch
	A78_TYPE1,          // as TYPE0 + POKEY chip on the PCB
	A78_TYPE2,          // Atari SuperGame pcb (8x16K banks with bankswitch)
	A78_TYPE3,          // as TYPE1 + POKEY chip on the PCB
	A78_TYPE6,          // as TYPE1 + RAM IC on the PCB
	A78_TYPEA,          // Alien Brigade, Crossbow (9x16K banks with diff bankswitch)
	A78_TYPE8,          // Rescue on Fractalus, as TYPE0 + 2K Mirror RAM IC on the PCB
	A78_ABSOLUTE,       // F18 Hornet
	A78_ACTIVISION,     // Double Dragon, Rampage
	A78_HSC,            // Atari HighScore cart
	A78_XB_BOARD,       // A7800 Expansion Board (it shall more or less apply to the Expansion Module too, but this is not officially released yet)
	A78_XM_BOARD,       // A7800 XM Expansion Module (theoretical specs only, since this is not officially released yet)
	A78_MEGACART,               // Homebrew by CPUWIZ, consists of SuperGame bank up to 512K + 32K RAM banked
	A78_VERSABOARD = 0x10,      // Homebrew by CPUWIZ, consists of SuperGame bank up to 256K + 32K RAM banked
	// VersaBoard variants configured as Type 1/3/A or VersaBoard + POKEY at $0450
	A78_TYPE0_POK450 = 0x20,
	A78_TYPE1_POK450 = 0x21,
	A78_TYPE6_POK450 = 0x24,
	A78_TYPEA_POK450 = 0x25,
	A78_VERSA_POK450 = 0x30
};


// ======================> device_a78_cart_interface

class device_a78_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_a78_cart_interface();

	// memory accessor
	virtual uint8_t read_04xx(offs_t offset) { return 0xff; }
	virtual uint8_t read_10xx(offs_t offset) { return 0xff; }
	virtual uint8_t read_30xx(offs_t offset) { return 0xff; }
	virtual uint8_t read_40xx(offs_t offset) { return 0xff; }
	virtual void write_04xx(offs_t offset, uint8_t data) {}
	virtual void write_10xx(offs_t offset, uint8_t data) {}
	virtual void write_30xx(offs_t offset, uint8_t data) {}
	virtual void write_40xx(offs_t offset, uint8_t data) {}

	void rom_alloc(uint32_t size);
	void ram_alloc(uint32_t size);
	void nvram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint8_t* get_nvram_base() { return &m_nvram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }
	uint32_t get_nvram_size() { return m_nvram.size(); }

protected:
	device_a78_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
	std::vector<uint8_t> m_nvram; // HiScore cart can save scores!
	// helpers
	uint32_t m_base_rom;
	int m_bank_mask;
};


// ======================> a78_cart_slot_device

class a78_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	a78_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: a78_cart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	a78_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~a78_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "a7800_cart"; }
	virtual const char *file_extensions() const noexcept override { return "a78"; }
	virtual u32 unhashed_header_length() const noexcept override { return 128; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_cart_type() { return m_type; }
	bool has_cart() { return m_cart != nullptr; }

	// reading and writing
	uint8_t read_04xx(offs_t offset);
	uint8_t read_10xx(offs_t offset);
	uint8_t read_30xx(offs_t offset);
	uint8_t read_40xx(offs_t offset);
	void write_04xx(offs_t offset, uint8_t data);
	void write_10xx(offs_t offset, uint8_t data);
	void write_30xx(offs_t offset, uint8_t data);
	void write_40xx(offs_t offset, uint8_t data);

private:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	std::pair<std::error_condition, std::string> verify_header(const uint8_t *header);
	int validate_header(int head, bool log) const;
	void internal_header_logging(const uint8_t *header, uint32_t len);

	device_a78_cart_interface *m_cart;
	int m_type;
};


// device type definition
DECLARE_DEVICE_TYPE(A78_CART_SLOT, a78_cart_slot_device)

#endif // MAME_BUS_A7800_A78_SLOT_H
