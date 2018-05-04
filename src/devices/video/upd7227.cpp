// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    uPD7227 Intelligent Dot-Matrix LCD Controller/Driver emulation

**********************************************************************/

#include "emu.h"
#include "upd7227.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(UPD7227, upd7227_device, "upd7227", "NEC uPD7227")


void upd7227_device::upd7227_map(address_map &map)
{
	map(0x00, 0x27).ram();
	map(0x40, 0x67).ram();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd7227_device - constructor
//-------------------------------------------------

upd7227_device::upd7227_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UPD7227, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("videoram", ENDIANNESS_BIG, 8, 7, 0, address_map_constructor(FUNC(upd7227_device::upd7227_map), this))
	, m_cs(1)
	, m_cd(1)
	, m_sck(1)
	, m_si(1)
	, m_so(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7227_device::device_start()
{
	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_cd));
	save_item(NAME(m_sck));
	save_item(NAME(m_si));
	save_item(NAME(m_so));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd7227_device::device_reset()
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector upd7227_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

uint32_t upd7227_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


//-------------------------------------------------
//  cs_w - chip select
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7227_device::cs_w )
{
	m_cs = state;
}


//-------------------------------------------------
//  cd_w - command/data select
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7227_device::cd_w )
{
	m_cd = state;
}


//-------------------------------------------------
//  sck_w - serial clock
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7227_device::sck_w )
{
	m_sck = state;
}


//-------------------------------------------------
//  si_w - serial input
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7227_device::si_w )
{
	m_si = state;
}


//-------------------------------------------------
//  so_r - serial output/busy
//-------------------------------------------------

READ_LINE_MEMBER( upd7227_device::so_r )
{
	return m_so;
}
