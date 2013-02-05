/**********************************************************************

    Commodore PET Memory Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/petexp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PET_EXPANSION_SLOT = &device_creator<pet_expansion_slot_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_expansion_slot_device - constructor
//-------------------------------------------------

pet_expansion_slot_device::pet_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PET_EXPANSION_SLOT, "PET memory expansion port", tag, owner, clock),
	device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  pet_expansion_slot_device - destructor
//-------------------------------------------------

pet_expansion_slot_device::~pet_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_pet_expansion_card_interface - constructor
//-------------------------------------------------

device_pet_expansion_card_interface::device_pet_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<pet_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_pet_expansion_card_interface - destructor
//-------------------------------------------------

device_pet_expansion_card_interface::~device_pet_expansion_card_interface()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pet_expansion_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const pet_expansion_slot_interface *intf = reinterpret_cast<const pet_expansion_slot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<pet_expansion_slot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_dma_bd_cb, 0, sizeof(m_in_dma_bd_cb));
		memset(&m_out_dma_bd_cb, 0, sizeof(m_out_dma_bd_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_pet_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_in_dma_bd_func.resolve(m_in_dma_bd_cb, *this);
	m_out_dma_bd_func.resolve(m_out_dma_bd_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pet_expansion_slot_device::device_reset()
{
	if (m_card != NULL)
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  norom_r - NO ROM read
//-------------------------------------------------

int pet_expansion_slot_device::norom_r(address_space &space, offs_t offset, int sel)
{
	return m_card ? m_card->pet_norom_r(space, offset, sel) : 1;
}


//-------------------------------------------------
//  read - buffered data read
//-------------------------------------------------

UINT8 pet_expansion_slot_device::read(address_space &space, offs_t offset, UINT8 data, int sel)
{
	if (m_card != NULL)
	{
		data = m_card->pet_bd_r(space, offset, data, sel);
	}

	return data;
}


//-------------------------------------------------
//  write - buffered data write
//-------------------------------------------------

void pet_expansion_slot_device::write(address_space &space, offs_t offset, UINT8 data, int sel)
{
	if (m_card != NULL)
	{
		m_card->pet_bd_w(space, offset, data, sel);
	}
}


//-------------------------------------------------
//  diag_r - DIAG read
//-------------------------------------------------

READ_LINE_MEMBER( pet_expansion_slot_device::diag_r )
{
	return m_card ? m_card->pet_diag_r() : 1;
}


//-------------------------------------------------
//  irq_w - IRQ write
//-------------------------------------------------

WRITE_LINE_MEMBER( pet_expansion_slot_device::irq_w )
{
	if (m_card) m_card->pet_irq_w(state);
}


//-------------------------------------------------
//  dma_bd_r - DMA read
//-------------------------------------------------

UINT8 pet_expansion_slot_device::dma_bd_r(offs_t offset)
{
	return m_in_dma_bd_func(offset);
}


//-------------------------------------------------
//  dma_bd_w - DMA write
//-------------------------------------------------

void pet_expansion_slot_device::dma_bd_w(offs_t offset, UINT8 data)
{
	m_out_dma_bd_func(offset, data);
}


//-------------------------------------------------
//  phi2 - system clock frequency
//-------------------------------------------------

int pet_expansion_slot_device::phi2()
{
	return clock();
}
