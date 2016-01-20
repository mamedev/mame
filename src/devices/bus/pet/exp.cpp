// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET Memory Expansion Port emulation

**********************************************************************/

#include "exp.h"



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

pet_expansion_slot_device::pet_expansion_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PET_EXPANSION_SLOT, "PET memory expansion port", tag, owner, clock, "pet_expansion_slot", __FILE__),
	device_slot_interface(mconfig, *this), m_card(nullptr),
	m_read_dma(*this),
	m_write_dma(*this)
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
//  device_start - device-specific startup
//-------------------------------------------------

void pet_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_pet_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_read_dma.resolve_safe(0);
	m_write_dma.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pet_expansion_slot_device::device_reset()
{
	if (m_card != nullptr)
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

UINT8 pet_expansion_slot_device::read(address_space &space, offs_t offset, UINT8 data, int &sel)
{
	if (m_card != nullptr)
	{
		data = m_card->pet_bd_r(space, offset, data, sel);
	}

	return data;
}


//-------------------------------------------------
//  write - buffered data write
//-------------------------------------------------

void pet_expansion_slot_device::write(address_space &space, offs_t offset, UINT8 data, int &sel)
{
	if (m_card != nullptr)
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
	return m_read_dma(offset);
}


//-------------------------------------------------
//  dma_bd_w - DMA write
//-------------------------------------------------

void pet_expansion_slot_device::dma_bd_w(offs_t offset, UINT8 data)
{
	m_write_dma(offset, data);
}


//-------------------------------------------------
//  phi2 - system clock frequency
//-------------------------------------------------

int pet_expansion_slot_device::phi2()
{
	return clock();
}


//-------------------------------------------------
//  SLOT_INTERFACE( pet_expansion_cards )
//-------------------------------------------------

// slot devices
#include "64k.h"
#include "hsg.h"
#include "superpet.h"

SLOT_INTERFACE_START( pet_expansion_cards )
	SLOT_INTERFACE("64k", PET_64K)
	SLOT_INTERFACE("hsga", CBM8000_HSG_A)
	SLOT_INTERFACE("hsgb", CBM8000_HSG_B)
	SLOT_INTERFACE("superpet", SUPERPET)
SLOT_INTERFACE_END
