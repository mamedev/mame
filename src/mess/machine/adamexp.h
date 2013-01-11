/**********************************************************************

    Coleco Adam Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __ADAM_EXPANSION_SLOT__
#define __ADAM_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define ADAM_LEFT_EXPANSION_SLOT_TAG        "slot1"
#define ADAM_CENTER_EXPANSION_SLOT_TAG      "slot2"
#define ADAM_RIGHT_EXPANSION_SLOT_TAG       "slot3"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define ADAM_EXPANSION_SLOT_INTERFACE(_name) \
	const adam_expansion_slot_interface (_name) =


#define MCFG_ADAM_EXPANSION_SLOT_ADD(_tag, _clock, _config, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, ADAM_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_expansion_slot_interface

struct adam_expansion_slot_interface
{
	devcb_write_line    m_out_int_cb;
};


// ======================> adam_expansion_slot_device

class device_adam_expansion_slot_card_interface;

class adam_expansion_slot_device : public device_t,
									public adam_expansion_slot_interface,
									public device_slot_interface,
									public device_image_interface
{
public:
	// construction/destruction
	adam_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~adam_expansion_slot_device();

	// computer interface
	UINT8 bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2);
	void bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2);

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( int_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// image-level overrides
	virtual bool call_load();
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return "adam_rom"; }
	virtual const char *file_extensions() const { return "bin,rom"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// slot interface overrides
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options);

	devcb_resolved_write_line   m_out_int_func;

	device_adam_expansion_slot_card_interface *m_cart;
};


// ======================> device_adam_expansion_slot_card_interface

class device_adam_expansion_slot_card_interface : public device_slot_card_interface
{
	friend class adam_expansion_slot_device;

public:
	// construction/destruction
	device_adam_expansion_slot_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_adam_expansion_slot_card_interface();

protected:
	// initialization
	virtual UINT8* adam_rom_pointer(running_machine &machine, size_t size);
	virtual UINT8* adam_ram_pointer(running_machine &machine, size_t size);

	// runtime
	virtual UINT8 adam_bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) { return data; }
	virtual void adam_bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) { }

	adam_expansion_slot_device *m_slot;

	UINT8 *m_rom;
	UINT8 *m_ram;

	size_t m_rom_mask;
	size_t m_ram_mask;
};


// device type definition
extern const device_type ADAM_EXPANSION_SLOT;


// slot devices
#include "machine/adamlink.h"
#include "machine/adam_ide.h"
#include "machine/adam_ram.h"

SLOT_INTERFACE_EXTERN( adam_slot1_devices );
SLOT_INTERFACE_EXTERN( adam_slot2_devices );
SLOT_INTERFACE_EXTERN( adam_slot3_devices );



#endif
