// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_DATACH_H
#define __NES_DATACH_H

#include "bandai.h"
#include "machine/i2cmem.h"
#include "machine/bcreader.h"

//--------------------------------
//
//  Datach Cartslot implementation
//
//--------------------------------

// ======================> datach_cart_interface

class datach_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	datach_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~datach_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);

	UINT8 *get_cart_base() { return m_rom; }
	void write_prg_bank(UINT8 bank) { m_bank = bank; }

	optional_device<i2cmem_device> m_i2cmem;

protected:
	// internal state
	UINT8 *m_rom;
	// ROM is accessed via two 16K banks, but only the first one can be switched
	UINT8 m_bank;
};

// ======================> nes_datach_slot_device

class nes_datach_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	nes_datach_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~nes_datach_slot_device();

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
	virtual const char *image_interface() const { return "datach_cart"; }
	virtual const char *file_extensions() const { return "nes,bin"; }
	virtual const option_guide *create_option_guide() const { return nullptr; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	virtual DECLARE_READ8_MEMBER(read);
	void write_prg_bank(UINT8 bank) { if (m_cart) m_cart->write_prg_bank(bank); }

	datach_cart_interface*      m_cart;
};

// device type definition
extern const device_type NES_DATACH_SLOT;


#define MCFG_DATACH_MINICART_ADD(_tag, _slot_intf) \
		MCFG_DEVICE_ADD(_tag, NES_DATACH_SLOT, 0) \
		MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, NULL, false)


//--------------------------------
//
//  Datach Minicart implementation
//
//--------------------------------

// ======================> nes_datach_rom_device

class nes_datach_rom_device : public device_t,
							public datach_cart_interface
{
public:
	// construction/destruction
	nes_datach_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_datach_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual UINT8* get_cart_base();

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
};

// ======================> nes_datach_24c01_device

class nes_datach_24c01_device : public nes_datach_rom_device
{
public:
	// construction/destruction
	nes_datach_24c01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
};

// device type definition
extern const device_type NES_DATACH_ROM;
extern const device_type NES_DATACH_24C01;


//---------------------------------
//
//  Datach Base Unit implementation
//
//---------------------------------

// ======================> nes_datach_device

class nes_datach_device : public nes_lz93d50_device
{
public:
	// construction/destruction
	nes_datach_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

protected:
	UINT8 m_datach_latch;
	required_device<i2cmem_device> m_i2cmem;
	required_device<barcode_reader_device> m_reader;
	required_device<nes_datach_slot_device> m_subslot;
	UINT8 m_i2c_dir;
	UINT8 m_i2c_in_use;

	static const device_timer_id TIMER_SERIAL = 1;
	emu_timer *serial_timer;
};


// device type definition
extern const device_type NES_DATACH;

#endif
