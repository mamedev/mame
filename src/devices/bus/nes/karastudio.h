// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_KARASTUDIO_H
#define __NES_KARASTUDIO_H

#include "nxrom.h"


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
	kstudio_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~kstudio_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);

	UINT8 *get_cart_base() { return m_rom; }
	void write_prg_bank(UINT8 bank) { m_bank = bank; }

protected:
	// internal state
	UINT8 *m_rom;
	// ROM is accessed via two 16K banks, but only the first one can be switched
	UINT8 m_bank;
};

// ======================> nes_kstudio_slot_device

class nes_kstudio_slot_device : public device_t,
									public device_image_interface,
									public device_slot_interface
{
public:
	// construction/destruction
	nes_kstudio_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~nes_kstudio_slot_device();

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override { update_names(); }

	// image-level overrides
	virtual bool call_load() override;
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) override;

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "ks_cart"; }
	virtual const char *file_extensions() const override { return "bin"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	// slot interface overrides
	virtual std::string get_default_card_software() override;

	virtual DECLARE_READ8_MEMBER(read);
	void write_prg_bank(UINT8 bank) { if (m_cart) m_cart->write_prg_bank(bank); }

	kstudio_cart_interface*      m_cart;
};

// device type definition
extern const device_type NES_KSEXPANSION_SLOT;


#define MCFG_KSTUDIO_MINICART_ADD(_tag, _slot_intf) \
	MCFG_DEVICE_ADD(_tag, NES_KSEXPANSION_SLOT, 0) \
MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, NULL, false)


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
	nes_kstudio_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual UINT8* get_cart_base();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// device type definition
extern const device_type NES_KSEXPANSION_ROM;


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
	nes_karaokestudio_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	int m_exp_active;
	required_device<nes_kstudio_slot_device> m_subslot;
	required_ioport m_mic_ipt;
};


// device type definition
extern const device_type NES_KARAOKESTUDIO;

#endif
