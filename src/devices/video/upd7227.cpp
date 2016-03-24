// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    uPD7227 Intelligent Dot-Matrix LCD Controller/Driver emulation

**********************************************************************/

#include "emu.h"
#include "upd7227.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type UPD7227 = &device_creator<upd7227_device>;


static ADDRESS_MAP_START( upd7227_map, AS_PROGRAM, 8, upd7227_device )
	AM_RANGE(0x00, 0x27) AM_RAM
	AM_RANGE(0x40, 0x67) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd7227_device - constructor
//-------------------------------------------------

upd7227_device::upd7227_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD7227, "uPD7227", tag, owner, clock, "upd7227", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("videoram", ENDIANNESS_BIG, 8, 7, 0, *ADDRESS_MAP_NAME(upd7227_map)),
		m_cs(1),
		m_cd(1),
		m_sck(1),
		m_si(1),
		m_so(1)
{
}


//-------------------------------------------------
//  static_set_offsets - configuration helper
//-------------------------------------------------

void upd7227_device::static_set_offsets(device_t &device, int sx, int sy)
{
	upd7227_device &upd7227 = downcast<upd7227_device &>(device);

	upd7227.m_sx = sx;
	upd7227.m_sy = sy;
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

const address_space_config *upd7227_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : nullptr;
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

UINT32 upd7227_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
