// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************
 Bandai WonderSwan cartridge slot

 48-pin single-sided PCB edge connector

 The SoC maps the following ranges to the cartridge slot:
 * Memory  0x10000-0x1ffff  8-bit read/write (typically used for cartridge SRAM)
                            3-cycle access on WonderSwan, 1-cycle access on WonderSwan Color
 * Memory  0x20000-0xfffff  16-bit read-only (writes not emitted)
 * I/O     0xc0-0xff

 Pin  Name  Notes
   1  GND
   2  A15
   3  A10
   4  A11
   5  A9
   6  A8
   7  A13
   8  A14
   9  A12
  10  A7
  11  A6
  12  A5
  13  A4
  14  D15
  15  D14
  16  D7
  17  D6
  18  D5
  19  D4
  20  D3
  21  D2
  22  D1
  23  D0
  24  +3.3V
  25  +3.3V
  26  A0
  27  A1
  28  A2
  29  A3
  30  A19
  31  A18
  32  A17
  33  A16
  34  D8
  35  D9
  36  D10
  37  D11
  38  D12
  39  D13
  40  /RESET
  41  ?       input used while authenticating cartridge
  42  IO/M    low when accessing I/O ports
  43  /RD
  44  /WR
  45  /CART   low when accessing cartridge
  46  INT     usually used for RTC alarm interrupt
  47  CLK     384 kHz on WonderSwan
  48  GND
 ***************************************************************************/
#ifndef MAME_BUS_WSWAN_SLOT_H
#define MAME_BUS_WSWAN_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	WS_STD = 0,
	WS_SRAM,
	WS_EEPROM,
	WWITCH
};


// ======================> device_ws_cart_interface

class device_ws_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_ws_cart_interface();

	// reading and writing
	virtual u16 read_rom20(offs_t offset, u16 mem_mask) { return 0xffff; }
	virtual u16 read_rom30(offs_t offset, u16 mem_mask) { return 0xffff; }
	virtual u16 read_rom40(offs_t offset, u16 mem_mask) { return 0xffff; }
	virtual u16 read_ram(offs_t offset, u16 mem_mask) { return 0xffff; }
	virtual void write_ram(offs_t offset, u16 data, u16 mem_mask) {}
	virtual u16 read_io(offs_t offset, u16 mem_mask) { return 0xffff; }
	virtual void write_io(offs_t offset, u16 data, u16 mem_mask) { }

	void rom_alloc(u32 size);
	void nvram_alloc(u32 size);
	u16* get_rom_base() { return m_rom; }
	uint8_t* get_nvram_base() { return &m_nvram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_nvram_size() { return m_nvram.size(); }

	void save_nvram() { device().save_item(NAME(m_nvram)); }
	void set_has_rtc(bool val) { m_has_rtc = val; }

protected:
	device_ws_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	u16 *m_rom;
	u32 m_rom_size;
	std::vector<u8> m_nvram;
	u32 m_nvram_size;
	int m_bank_mask;

	bool m_has_rtc;
};


// ======================> ws_cart_slot_device

class ws_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_ws_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	ws_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&opts, const char *dflt)
		: ws_cart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	ws_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~ws_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "wswan_cart"; }
	virtual const char *file_extensions() const noexcept override { return "ws,wsc,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }
	int get_cart_type(const u16 *ROM, u32 len, u32 &nvram_len) const;
	void internal_header_logging(const u16 *ROM, u32 offs, u32 len);

	void save_nvram()   { if (m_cart && m_cart->get_nvram_size()) m_cart->save_nvram(); }

	// reading and writing
	virtual u16 read_rom20(offs_t offset, u16 mem_mask);
	virtual u16 read_rom30(offs_t offset, u16 mem_mask);
	virtual u16 read_rom40(offs_t offset, u16 mem_mask);
	virtual u16 read_ram(offs_t offset, u16 mem_mask);
	virtual void write_ram(offs_t offset, u16 data, u16 mem_mask);
	virtual u16 read_io(offs_t offset, u16 mem_mask);
	virtual void write_io(offs_t offset, u16 data, u16 mem_mask);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_ws_cart_interface *m_cart;
};



// device type definition
DECLARE_DEVICE_TYPE(WS_CART_SLOT, ws_cart_slot_device)

#endif // MAME_BUS_WSWAN_SLOT_H
