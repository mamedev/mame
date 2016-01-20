// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET Memory Expansion Port emulation

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

#define MCFG_PET_EXPANSION_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, PET_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_PET_EXPANSION_SLOT_DMA_CALLBACKS(_read, _write) \
	downcast<pet_expansion_slot_device *>(device)->set_callbacks(DEVCB_##_read, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_expansion_slot_device

class device_pet_expansion_card_interface;

class pet_expansion_slot_device : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	pet_expansion_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~pet_expansion_slot_device();

	template<class _read, class _write> void set_callbacks(_read rd, _write wr) {
		m_read_dma.set_callback(rd);
		m_write_dma.set_callback(wr);
	}

	// computer interface
	int norom_r(address_space &space, offs_t offset, int sel);
	UINT8 read(address_space &space, offs_t offset, UINT8 data, int &sel);
	void write(address_space &space, offs_t offset, UINT8 data, int &sel);
	DECLARE_READ_LINE_MEMBER( diag_r );
	DECLARE_WRITE_LINE_MEMBER( irq_w );

	// cartridge interface
	UINT8 dma_bd_r(offs_t offset);
	void dma_bd_w(offs_t offset, UINT8 data);
	int phi2();

	enum
	{
		SEL_NONE = -1,
		SEL0 = 0,
		SEL1,
		SEL2,
		SEL3,
		SEL4,
		SEL5,
		SEL6,
		SEL7,
		SEL8,
		SEL9,
		SELA,
		SELB,
		SELC,
		SELD,
		SELE,
		SELF
	};

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	device_pet_expansion_card_interface *m_card;

	devcb_read8  m_read_dma;
	devcb_write8 m_write_dma;
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
	virtual UINT8 pet_bd_r(address_space &space, offs_t offset, UINT8 data, int &sel) { return data; };
	virtual void pet_bd_w(address_space &space, offs_t offset, UINT8 data, int &sel) { };
	virtual int pet_diag_r() { return 1; }
	virtual void pet_irq_w(int state) { }

	pet_expansion_slot_device *m_slot;
};


// device type definition
extern const device_type PET_EXPANSION_SLOT;


SLOT_INTERFACE_EXTERN( pet_expansion_cards );



#endif
