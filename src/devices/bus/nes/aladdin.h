// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_ALADDIN_H
#define MAME_BUS_NES_ALADDIN_H

#pragma once

#include "nxrom.h"
#include "imagedev/cartrom.h"


//----------------------------------
//
//  Aladdin Cartslot implementation
//
//----------------------------------

// ======================> aladdin_cart_interface

class aladdin_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~aladdin_cart_interface();

	// reading and writing
	virtual uint8_t read(offs_t offset);

	uint8_t *get_cart_base() { return m_rom; }
	void set_cart_size(uint32_t size) { m_rom_size = size; m_rom_mask = (size / 0x4000) - 1; }
	virtual void write_prg(uint32_t offset, uint8_t data) { }

protected:
	aladdin_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	uint8_t m_lobank, m_hibank, m_rom_mask;
};

// ======================> nes_aladdin_slot_device

class nes_aladdin_device;

class nes_aladdin_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<aladdin_cart_interface>
{
	friend class nes_aladdin_device;
public:
	// construction/destruction
	template <typename T>
	nes_aladdin_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts)
		: nes_aladdin_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	nes_aladdin_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~nes_aladdin_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "ade_cart"; }
	virtual const char *file_extensions() const noexcept override { return "nes,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	uint8_t read(offs_t offset);
	void write_prg(uint32_t offset, uint8_t data) { if (m_cart) m_cart->write_prg(offset, data); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	aladdin_cart_interface *m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_ALADDIN_SLOT, nes_aladdin_slot_device)


//----------------------------------
//
//  Aladdin Minicart implementation
//
//----------------------------------

// ======================> nes_algn_rom_device

class nes_algn_rom_device : public device_t,
							public aladdin_cart_interface
{
public:
	// construction/destruction
	nes_algn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual uint8_t* get_cart_base();
	virtual void write_prg(uint32_t offset, uint8_t data) override;

protected:
	nes_algn_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// ======================> nes_algq_rom_device

class nes_algq_rom_device : public nes_algn_rom_device
{
public:
	// construction/destruction
	nes_algq_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void write_prg(uint32_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_bank_base;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_ALGN_ROM, nes_algn_rom_device)
DECLARE_DEVICE_TYPE(NES_ALGQ_ROM, nes_algq_rom_device)


//-----------------------------------------------
//
//  Codemasters Aladdin passthru implementation
//
//-----------------------------------------------

// ======================> nes_aladdin_device

class nes_aladdin_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_aladdin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<nes_aladdin_slot_device> m_subslot;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_ALADDIN, nes_aladdin_device)

#endif // MAME_BUS_NES_ALADDIN_H
