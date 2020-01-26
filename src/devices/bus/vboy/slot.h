// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************
 Virtual Boy cartridge slot

 60-pin two-row connector (2.0 mm pin pitch, 1.6 mm row pitch)

     GND   1   2  GND
    /WE0   3   4  /ES
    /WE1   5   6  /CS
  /RESET   7   8  VCC
 /INTCRO   9  10  A23
     A19  11  12  A22
     A18  13  14  A21
      A8  15  16  A20
      A7  17  18  A9
      A6  19  20  A10
      A5  21  22  A11
      A4  23  24  A12
      A3  25  26  A13
      A2  27  28  A14
      A1  29  30  A15
     /RS  31  32  A16
     GND  33  34  A17
     /OE  35  36  VCC
      D0  37  38  D15
      D8  39  40  D7
      D1  41  42  D14
      D9  43  44  D6
      D2  45  46  D13
     D10  47  48  D5
      D3  49  50  D12
     D11  51  52  D4
     VCC  53  54  VCC
     ARO  55  56  ALO
     ARI  57  58  ALI
     GND  59  60  GND

 The cartridge effectively has three 16 bit 8 megaword (16 megabyte) read/
 write memory spaces.  These are conventionally called EXP, CHIP and ROM.
 Only 16 bit word accesses are possible (no lane select signals, and no
 address bit A0).  No commercial cartridges use the EXP space.  The CHIP
 space is used for cartridge SRAM (if present) and the ROM space is used
 for program ROM.

 Space  Select  Read Enable  Write Enable  Mapping on Virtual Boy
 EXP    /ES     /OE          /WE1          0x0400'0000-0x04ff'ffff
 CHIP   /CS     /OE          /WE0          0x0600'0000-0x06ff'ffff
 ROM    /RS     /OE          /WE1          0x0700'0000-0x07ff'ffff

 Audio is passed through the cartridge, allowing cartridge hardware to
 modify it.  The output of the Virtual Boy's VSU sound chip is DC-coupled
 to ARO (right) and ALO (left) via 4.7 kΩ resistors.  ARI (right) and ALI
 (left) are AC-coupled to the Virtual Boy's audio amplifier.

 VCC is +5V
 /INTCRO is an interrupt input from the cartridge to the host
 /RESET is bidirectional - the system or cartridge can pull it low
 ***************************************************************************/
#ifndef MAME_BUS_VBOY_SLOT_H
#define MAME_BUS_VBOY_SLOT_H

#pragma once

#include "softlist_dev.h"


//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

class device_vboy_cart_interface;



//**************************************************************************
//  CLASS DECLARATIONS
//**************************************************************************

class vboy_cart_slot_device :
		public device_t,
		public device_image_interface,
		public device_single_card_slot_interface<device_vboy_cart_interface>
{
public:
	template <typename T>
	vboy_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt) :
		vboy_cart_slot_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	vboy_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0U);

	// configuration
	auto intcro() { return m_intcro.bind(); }
	template <typename T> void set_exp(T &&tag, int no, offs_t base) { m_exp_space.set_tag(std::forward<T>(tag), no); m_exp_base = base; }
	template <typename T> void set_chip(T &&tag, int no, offs_t base) { m_chip_space.set_tag(std::forward<T>(tag), no); m_chip_base = base; }
	template <typename T> void set_rom(T &&tag, int no, offs_t base) { m_rom_space.set_tag(std::forward<T>(tag), no); m_rom_base = base; }

	// device_image_interface implementation
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual iodevice_t image_type() const noexcept override { return IO_CARTSLOT; }
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual char const *image_interface() const noexcept override { return "vboy_cart"; }
	virtual char const *file_extensions() const noexcept override { return "vb,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

private:
	devcb_write_line m_intcro;
	optional_address_space m_exp_space;
	optional_address_space m_chip_space;
	optional_address_space m_rom_space;
	offs_t m_exp_base;
	offs_t m_chip_base;
	offs_t m_rom_base;

	device_vboy_cart_interface *m_cart;

	friend class device_vboy_cart_interface;
};


class device_vboy_cart_interface : public device_interface
{
public:
	virtual image_init_result load() ATTR_COLD = 0;
	virtual void unload() ATTR_COLD;

protected:
	device_vboy_cart_interface(machine_config const &mconfig, device_t &device);

	bool has_slot() const { return nullptr != m_slot; }
	address_space *exp_space() { return m_slot ? m_slot->m_exp_space.target() : nullptr; }
	address_space *chip_space() { return m_slot ? m_slot->m_chip_space.target() : nullptr; }
	address_space *rom_space() { return m_slot ? m_slot->m_rom_space.target() : nullptr; }
	offs_t exp_base() { return m_slot ? m_slot->m_exp_base : 0U; }
	offs_t chip_base() { return m_slot ? m_slot->m_chip_base : 0U; }
	offs_t rom_base() { return m_slot ? m_slot->m_rom_base : 0U; }

	void battery_load(void *buffer, int length, int fill) { assert(m_slot); m_slot->battery_load(buffer, length, fill); }
	void battery_load(void *buffer, int length, void *def_buffer) { assert(m_slot); m_slot->battery_load(buffer, length, def_buffer); }
	void battery_save(void const *buffer, int length) { assert(m_slot); m_slot->battery_save(buffer, length); }

	void intcro(int state) { if (m_slot) m_slot->m_intcro(state); }

private:
	vboy_cart_slot_device *const m_slot;
};



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

void vboy_carts(device_slot_interface &device);



//**************************************************************************
//  DEVICE TYPE DECLARATIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(VBOY_CART_SLOT, vboy_cart_slot_device)

#endif // MAME_BUS_VBOY_SLOT_H
