// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_A800_A800_SLOT_H
#define MAME_BUS_A800_A800_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

#define A800SLOT_ROM_REGION_TAG ":cart:rom"

/* PCB */
enum
{
	A800_8K = 0,
	A800_8K_RIGHT,
	A800_16K,
	A800_OSS034M,
	A800_OSS043M,
	A800_OSSM091,
	A800_OSS8K,
	A800_PHOENIX,
	A800_XEGS,
	A800_BBSB,
	A800_DIAMOND,
	A800_WILLIAMS,
	A800_EXPRESS,
	A800_SPARTADOS,
	A800_BLIZZARD,
	A800_TURBO64,
	A800_TURBO128,
	A800_TELELINK2,
	A800_MICROCALC,
	A800_CORINA,
	A5200_4K,
	A5200_8K,
	A5200_16K,
	A5200_32K,
	A5200_16K_2CHIPS,
	A5200_BBSB
};


// ======================> device_a800_cart_interface

class device_a800_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_a800_cart_interface();

	// memory accessor
	virtual uint8_t read_80xx(offs_t offset) { return 0xff; }
	virtual uint8_t read_d5xx(offs_t offset) { return 0xff; }
	virtual void write_80xx(offs_t offset, uint8_t data) {}
	virtual void write_d5xx(offs_t offset, uint8_t data) {}

	void rom_alloc(uint32_t size, const char *tag);
	void ram_alloc(uint32_t size);
	void nvram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint8_t* get_nvram_base() { return &m_nvram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }
	uint32_t get_nvram_size() { return m_nvram.size(); }

protected:
	device_a800_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
	std::vector<uint8_t> m_nvram; // HiScore cart can save scores!
	// helpers
	int m_bank_mask;
};


// ======================> a800_cart_slot_device

class a800_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_a800_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	a800_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: a800_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	a800_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~a800_cart_slot_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "a8bit_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom,car"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_cart_type() { return m_type; }
	int identify_cart_type(const uint8_t *header) const;
	bool has_cart() { return m_cart != nullptr; }

	// reading and writing
	uint8_t read_80xx(offs_t offset);
	uint8_t read_d5xx(offs_t offset);
	void write_80xx(offs_t offset, uint8_t data);
	void write_d5xx(offs_t offset, uint8_t data);

protected:
	a800_cart_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

private:
	device_a800_cart_interface*       m_cart;
	int m_type;
};


// The variants below are added to handle the additional formats for a5200, and to give more
// clear error messages if you try to load an A5200 game into an A800 or a XEGS, etc.

// ======================> a5200_cart_slot_device

class a5200_cart_slot_device : public a800_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	a5200_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: a5200_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	a5200_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~a5200_cart_slot_device();

	virtual const char *file_extensions() const noexcept override { return "bin,rom,car,a52"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;
};

// ======================> xegs_cart_slot_device

class xegs_cart_slot_device : public a800_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	xegs_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: xegs_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	xegs_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~xegs_cart_slot_device();

	virtual const char *file_extensions() const noexcept override { return "bin,rom,car"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;
};

// device type definition
DECLARE_DEVICE_TYPE(A800_CART_SLOT,  a800_cart_slot_device)
DECLARE_DEVICE_TYPE(A5200_CART_SLOT, a5200_cart_slot_device)
DECLARE_DEVICE_TYPE(XEGS_CART_SLOT,  xegs_cart_slot_device)

#endif // MAME_BUS_A800_A800_SLOT_H
