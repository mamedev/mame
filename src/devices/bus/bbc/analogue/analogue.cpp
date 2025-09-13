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
	: device_interface(device, "bbcanalogue")
	, m_slot(dynamic_cast<bbc_analogue_slot_device*>(device.owner()))
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_analogue_slot_device - constructor
//-------------------------------------------------

bbc_analogue_slot_device::bbc_analogue_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_ANALOGUE_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_bbc_analogue_interface>(mconfig, *this)
	, m_card(nullptr)
	, m_lpstb_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_analogue_slot_device::device_start()
{
	m_card = get_card_device();
}

uint16_t bbc_analogue_slot_device::ch_r(offs_t channel)
{
	if (m_card)
		return m_card->ch_r(channel);
	else
		return 0xfff0;
}

uint8_t bbc_analogue_slot_device::pb_r()
{
	if (m_card)
		return m_card->pb_r();
	else
		return 0x30;
}

void bbc_analogue_slot_device::pb_w(uint8_t data)
{
	if (m_card)
		m_card->pb_w(data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_analogue_devices )
//-------------------------------------------------


// slot devices
#include "bitstik.h"
#include "cfa3000a.h"
//#include "colourlp.h"
//#include "i2c_rtc.h"
#include "joystick.h"
#include "lightpen.h"
#include "micromike.h"
#include "quinkey.h"


void bbc_analogue_devices_no_lightpen(device_slot_interface &device)
{
	device.option_add("acornjoy",    BBC_ACORNJOY);         /* Acorn ANH01 Joysticks */
	device.option_add("bitstik1",    BBC_BITSTIK1);         /* Acorn ANF04 Bitstik */
	device.option_add("bitstik2",    BBC_BITSTIK2);         /* Robocom Bitstik 2 */
	//device.option_add("i2c_rtc",     BBC_I2C_RTCA);         /* I2C RTC Module */
	device.option_add("micromike",   BBC_MICROMIKE);        /* Micro Mike */
	device.option_add("voltmace3b",  BBC_VOLTMACE3B);       /* Voltmace Delta 3b "Twin" Joysticks */
	device.option_add("quinkey",     BBC_QUINKEY_INTF);     /* Microwriter Quinkey */
	device.option_add_internal("cfa3000a", CFA3000_ANLG);   /* Hanson CFA 3000 Analogue */
}

void bbc_analogue_devices(device_slot_interface &device)
{
	bbc_analogue_devices_no_lightpen(device);
	//device.option_add("colourlp",    BBC_COLOURLP);         /* RH Electronics Colour Light Pen */
	device.option_add("datapen",     BBC_DATAPEN);          /* Datapen Light Pen */
	device.option_add("robinlp",     BBC_ROBINLP);          /* The Robin Light Pen */
	device.option_add("stacklp",     BBC_STACKLP);          /* Stack Light Pen */
	device.option_add("stacklr",     BBC_STACKLR);          /* Stack Light Rifle */
}
