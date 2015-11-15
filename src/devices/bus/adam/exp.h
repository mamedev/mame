// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Expansion Port emulation

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

#define MCFG_ADAM_EXPANSION_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ADAM_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_ADAM_EXPANSION_SLOT_IRQ_CALLBACK(_write) \
	devcb = &adam_expansion_slot_device::set_irq_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_expansion_slot_device

class device_adam_expansion_slot_card_interface;

class adam_expansion_slot_device : public device_t,
									public device_slot_interface,
									public device_image_interface
{
public:
	// construction/destruction
	adam_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~adam_expansion_slot_device() { }

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<adam_expansion_slot_device &>(device).m_write_irq.set_callback(object); }

	// computer interface
	UINT8 bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2);
	void bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2);

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }

protected:
	// device-level overrides
	virtual void device_config_complete() { update_names(); }
	virtual void device_start();
	virtual void device_reset();

	// image-level overrides
	virtual bool call_load();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

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
	virtual void get_default_card_software(std::string &result);

	devcb_write_line   m_write_irq;

	device_adam_expansion_slot_card_interface *m_card;
};


// ======================> device_adam_expansion_slot_card_interface

class device_adam_expansion_slot_card_interface : public device_slot_card_interface
{
	friend class adam_expansion_slot_device;

public:
	// construction/destruction
	device_adam_expansion_slot_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_adam_expansion_slot_card_interface() { }

protected:
	// runtime
	virtual UINT8 adam_bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) { return data; }
	virtual void adam_bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2) { }

	adam_expansion_slot_device *m_slot;

	optional_shared_ptr<UINT8> m_rom;
};


// device type definition
extern const device_type ADAM_EXPANSION_SLOT;


SLOT_INTERFACE_EXTERN( adam_slot1_devices );
SLOT_INTERFACE_EXTERN( adam_slot2_devices );
SLOT_INTERFACE_EXTERN( adam_slot3_devices );



#endif
