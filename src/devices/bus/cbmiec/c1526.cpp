// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1526/MPS-802/4023 Printer emulation

**********************************************************************/

#include "c1526.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6504_TAG "u7d"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C1526 = &device_creator<c1526_t>;
const device_type MPS802 = &device_creator<c1526_t>;
const device_type C4023 = &device_creator<c4023_t>;


//-------------------------------------------------
//  ROM( c1526 )
//-------------------------------------------------

ROM_START( c1526 )
	ROM_REGION( 0x2000, M6504_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "r05", "Revision 5" )
	ROMX_LOAD( "325341-05.u8d", 0x0000, 0x2000, CRC(3ef63c59) SHA1(a71be83a476d2777d33dddb0103c036a047975ba), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r07c", "Revision 7c" )
	ROMX_LOAD( "325341-08.u8d", 0x0000, 0x2000, CRC(38f85b4a) SHA1(25880091979b21fdaf713b53ef2f1cb8063a3505), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "r07b", "Revision 7b (Swe/Fin)" )
	ROMX_LOAD( "cbm 1526 vers. 1.0 skand.gen.u8d", 0x0000, 0x2000, CRC(21051f69) SHA1(7e622fc39985ebe9333d2b546b3c85fd6ab17a53), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "grafik", "MPS802 GrafikROM II v60.12" )
	ROMX_LOAD( "mps802 grafikrom ii v60.12.u8d", 0x0000, 0x2000, CRC(9f5e6b18) SHA1(8b7f620a8f85e250b142d72b812a67fd0e292d68), ROM_BIOS(4) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c1526_t::device_rom_region() const
{
	return ROM_NAME( c1526 );
}


//-------------------------------------------------
//  ROM( c4023 )
//-------------------------------------------------

ROM_START( c4023 )
	ROM_REGION( 0x2000, M6504_TAG, 0 )
	ROM_LOAD( "325360-03.u8d", 0x0000, 0x2000, CRC(c6bb0977) SHA1(7a8c43d2e205f58d83709c04bc7795602a892ddd) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c4023_t::device_rom_region() const
{
	return ROM_NAME( c4023 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1526_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c1526_mem, AS_PROGRAM, 8, c1526_base_t )
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION(M6504_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( c1526 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c1526 )
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_4MHz/4)
	MCFG_CPU_PROGRAM_MAP(c1526_mem)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c1526_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c1526 );
}


//-------------------------------------------------
//  MACHINE_DRIVER( c4023 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c4023 )
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_4MHz/4)
	MCFG_CPU_PROGRAM_MAP(c1526_mem)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c4023_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c4023 );
}


//-------------------------------------------------
//  INPUT_PORTS( c1526 )
//-------------------------------------------------

static INPUT_PORTS_START( c1526 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1526_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1526 );
}


//-------------------------------------------------
//  INPUT_PORTS( c4023 )
//-------------------------------------------------

static INPUT_PORTS_START( c4023 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c4023_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( c4023 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1526_base_t - constructor
//-------------------------------------------------

c1526_base_t:: c1526_base_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
}


//-------------------------------------------------
//  c1526_t - constructor
//-------------------------------------------------

c1526_t::c1526_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	c1526_base_t(mconfig, C1526, "1526", tag, owner, clock, "c1526", __FILE__),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  c4023_t - constructor
//-------------------------------------------------

c4023_t::c4023_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	c1526_base_t(mconfig, C4023, "4023", tag, owner, clock, "c4023", __FILE__),
	device_ieee488_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1526_base_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1526_base_t::device_reset()
{
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void c1526_t::cbm_iec_atn(int state)
{
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void c1526_t::cbm_iec_data(int state)
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void c1526_t::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  ieee488_atn_w -
//-------------------------------------------------

void c4023_t::ieee488_atn(int state)
{
}


//-------------------------------------------------
//  ieee488_ifc_w -
//-------------------------------------------------

void c4023_t::ieee488_ifc(int state)
{
	if (!state)
	{
		device_reset();
	}
}
