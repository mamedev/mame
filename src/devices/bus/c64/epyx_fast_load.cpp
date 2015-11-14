// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Epyx Fast Load cartridge emulation

**********************************************************************/

#include "epyx_fast_load.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

// t = R*C = 3.3K * 0.47uF * 40% Vtr = 792.2905424610516 usec
// (3.3K pull-up on the EXROM line inside the C64, PLA Vih min = 2.0V = 40% of 5.0V)
#define TIMER_PERIOD    792



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_EPYX_FAST_LOAD = &device_creator<c64_epyx_fast_load_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_epyx_fast_load_cartridge_device - constructor
//-------------------------------------------------

c64_epyx_fast_load_cartridge_device::c64_epyx_fast_load_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_EPYX_FAST_LOAD, "C64 Epyx Fast Load cartridge", tag, owner, clock, "c64_epyx_fast_load", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this), m_exrom_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_epyx_fast_load_cartridge_device::device_start()
{
	// allocate timer
	m_exrom_timer = timer_alloc();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_epyx_fast_load_cartridge_device::device_reset()
{
	m_exrom = 0;
	m_exrom_timer->adjust(attotime::from_usec(TIMER_PERIOD), 0);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void c64_epyx_fast_load_cartridge_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_exrom = 1;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_epyx_fast_load_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		m_exrom = 0;
		m_exrom_timer->adjust(attotime::from_usec(TIMER_PERIOD), 0);

		data = m_roml[offset & 0x1fff];
	}
	else if (!io1)
	{
		m_exrom = 0;
		m_exrom_timer->adjust(attotime::from_usec(TIMER_PERIOD), 0);
	}
	else if (!io2)
	{
		data = m_roml[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_epyx_fast_load_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_exrom = 0;
		m_exrom_timer->adjust(attotime::from_usec(TIMER_PERIOD), 0);
	}
}
