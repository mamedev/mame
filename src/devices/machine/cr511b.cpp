// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    CR-511-B CD-ROM drive

    CD-ROM drive with a custom MKE/Panasonic interface as used in the
    Commodore CDTV and early SoundBlaster cards.

***************************************************************************/

#include "emu.h"
#include "cr511b.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CR511B, cr511b_device, "cr511b", "CR-511-B CD-ROM drive")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cr511b_device::device_add_mconfig(machine_config &config)
{
	CDROM(config, m_cdrom).set_interface("cdrom");
	CDDA(config, m_cdda);
	m_cdda->add_route(0, ":lspeaker", 1.0);
	m_cdda->add_route(1, ":rspeaker", 1.0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cr511b_device - constructor
//-------------------------------------------------

cr511b_device::cr511b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CR511B, tag, owner, clock),
	m_cdrom(*this, "cdrom"),
	m_cdda(*this, "cdda"),
	m_stch_handler(*this),
	m_sten_handler(*this),
	m_drq_handler(*this),
	m_dten_handler(*this),
	m_scor_handler(*this),
	m_xaen_handler(*this),
	//m_motor(false),
	m_enabled(-1),
	m_cmd(-1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cr511b_device::device_start()
{
	// resolve callbacks
	m_stch_handler.resolve_safe();
	m_sten_handler.resolve_safe();
	m_drq_handler.resolve_safe();
	m_dten_handler.resolve_safe();
	m_scor_handler.resolve_safe();
	m_xaen_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cr511b_device::device_reset()
{
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t cr511b_device::read()
{
	return 0xff;
}

void cr511b_device::write(uint8_t data)
{
}

WRITE_LINE_MEMBER( cr511b_device::enable_w )
{
	m_enabled = state;
}

WRITE_LINE_MEMBER( cr511b_device::cmd_w )
{
	m_cmd = state;
}
