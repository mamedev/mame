// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET Memory Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PET_EXPANSION_SLOT, pet_expansion_slot_device, "pet_expansion_slot", "PET memory expansion port")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_expansion_slot_device - constructor
//-------------------------------------------------

pet_expansion_slot_device::pet_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PET_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_pet_expansion_card_interface>(mconfig, *this),
	m_card(nullptr),
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
	: device_interface(device, "petexp")
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
	m_card = get_card_device();

	// resolve callbacks
	m_read_dma.resolve_safe(0);
	m_write_dma.resolve_safe();
}


//-------------------------------------------------
//  norom_r - NO ROM read
//-------------------------------------------------

int pet_expansion_slot_device::norom_r(offs_t offset, int sel)
{
	return m_card ? m_card->pet_norom_r(offset, sel) : 1;
}


//-------------------------------------------------
//  read - buffered data read
//-------------------------------------------------

uint8_t pet_expansion_slot_device::read(offs_t offset, uint8_t data, int &sel)
{
	if (m_card)
		data = m_card->pet_bd_r(offset, data, sel);

	return data;
}


//-------------------------------------------------
//  write - buffered data write
//-------------------------------------------------

void pet_expansion_slot_device::write(offs_t offset, uint8_t data, int &sel)
{
	if (m_card != nullptr)
	{
		m_card->pet_bd_w(offset, data, sel);
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

uint8_t pet_expansion_slot_device::dma_bd_r(offs_t offset)
{
	return m_read_dma(offset);
}


//-------------------------------------------------
//  dma_bd_w - DMA write
//-------------------------------------------------

void pet_expansion_slot_device::dma_bd_w(offs_t offset, uint8_t data)
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

void pet_expansion_cards(device_slot_interface &device)
{
	device.option_add("64k", PET_64K);
	device.option_add("hsga", CBM8000_HSG_A);
	device.option_add("hsgb", CBM8000_HSG_B);
	device.option_add("superpet", SUPERPET);
}
