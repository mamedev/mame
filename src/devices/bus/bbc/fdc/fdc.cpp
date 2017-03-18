// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        BBC Micro Floppy Disc Controller slot emulation

**********************************************************************/

#include "emu.h"
#include "fdc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type BBC_FDC_SLOT = device_creator<bbc_fdc_slot_device>;


//**************************************************************************
//  DEVICE BBC_FDC CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_fdc_interface - constructor
//-------------------------------------------------

device_bbc_fdc_interface::device_bbc_fdc_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_fdc_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_bbc_fdc_interface - destructor
//-------------------------------------------------

device_bbc_fdc_interface::~device_bbc_fdc_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_fdc_slot_device - constructor
//-------------------------------------------------

bbc_fdc_slot_device::bbc_fdc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, BBC_FDC_SLOT, "BBC Micro FDC slot", tag, owner, clock, "bbc_fdc_slot", __FILE__),
		device_slot_interface(mconfig, *this),
	m_card(nullptr),
	m_intrq_handler(*this),
	m_drq_handler(*this)
{
}


//-------------------------------------------------
//  bbc_fdc_slot_device - destructor
//-------------------------------------------------

bbc_fdc_slot_device::~bbc_fdc_slot_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_fdc_slot_device::device_start()
{
	m_card = dynamic_cast<device_bbc_fdc_interface *>(get_card_device());

	// resolve callbacks
	m_intrq_handler.resolve_safe();
	m_drq_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_fdc_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_fdc_devices )
//-------------------------------------------------


// slot devices
#include "acorn.h"
#include "cumana.h"
#include "opus.h"
#include "watford.h"


SLOT_INTERFACE_START( bbc_fdc_devices )
	SLOT_INTERFACE("acorn8271", BBC_ACORN8271)
	SLOT_INTERFACE("acorn1770", BBC_ACORN1770)
	SLOT_INTERFACE("cumana1",   BBC_CUMANA1)
	SLOT_INTERFACE("cumana2",   BBC_CUMANA2)
	SLOT_INTERFACE("opus2791",  BBC_OPUS2791)
	SLOT_INTERFACE("opus2793",  BBC_OPUS2793)
	SLOT_INTERFACE("opus1770",  BBC_OPUS1770)
	SLOT_INTERFACE("weddb2",    BBC_WEDDB2)
	SLOT_INTERFACE("weddb3",    BBC_WEDDB3)
SLOT_INTERFACE_END
