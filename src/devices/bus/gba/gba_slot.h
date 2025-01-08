// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz,Fabio Priuli
#ifndef MAME_BUS_GBA_GBA_SLOT_H
#define MAME_BUS_GBA_GBA_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	GBA_STD = 0,
	GBA_SRAM,
	GBA_DRILLDOZ,
	GBA_WARIOTWS,
	GBA_EEPROM,
	GBA_EEPROM4,
	GBA_YOSHIUG,
	GBA_EEPROM64,
	GBA_BOKTAI,
	GBA_FLASH,
	GBA_FLASH_RTC,
	GBA_FLASH512,
	GBA_FLASH1M,
	GBA_FLASH1M_RTC,
	GBA_3DMATRIX
};


// ======================> device_gba_cart_interface

class device_gba_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_gba_cart_interface();

	// reading and writing
	virtual uint32_t read_rom(offs_t offset) { return 0xffffffff; }
	virtual uint32_t read_ram(offs_t offset, uint32_t mem_mask = ~0) { return 0xffffffff; }
	virtual uint32_t read_gpio(offs_t offset, uint32_t mem_mask = ~0) { return 0; }
	virtual uint32_t read_tilt(offs_t offset, uint32_t mem_mask = ~0) { return 0xffffffff; }
	virtual void write_ram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { }
	virtual void write_gpio(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { }
	virtual void write_tilt(offs_t offset, uint32_t data) { }
	virtual void write_mapper(offs_t offset, uint32_t data) { }

	void rom_alloc(uint32_t size, const char *tag);
	void nvram_alloc(uint32_t size);
	uint32_t* get_rom_base() { return m_rom; }
	uint32_t* get_romhlp_base() { return m_romhlp; }
	uint32_t* get_nvram_base() { return &m_nvram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_romhlp_size() { return m_romhlp_size; }
	uint32_t get_nvram_size() { return m_nvram.size()*sizeof(uint32_t); }
	void set_rom_size(uint32_t val) { m_rom_size = val; }

	void save_nvram()   { device().save_item(NAME(m_nvram)); }

protected:
	device_gba_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint32_t *m_rom;  // this points to the cart rom region
	uint32_t m_rom_size;  // this is the actual game size, not the rom region size!
	uint32_t *m_romhlp;
	uint32_t m_romhlp_size;
	std::vector<uint32_t> m_nvram;
};


// ======================> gba_cart_slot_device

class gba_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	gba_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: gba_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	gba_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~gba_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "gba_cart"; }
	virtual const char *file_extensions() const noexcept override { return "gba,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	static int get_cart_type(const uint8_t *ROM, uint32_t len);

	void save_nvram() { if (m_cart && m_cart->get_nvram_size()) m_cart->save_nvram(); }
	uint32_t get_rom_size() { if (m_cart) return m_cart->get_rom_size(); return 0; }

	// reading and writing
	virtual uint32_t read_rom(offs_t offset);
	virtual uint32_t read_ram(offs_t offset, uint32_t mem_mask = ~0);
	virtual uint32_t read_gpio(offs_t offset, uint32_t mem_mask = ~0);
	virtual uint32_t read_tilt(offs_t offset, uint32_t mem_mask = ~0) { if (m_cart) return m_cart->read_tilt(offset); else return 0xffffffff; }
	virtual void write_ram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	virtual void write_gpio(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	virtual void write_tilt(offs_t offset, uint32_t data) { if (m_cart) m_cart->write_tilt(offset, data); }
	virtual void write_mapper(offs_t offset, uint32_t data) { if (m_cart) m_cart->write_mapper(offset, data); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_gba_cart_interface* m_cart;
};



// device type definition
DECLARE_DEVICE_TYPE(GBA_CART_SLOT, gba_cart_slot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define GBASLOT_ROM_REGION_TAG ":cart:rom"
#define GBAHELP_ROM_REGION_TAG ":cart:romhlp"


//------------------------------------------------------------------------
//
// Misc structs to attempt NVRAM identification when loading from fullpath
//
//------------------------------------------------------------------------


#define GBA_CHIP_EEPROM     (1 << 0)
#define GBA_CHIP_SRAM       (1 << 1)
#define GBA_CHIP_FLASH      (1 << 2)
#define GBA_CHIP_FLASH_1M   (1 << 3)
#define GBA_CHIP_RTC        (1 << 4)
#define GBA_CHIP_FLASH_512  (1 << 5)
#define GBA_CHIP_EEPROM_64K (1 << 6)
#define GBA_CHIP_EEPROM_4K  (1 << 7)


#endif // MAME_BUS_GBA_GBA_SLOT_H
