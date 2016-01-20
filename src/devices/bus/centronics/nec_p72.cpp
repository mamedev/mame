// license:BSD-3-Clause
// copyright-holders:Ramiro Polla

#include "nec_p72.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NEC_P72 = &device_creator<nec_p72_t>;


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

const rom_entry *nec_p72_t::device_rom_region() const
{
	return ROM_NAME( p72 );
}


//-------------------------------------------------
//  ADDRESS_MAP( p72_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( p72_mem, AS_PROGRAM, 8, nec_p72_t )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM /* 1Mbit firmware */
	AM_RANGE(0x100000, 0x1fffff) AM_RAM /* 1Mbit external RAM */ /* TODO might be 2x1Mbit */
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( nec_p72 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( nec_p72 )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V33, XTAL_16MHz/2) /* TODO it's actually a V40 */
	MCFG_CPU_PROGRAM_MAP(p72_mem)

MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nec_p72_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( nec_p72 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nec_p72_t::device_start()
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nec_p72_t - constructor
//-------------------------------------------------

nec_p72_t::nec_p72_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, NEC_P72, "NEC PinWrite P72", tag, owner, clock, "p72", __FILE__),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu")
{
}

nec_p72_t::nec_p72_t(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu")
{
}
