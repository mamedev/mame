// license:BSD-3-Clause
// copyright-holders:Ramiro Polla

#include "emu.h"
#include "nec_p72.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NEC_P72, nec_p72_device, "p72", "NEC PinWriter P72")


//-------------------------------------------------
//  ROM( p72 )
//-------------------------------------------------

ROM_START( p72 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("p72.2c", 0x000000, 0x100000, CRC(2bc6846f) SHA1(10430f1d3b73ad413d77053515d9d53831a1341b))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nec_p72_device::device_rom_region() const
{
	return ROM_NAME( p72 );
}


//-------------------------------------------------
//  ADDRESS_MAP( p72_mem )
//-------------------------------------------------

void nec_p72_device::p72_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom(); /* 1Mbyte firmware */
	//AM_RANGE(0x100000, 0x1fffff) AM_RAM /* 1Mbyte external RAM */ /* TODO might be 2x1Mbit */
	// [RH] 29 August 2016: Commented out because the NEC V33 only has 20 address lines, and
	// the V40 has more, but we don't have an NEC V40 implemented yet.
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nec_p72_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	V33(config, m_maincpu, XTAL(16'000'000)/2); /* TODO it's actually a V40 */
	m_maincpu->set_addrmap(AS_PROGRAM, &nec_p72_device::p72_mem);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nec_p72_device::device_start()
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nec_p72_device - constructor
//-------------------------------------------------

nec_p72_device::nec_p72_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nec_p72_device(mconfig, NEC_P72, tag, owner, clock)
{
}

nec_p72_device::nec_p72_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu")
{
}
