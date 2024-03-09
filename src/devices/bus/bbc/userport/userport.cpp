// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC User Port emulation

**********************************************************************/

#include "emu.h"
#include "userport.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_USERPORT_SLOT, bbc_userport_slot_device, "bbc_userport_slot", "BBC Micro User port")



//**************************************************************************
//  DEVICE BBC_USERPORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_userport_interface - constructor
//-------------------------------------------------

device_bbc_userport_interface::device_bbc_userport_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "bbcuserport")
	, m_slot(dynamic_cast<bbc_userport_slot_device *>(device.owner()))
{
}


//-------------------------------------------------
//  ~device_bbc_userport_interface - destructor
//-------------------------------------------------

device_bbc_userport_interface::~device_bbc_userport_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_userport_slot_device - constructor
//-------------------------------------------------

bbc_userport_slot_device::bbc_userport_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_USERPORT_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_bbc_userport_interface>(mconfig, *this)
	, m_device(nullptr)
	, m_cb1_handler(*this)
	, m_cb2_handler(*this)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_userport_slot_device::device_start()
{
	m_device = get_card_device();
}


//-------------------------------------------------
//  pb_r
//-------------------------------------------------

uint8_t bbc_userport_slot_device::pb_r()
{
	if (m_device)
		return m_device->pb_r();
	else
		return 0xff;
}


//-------------------------------------------------
//  pb_w
//-------------------------------------------------

void bbc_userport_slot_device::pb_w(uint8_t data)
{
	if (m_device)
		m_device->pb_w(data);
}


//-------------------------------------------------
//  write_cb1
//-------------------------------------------------

void bbc_userport_slot_device::write_cb1(int state)
{
	if (m_device)
		m_device->write_cb1(state);
}


//-------------------------------------------------
//  write_cb2
//-------------------------------------------------

void bbc_userport_slot_device::write_cb2(int state)
{
	if (m_device)
		m_device->write_cb2(state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_userport_devices )
//-------------------------------------------------


// slot devices
#include "beebspch.h"
//#include "digitiser.h"
//#include "ev1.h"
#include "lcd.h"
#include "lvlecho.h"
#include "m4000.h"
#include "palext.h"
#include "pointer.h"
#include "sdcard.h"
#include "usersplit.h"
//#include "vci.h"
#include "voicebox.h"
#include "cfa3000kbd.h"


void bbc_userport_devices(device_slot_interface &device)
{
	device.option_add("amxmouse",   BBC_AMXMOUSE);        /* AMX Mouse */
	//device.option_add("atr",        BBC_ATR);             /* Advanced Teletext Receiver (GIS) */
	device.option_add("beebspch",   BBC_BEEBSPCH);        /* Beeb Speech Synthesiser (Watford Electronics) */
	//device.option_add("beebvdig",   BBC_BEEBVDIG);        /* Beeb Video Digitiser (Watford Electronics) */
	device.option_add("chameleon",  BBC_CHAMELEON);       /* Chameleon */
	device.option_add("cpalette",   BBC_CPALETTE);        /* Clwyd Technics Colour Palette */
	//device.option_add("ev1",        BBC_EV1);             /* Micro-Robotics EV1 */
	//device.option_add("hobbit",     BBC_HOBBIT);          /* Hobbit Floppy Tape System (Ikon) */
	device.option_add("lcd",        BBC_LCD);             /* Sprow LCD Display */
	device.option_add("lvlecho",    BBC_LVLECHO);         /* LVL Echo Keyboard */
	device.option_add("m4000",      BBC_M4000);           /* Hybrid Music 4000 Keyboard */
	device.option_add("m512mouse",  BBC_M512MOUSE);       /* Acorn Mouse (provided with Master 512) */
	device.option_add("sdcard",     BBC_SDCARD);          /* SD Card */
	device.option_add("sdcardt",    BBC_SDCARDT);         /* Turbo SD Card */
	device.option_add("tracker",    BBC_TRACKER);         /* Marconi RB2 Tracker Ball / Acorn Tracker Ball */
	device.option_add("usersplit",  BBC_USERSPLIT);       /*User Port Splitter (Watford Electronics) */
	//device.option_add("vci",        BBC_VCI);             /* Video Camera Interface (Data Harvest) */
	device.option_add("voicebox",   BBC_VOICEBOX);        /* Robin Voice Box */
	device.option_add_internal("cfa3000kbd", CFA3000_KBD);/* Henson CFA 3000 Keyboard */
}
