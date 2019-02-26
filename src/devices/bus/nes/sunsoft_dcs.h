// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_SUNSOFT_DCS_H
#define MAME_BUS_NES_SUNSOFT_DCS_H

#pragma once

#include "sunsoft.h"
#include "softlist_dev.h"


//-----------------------------------------------
//
//  Nantettate!! Baseball Cartslot implementation
//
//-----------------------------------------------

// ======================> ntb_cart_interface

class ntb_cart_interface : public device_slot_card_interface
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
								public device_image_interface,
								public device_slot_interface
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

	// device-level overrides
	virtual void device_start() override;

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "ntb_cart"; }
	virtual const char *file_extensions() const override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	virtual uint8_t read(offs_t offset);

protected:
	ntb_cart_interface*      m_cart;
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

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual uint8_t* get_cart_base();

protected:
	// device-level overrides
	virtual void device_start() override;
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
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void device_add_mconfig(machine_config &config) override;

private:
	static constexpr device_timer_id TIMER_PROTECT = 0;

	int m_timer_on, m_exrom_enable;
	required_device<nes_ntb_slot_device> m_subslot;

	emu_timer *ntb_enable_timer;
	attotime timer_freq;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SUNSOFT_DCS, nes_sunsoft_dcs_device)

#endif // MAME_BUS_NES_SUNSOFT_DCS_H
