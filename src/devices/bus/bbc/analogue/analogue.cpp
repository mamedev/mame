// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC analogue port emulation

**********************************************************************/

#include "emu.h"
#include "analogue.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_ANALOGUE_SLOT, bbc_analogue_slot_device, "bbc_analogue_slot", "BBC Micro Analogue port")



//**************************************************************************
//  DEVICE BBC_ANALOGUE PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_analogue_interface - constructor
//-------------------------------------------------

device_bbc_analogue_interface::device_bbc_analogue_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_analogue_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_analogue_slot_device - constructor
//-------------------------------------------------

bbc_analogue_slot_device::bbc_analogue_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_ANALOGUE_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_card(nullptr),
	m_lpstb_handler(*this)
{
}


void bbc_analogue_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const carddev = get_card_device();
	if (carddev && !dynamic_cast<device_bbc_analogue_interface *>(carddev))
		osd_printf_error("Card device %s (%s) does not implement device_bbc_analogue_interface\n", carddev->tag(), carddev->name());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_analogue_slot_device::device_start()
{
	device_t *const carddev = get_card_device();
	m_card = dynamic_cast<device_bbc_analogue_interface *>(get_card_device());
	if (carddev && !m_card)
		osd_printf_error("Card device %s (%s) does not implement device_bbc_analogue_interface\n", carddev->tag(), carddev->name());

	// resolve callbacks
	m_lpstb_handler.resolve_safe();
}

uint8_t bbc_analogue_slot_device::ch_r(int channel)
{
	if (m_card)
		return m_card->ch_r(channel);
	else
		return 0x00;
}

uint8_t bbc_analogue_slot_device::pb_r()
{
	if (m_card)
		return m_card->pb_r();
	else
		return 0x30;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_analogue_slot_device::device_reset()
{
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_analogue_devices )
//-------------------------------------------------


// slot devices
#include "joystick.h"
#include "bitstik.h"
//#include "lightpen.h"
//#include "micromike.h"
//#include "quinkey.h"
#include "cfa3000a.h"


void bbc_analogue_devices(device_slot_interface &device)
{
	device.option_add("acornjoy",    BBC_ACORNJOY);         /* Acorn ANH01 Joysticks */
	device.option_add("bitstik1",    BBC_BITSTIK1);         /* Acorn ANF04 Bitstik */
	device.option_add("bitstik2",    BBC_BITSTIK2);         /* Robocom Bitstik 2 */
	//device.option_add("lightpen",    BBC_LIGHTPEN);         /* RH Electronics Lightpen */
	//device.option_add("micromike",   BBC_MICROMIKE);        /* Micro Mike */
	device.option_add("voltmace3b",  BBC_VOLTMACE3B);       /* Voltmace Delta 3b "Twin" Joysticks */
	//device.option_add("quinkey",     BBC_QUINKEY);          /* Microwriter Quinkey */
	device.option_add("cfa3000a",    CFA3000_ANLG);         /* Hanson CFA 3000 Analogue */
}
