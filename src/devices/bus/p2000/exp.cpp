// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P2000_EXPANSION_SLOT, p2000_expansion_slot_device, "p2000_expansion_slot", "P2000 expansion slot")


//**************************************************************************
//  DEVICE P2000 EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_p2000_expansion_slot_card_interface - constructor
//-------------------------------------------------

device_p2000_expansion_slot_card_interface::device_p2000_expansion_slot_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "p2000exp")
{
	m_slot = dynamic_cast<p2000_expansion_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  p2000_expansion_slot_device - constructor
//-------------------------------------------------

p2000_expansion_slot_device::p2000_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, P2000_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_p2000_expansion_slot_card_interface>(mconfig, *this),
	m_io_space(*this, finder_base::DUMMY_TAG, -1),
	m_write_irq(*this), 
    m_in_mode80(*this), 
    m_card(nullptr)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void p2000_expansion_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_write_irq.resolve_safe();
    m_in_mode80.resolve_safe(0);
}

//-------------------------------------------------
//  dew_r - vblank trigger data write
//-------------------------------------------------

uint8_t p2000_expansion_slot_device::dew_r()
{
    uint8_t data = 0;
    if (m_card != nullptr)
	{
        data = m_card->dew_r();
    }
    return data;
}

uint8_t p2000_expansion_slot_device::vidon_r()
{
    uint8_t data = 1;
    if (m_card != nullptr)
	{
        data = m_card->vidon_r();
    }
    return data;
}

uint32_t p2000_expansion_slot_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
    uint8_t data = 0;
    if (m_card != nullptr)
	{
        data = m_card->screen_update(screen, bitmap, cliprect);
    }
    return data;
}

// slot devices
#include "centronics.h"
#include "hires.h"
#include "m2009.h"
#include "m2200.h"
#include "mouse.h"
#include "serial.h"
#include "uniface.h"

//-------------------------------------------------
//  SLOT_INTERFACE( p2000_slot2_devices )
//-------------------------------------------------

void p2000_slot2_devices(device_slot_interface &device)
{
    device.option_add("uniface",    P2000_UNIFACE);      /* UNIFACE - Universal I/O interface */
    device.option_add("centronics", P2000_CENTRONICS);   /* MW102 - Centronics printer interface */
    device.option_add("m2003",      P2000_M2003);        /* Miniware M2003 Centronics printer interface */
    device.option_add("p2ggcent",   P2000_P2GGCENT);     /* P2000gg Centronics printer interface */
    device.option_add("m2009",      P2000_M2009);        /* Miniware M2009 Autodial/Answer 1200-75b/300b modem */
    device.option_add("p2174",      P2000_P2174V24);     /* P2174 - Philips V.24/RS-232 Interface */
    device.option_add("v24",        P2000_PTCV24);       /* PTC V.24 Serial Interface */
    device.option_add("viewdata",   P2000_VIEWDATA);     /* P2171-1 Viewdadata V.24 Serial Interface Cartridge */
    device.option_add("m2001",      P2000_M2001V24);     /* Minware M2001 V.24 Serial Interface */
    device.option_add("mouse",      P2000_MOUSE);        /* MSX Mouse interface module */
 //	  device.option_add("IEEE",       P2000_IEEE);         /* IEEE interface -- specs/design needed */
 //   device.option_add("iecbus",     P2000_IECBUS;        /* P2373 - IEEC-BUS Interface --specs/design needed */
 //   device.option_add("midi",       P2000_MIDI;          /* Midi interface cartridge -- specs/design needed */
}


//-------------------------------------------------
//  SLOT_INTERFACE( p2000_ext1_devices )
//-------------------------------------------------

void p2000_ext1_devices(device_slot_interface &device)
{
    device.option_add("fdc",      P2000_FDC);            /* Floppy Disc Controller M/T-model */
    device.option_add("m2200",    P2000_M2200);          /* Miniware M2200 Multi Purpose Floppy Disc Controller (256KB RAM DISK) */
    device.option_add("m2200d",   P2000_M2200D);         /* Miniware M2200 Multi Purpose Floppy Disc Controller (64KB RAM DISK) */
}

//-------------------------------------------------
//  SLOT_INTERFACE( p2000_ext2_devices )
//-------------------------------------------------

void p2000_ext2_devices(device_slot_interface &device)
{
	device.option_add("hires", P2000_HIRES);
//  device.option_add("cpm",       P2000_CPM);         /* CPM interface -- specs/design needed */
}


