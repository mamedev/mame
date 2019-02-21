// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_KARASTUDIO_H
#define MAME_BUS_NES_KARASTUDIO_H

#pragma once

#include "nxrom.h"
#include "softlist_dev.h"


//-----------------------------------------
//
//  Karaoke Studio Cartslot implementation
//
//-----------------------------------------

// ======================> kstudio_cart_interface

class kstudio_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~kstudio_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);

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
									public device_image_interface,
									public device_slot_interface
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

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "ks_cart"; }
	virtual const char *file_extensions() const override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	virtual DECLARE_READ8_MEMBER(read);
	void write_prg_bank(uint8_t bank) { if (m_cart) m_cart->write_prg_bank(bank); }

protected:
	// device-level overrides
	virtual void device_start() override;

	kstudio_cart_interface*      m_cart;
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

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual uint8_t* get_cart_base();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
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

	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	int m_exp_active;
	required_device<nes_kstudio_slot_device> m_subslot;
	required_ioport m_mic_ipt;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_KARAOKESTUDIO, nes_karaokestudio_device)

#endif // MAME_BUS_NES_KARASTUDIO_H
