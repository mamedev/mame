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

DEFINE_DEVICE_TYPE(BBC_FDC_SLOT, bbc_fdc_slot_device, "bbc_fdc_slot", "BBC Micro FDC slot")


//**************************************************************************
//  DEVICE BBC_FDC CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_fdc_interface - constructor
//-------------------------------------------------

device_bbc_fdc_interface::device_bbc_fdc_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "bbcfdc")
{
	m_slot = dynamic_cast<bbc_fdc_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_fdc_slot_device - constructor
//-------------------------------------------------

bbc_fdc_slot_device::bbc_fdc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_FDC_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_bbc_fdc_interface>(mconfig, *this),
	m_card(nullptr),
	m_intrq_cb(*this),
	m_drq_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_fdc_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_intrq_cb.resolve_safe();
	m_drq_cb.resolve_safe();
}

//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_fdc_slot_device::read(offs_t offset)
{
	if (m_card)
		return m_card->read(offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void bbc_fdc_slot_device::write(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->write(offset, data);
}

//-------------------------------------------------
//  SLOT_INTERFACE( bbc_fdc_devices )
//-------------------------------------------------


// slot devices
#include "acorn.h"
#include "ams.h"
#include "cumana.h"
#include "cv1797.h"
#include "microware.h"
#include "opus.h"
//#include "solidisk.h"
#include "watford.h"


void bbc_fdc_devices(device_slot_interface &device)
{
	device.option_add("acorn8271", BBC_ACORN8271);
	device.option_add("acorn1770", BBC_ACORN1770);
	device.option_add("ams3",      BBC_AMS3);
	device.option_add("cumana1",   BBC_CUMANA1);
	device.option_add("cumana2",   BBC_CUMANA2);
	device.option_add("cv1797",    BBC_CV1797);
	device.option_add("microware", BBC_MICROWARE);
	device.option_add("opus8272",  BBC_OPUS8272);
	device.option_add("opus2791",  BBC_OPUS2791);
	device.option_add("opus2793",  BBC_OPUS2793);
	device.option_add("opus1770",  BBC_OPUS1770);
	//device.option_add("stl8271",   BBC_STL8271);
	//device.option_add("stl1770_1", BBC_STL1770_1);
	//device.option_add("stl1770_2", BBC_STL1770_2);
	device.option_add("weddb2",    BBC_WEDDB2);
	device.option_add("weddb3",    BBC_WEDDB3);
}
