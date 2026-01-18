// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Master Internal Modem port

**********************************************************************/

#include "emu.h"
#include "modem.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MODEM_SLOT, bbc_modem_slot_device, "bbc_modem_slot", "BBC Master Internal Modem port")


//**************************************************************************
//  DEVICE BBC_MODEM PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_modem_interface - constructor
//-------------------------------------------------

device_bbc_modem_interface::device_bbc_modem_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "bbcmodem")
	, m_slot(dynamic_cast<bbc_modem_slot_device*>(device.owner()))
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_modem_slot_device - constructor
//-------------------------------------------------

bbc_modem_slot_device::bbc_modem_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_MODEM_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_bbc_modem_interface>(mconfig, *this)
	, m_card(nullptr)
	, m_irq_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_modem_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_modem_slot_device::read(offs_t offset)
{
	if (m_card)
		return m_card->read(offset & 0x0f);
	else
		return 0xfe;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void bbc_modem_slot_device::write(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->write(offset & 0x0f, data);
}

//-------------------------------------------------
//  SLOT_INTERFACE( bbcm_modem_devices )
//-------------------------------------------------


// slot devices
//#include "master_modem.h"
#include "meup.h"
#include "scsiaiv.h"


void bbcm_modem_devices(device_slot_interface &device)
{
	device.option_add("meup",         BBC_MEUP);            // Master Extra User Ports
	//device.option_add("modem",        BBC_MASTER_MODEM);    // Beebug Master Modem
	device.option_add("scsiaiv",      BBC_SCSIAIV);         // Acorn AIV SCSI Host Adaptor
	//device.option_add("vp415",        BBC_VP415);           // Philips VP415
}
