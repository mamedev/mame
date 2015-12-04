// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_ALADDIN_H
#define __NES_ALADDIN_H

#include "nxrom.h"


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
	aladdin_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~aladdin_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);

	UINT8 *get_cart_base() { return m_rom; }
	void set_cart_size(UINT32 size) { m_rom_size = size; m_rom_mask = (size / 0x4000) - 1; }
	virtual void write_prg(UINT32 offset, UINT8 data) { }

protected:
	// internal state
	UINT8 *m_rom;
	UINT32 m_rom_size;
	UINT8 m_lobank, m_hibank, m_rom_mask;
};

// ======================> nes_aladdin_slot_device

class nes_aladdin_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	nes_aladdin_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~nes_aladdin_slot_device();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { update_names(); }

	// image-level overrides
	virtual bool call_load();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return "ade_cart"; }
	virtual const char *file_extensions() const { return "nes,bin"; }
	virtual const option_guide *create_option_guide() const { return nullptr; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	virtual DECLARE_READ8_MEMBER(read);
	void write_prg(UINT32 offset, UINT8 data) { if (m_cart) m_cart->write_prg(offset, data); }

	aladdin_cart_interface*      m_cart;
};

// device type definition
extern const device_type NES_ALADDIN_SLOT;


#define MCFG_ALADDIN_MINICART_ADD(_tag, _slot_intf) \
	MCFG_DEVICE_ADD(_tag, NES_ALADDIN_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, NULL, false)


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
	nes_algn_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_algn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual UINT8* get_cart_base();
	virtual void write_prg(UINT32 offset, UINT8 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
};


// ======================> nes_algq_rom_device

class nes_algq_rom_device : public nes_algn_rom_device
{
public:
	// construction/destruction
	nes_algq_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual void write_prg(UINT32 offset, UINT8 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	UINT8 m_bank_base;
};

// device type definition
extern const device_type NES_ALGN_ROM;
extern const device_type NES_ALGQ_ROM;


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
	nes_aladdin_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

protected:
	required_device<nes_aladdin_slot_device> m_subslot;
};


// device type definition
extern const device_type NES_ALADDIN;

#endif
