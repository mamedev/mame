// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Micro Voice Synthesis Processor slot emulation

    This socket is intended to have a TMS5220 fitted as part of the Acorn
    Speech System upgrade, but Cheetah provided a cheaper speech solution
    using a SP0256 that fitted a board into this socket.

**********************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_VSP_SLOT, bbc_vsp_slot_device, "bbc_vsp_slot", "BBC Micro Voice Synthesis Processor slot")


//**************************************************************************
//  DEVICE BBC_VSP CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_vsp_interface - constructor
//-------------------------------------------------

device_bbc_vsp_interface::device_bbc_vsp_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "bbcvsp")
	, m_slot(dynamic_cast<bbc_vsp_slot_device *>(device.owner()))
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_vsp_slot_device - constructor
//-------------------------------------------------

bbc_vsp_slot_device::bbc_vsp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_VSP_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_bbc_vsp_interface>(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
	, m_vsm(*this, finder_base::DUMMY_TAG)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_vsp_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_vsp_slot_device::read()
{
	if (m_card)
		return m_card->read();
	else
		return 0x00;
}

int bbc_vsp_slot_device::readyq_r()
{
	if (m_card)
		return m_card->readyq_r();
	else
		return 1;
}

int bbc_vsp_slot_device::intq_r()
{
	if (m_card)
		return m_card->intq_r();
	else
		return 1;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void bbc_vsp_slot_device::write(uint8_t data)
{
	if (m_card)
		m_card->write(data);
}

void bbc_vsp_slot_device::combined_rsq_wsq_w(uint8_t data)
{
	if (m_card)
		m_card->combined_rsq_wsq_w(data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_vsp_devices )
//-------------------------------------------------


// slot devices
#include "speech.h"
#include "sweetalker.h"


void bbc_vsp_devices(device_slot_interface &device)
{
	device.option_add("speech", BBC_SPEECH);
	device.option_add("sweetalker", BBC_SWEETALKER);
}
