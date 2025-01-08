// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_KARASTUDIO_H
#define MAME_BUS_NES_KARASTUDIO_H

#pragma once

#include "nxrom.h"
#include "imagedev/cartrom.h"


//-----------------------------------------
//
//  Karaoke Studio Cartslot implementation
//
//-----------------------------------------

// ======================> kstudio_cart_interface

class kstudio_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~kstudio_cart_interface();

	// reading and writing
	virtual uint8_t read(offs_t offset);

	uint8_t *get_cart_base() { return m_rom; }
	void write_prg_bank(uint8_t bank) { m_bank = bank; }

protected:
	kstudio_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	// ROM is accessed via two 16K banks, but only the first one can be switched
	uint8_t m_bank;
};

// ======================> nes_kstudio_slot_device

class nes_karaokestudio_device;

class nes_kstudio_slot_device : public device_t,
									public device_cartrom_image_interface,
									public device_single_card_slot_interface<kstudio_cart_interface>
{
	friend class nes_karaokestudio_device;
public:
	// construction/destruction
	template <typename T>
	nes_kstudio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts)
		: nes_kstudio_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	nes_kstudio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~nes_kstudio_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "ks_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	uint8_t read(offs_t offset);
	void write_prg_bank(uint8_t bank) { if (m_cart) m_cart->write_prg_bank(bank); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	kstudio_cart_interface *m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_KSEXPANSION_SLOT, nes_kstudio_slot_device)


//-----------------------------------------------
//
//  Karaoke Studio Expansion cart implementation
//
//-----------------------------------------------

// ======================> nes_kstudio_rom_device

class nes_kstudio_rom_device : public device_t,
									public kstudio_cart_interface
{
public:
	// construction/destruction
	nes_kstudio_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t *get_cart_base();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_KSEXPANSION_ROM, nes_kstudio_rom_device)


//-------------------------------------------
//
//  Karaoke Studio Base Cart implementation
//
//-------------------------------------------

// ======================> nes_karaokestudio_device

class nes_karaokestudio_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_karaokestudio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	int m_exp_active;
	required_device<nes_kstudio_slot_device> m_subslot;
	required_ioport m_mic_ipt;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_KARAOKESTUDIO, nes_karaokestudio_device)

#endif // MAME_BUS_NES_KARASTUDIO_H
