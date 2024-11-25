// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_SUNSOFT_DCS_H
#define MAME_BUS_NES_SUNSOFT_DCS_H

#pragma once

#include "sunsoft.h"
#include "imagedev/cartrom.h"


//-----------------------------------------------
//
//  Nantettate!! Baseball Cartslot implementation
//
//-----------------------------------------------

// ======================> ntb_cart_interface

class ntb_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~ntb_cart_interface();

	// reading and writing
	virtual uint8_t read(offs_t offset) { return m_rom[offset]; }

	uint8_t *get_cart_base() { return m_rom; }

protected:
	ntb_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
};

// ======================> nes_ntb_slot_device

class nes_sunsoft_dcs_device;

class nes_ntb_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<ntb_cart_interface>
{
	friend class nes_sunsoft_dcs_device;
public:
	// construction/destruction
	template <typename T>
	nes_ntb_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts)
		: nes_ntb_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	nes_ntb_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~nes_ntb_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "ntb_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	virtual uint8_t read(offs_t offset);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	ntb_cart_interface *m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_NTB_SLOT, nes_ntb_slot_device)


//-----------------------------------------------
//
//  Nantettate!! Baseball Minicart implementation
//
//-----------------------------------------------

// ======================> nes_ntb_rom_device

class nes_ntb_rom_device : public device_t,
							public ntb_cart_interface
{
public:
	// construction/destruction
	nes_ntb_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t *get_cart_base();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_NTB_ROM, nes_ntb_rom_device)



//------------------------------------------------
//
//  Nantettate!! Baseball base cart implementation
//  a.k.a. Sunsoft Dual Cassette System
//  (variant of Sunsoft-4 PCB)
//
//------------------------------------------------

// ======================> nes_sunsoft_dcs_device

class nes_sunsoft_dcs_device : public nes_sunsoft_4_device
{
public:
	// construction/destruction
	nes_sunsoft_dcs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(protect_tick);

private:
	int m_timer_on, m_exrom_enable;
	required_device<nes_ntb_slot_device> m_subslot;

	emu_timer *ntb_enable_timer;
	attotime timer_freq;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SUNSOFT_DCS, nes_sunsoft_dcs_device)

#endif // MAME_BUS_NES_SUNSOFT_DCS_H
