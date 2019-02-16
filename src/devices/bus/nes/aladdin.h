// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_ALADDIN_H
#define MAME_BUS_NES_ALADDIN_H

#pragma once

#include "nxrom.h"
#include "softlist_dev.h"


//----------------------------------
//
//  Aladdin Cartslot implementation
//
//----------------------------------

// ======================> aladdin_cart_interface

class aladdin_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~aladdin_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);

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
								public device_image_interface,
								public device_slot_interface
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
	virtual const char *image_interface() const override { return "ade_cart"; }
	virtual const char *file_extensions() const override { return "nes,bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	virtual DECLARE_READ8_MEMBER(read);
	void write_prg(uint32_t offset, uint8_t data) { if (m_cart) m_cart->write_prg(offset, data); }

protected:
	aladdin_cart_interface*      m_cart;
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
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual uint8_t* get_cart_base();
	virtual void write_prg(uint32_t offset, uint8_t data) override;

protected:
	nes_algn_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
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
	virtual void device_start() override;
	virtual void device_reset() override;

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

	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void device_add_mconfig(machine_config &config) override;

	required_device<nes_aladdin_slot_device> m_subslot;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_ALADDIN, nes_aladdin_device)

#endif // MAME_BUS_NES_ALADDIN_H
