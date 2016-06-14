// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	Sony LDP-1000 laserdisc emulation.

	TODO:
	- Dump BIOSes (seven of them according to docs)
	- Hookup with Sony SMC-70 / SMC-777

***************************************************************************/

#include "emu.h"
#include "machine/ldp1000.h"


ROM_START( ldp1000 )
	ROM_REGION( 0x2000, "ldp1000", 0 )
	ROM_LOAD( "ldp1000_bios.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SONY_LDP1000 = &device_creator<sony_ldp1000_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ldp1000_device - constructor
//-------------------------------------------------

sony_ldp1000_device::sony_ldp1000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: laserdisc_device(mconfig, SONY_LDP1000, "Sony LDP-1000", tag, owner, clock, "ldp1000", __FILE__)
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void sony_ldp1000_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sony_ldp1000_device::device_start()
{
	laserdisc_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sony_ldp1000_device::device_reset()
{
	laserdisc_device::device_reset();

}

//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const rom_entry *sony_ldp1000_device::device_rom_region() const
{
	return ROM_NAME(ldp1000);
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void sony_ldp1000_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	//printf("%d vsync\n",fieldnum);
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

INT32 sony_ldp1000_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	//printf("%d update\n",fieldnum);

	return fieldnum;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( sony_ldp1000_device::read )
{
	return 0;
}

WRITE8_MEMBER( sony_ldp1000_device::write )
{
}
