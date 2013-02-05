/**********************************************************************

    Commodore PET Memory Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

**********************************************************************/

#pragma once

#ifndef __PET_EXPANSION_SLOT__
#define __PET_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PET_EXPANSION_SLOT_TAG     "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define PET_EXPANSION_INTERFACE(_name) \
	const pet_expansion_slot_interface (_name) =


#define MCFG_PET_EXPANSION_SLOT_ADD(_tag, _clock, _config, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, PET_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_expansion_slot_interface

struct pet_expansion_slot_interface
{
	devcb_read8         m_in_dma_bd_cb;
	devcb_write8        m_out_dma_bd_cb;
};


// ======================> pet_expansion_slot_device

class device_pet_expansion_card_interface;

class pet_expansion_slot_device : public device_t,
									public device_slot_interface,
									public pet_expansion_slot_interface
{
public:
	// construction/destruction
	pet_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~pet_expansion_slot_device();

	// computer interface
	int norom_r(address_space &space, offs_t offset, int sel);
	UINT8 read(address_space &space, offs_t offset, UINT8 data, int sel);
	void write(address_space &space, offs_t offset, UINT8 data, int sel);
	DECLARE_READ_LINE_MEMBER( diag_r );
	DECLARE_WRITE_LINE_MEMBER( irq_w );

	// cartridge interface
	UINT8 dma_bd_r(offs_t offset);
	void dma_bd_w(offs_t offset, UINT8 data);
	int phi2();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	device_pet_expansion_card_interface *m_card;

	devcb_resolved_read8        m_in_dma_bd_func;
	devcb_resolved_write8       m_out_dma_bd_func;
};


// ======================> device_pet_expansion_card_interface

class device_pet_expansion_card_interface : public device_slot_card_interface
{
	friend class pet_expansion_slot_device;

public:
	// construction/destruction
	device_pet_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pet_expansion_card_interface();

protected:
	// runtime
	virtual int pet_norom_r(address_space &space, offs_t offset, int sel) { return 1; }
	virtual UINT8 pet_bd_r(address_space &space, offs_t offset, UINT8 data, int sel) { return data; };
	virtual void pet_bd_w(address_space &space, offs_t offset, UINT8 data, int sel) { };
	virtual int pet_diag_r() { return 1; }
	virtual void pet_irq_w(int state) { }

	pet_expansion_slot_device *m_slot;
};


// device type definition
extern const device_type PET_EXPANSION_SLOT;



#endif
