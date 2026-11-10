// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-1000 Dot Matrix Printer emulation

**********************************************************************/

#include "emu.h"
#include "mps1000.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS1000, mps1000_device, "mps1000", "Commodore MPS-1000 Dot Matrix Printer")


//-------------------------------------------------
//  ROM( mps1000 )
//-------------------------------------------------

ROM_START( mps1000 )
	ROM_REGION( 0x8000, "firmware", 0 )
	ROM_LOAD( "mps1000-e2-ce8.bin", 0x0000, 0x8000, CRC(412bd4e2) SHA1(af88d10077d990fa1edbc6cdbd059af64738b8d4) )

	ROM_REGION( 0x4000, "hwh", 0 )
	// ver 2.20, 06/DEC/1986 (But it could also perhaps mean 12/JUN/1986)
	// I can't tell the PCB reference because this was dumped from spare EPROMs from a drawer
	// The dump seems to be good because the data content of all of my 4 spare EPROMs matched perfectly.
	ROM_LOAD( "mps_1000_vers_2.20_12-06-86.rom", 0x0000, 0x2000, CRC(0a91ea8a) SHA1(ccb679f3d1f7f4eddb4e8899fe9e9a594dcfca5d) )
	// this is another Z80-based bios.
	ROM_LOAD( "hwh_28.06.88_ep12-2_s80r2.rom", 0x2000, 0x2000, CRC(1d17c586) SHA1(1afd1d1bbebe4d79c3cfcc30ac617b1a8ddb99e1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps1000_device::device_rom_region() const
{
	return ROM_NAME( mps1000 );
}


//-------------------------------------------------
//  ADDRESS_MAP( mps1000_mem )
//-------------------------------------------------

void mps1000_device::mps1000_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("firmware", 0);
	map(0xa000, 0xa7ff).ram();
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mps1000_device::device_add_mconfig(machine_config &config)
{
	UPD7810(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mps1000_device::mps1000_mem);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps1000_device - constructor
//-------------------------------------------------

mps1000_device::mps1000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MPS1000, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_maincpu(*this, "maincpu")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1000_device::device_start()
{
}
