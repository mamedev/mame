// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Colecovision Expansion Slot

    60-pin slot

***************************************************************************/

#include "emu.h"
#include "expansion.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COLECO_EXPANSION, coleco_expansion_device, "coleco_expansion", "Colecovision Expansion Bus")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

coleco_expansion_device::coleco_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COLECO_EXPANSION, tag, owner, clock),
	device_single_card_slot_interface<device_coleco_expansion_interface>(mconfig, *this),
	device_mixer_interface(mconfig, *this),
	m_program(*this, finder_base::DUMMY_TAG, -1),
	m_io(*this, finder_base::DUMMY_TAG, -1),
	m_int_handler(*this),
	m_nmi_handler(*this),
	m_card(nullptr)
{
}

coleco_expansion_device::~coleco_expansion_device()
{
}

void coleco_expansion_device::device_start()
{
	// get inserted card
	m_card = get_card_device();
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_coleco_expansion_interface::device_coleco_expansion_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "colecoexp")
{
	m_expansion = dynamic_cast<coleco_expansion_device *>(device.owner());
}

device_coleco_expansion_interface::~device_coleco_expansion_interface()
{
}
