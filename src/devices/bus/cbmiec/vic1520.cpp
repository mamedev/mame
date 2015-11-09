// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1520 Plotter emulation

**********************************************************************/

/*
PA0 ATN
PA1 _CLK
PA5 ATTN ACK
PA6 NRFD
PA7 _DATA IN

PB0 IEEE SELECT
PB1 IEEE SELECT
PB2 IEEE SELECT
PB4 LED
PB5 REMOVE
PB6 CHANGE
PB7 FEED

PC0 _DN
PC1 _UP
PC7 COLOR SENSOR SW

PD0 X MOTOR COM A
PD1 X MOTOR COM B
PD2 X MOTOR COM C
PD3 X MOTOR COM D
PD4 Y MOTOR COM A
PD5 Y MOTOR COM B
PD6 Y MOTOR COM C
PD7 Y MOTOR COM D
*/

#include "vic1520.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6500_1_TAG "u1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC1520 = &device_creator<vic1520_t>;


//-------------------------------------------------
//  ROM( vic1520 )
//-------------------------------------------------

ROM_START( vic1520 )
	ROM_REGION( 0x800, M6500_1_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "r01", "325340-01" )
	ROMX_LOAD( "325340-01.u1", 0x000, 0x800, CRC(3757da6f) SHA1(8ab43603f74b0f269bbe890d1939a9ae31307eb1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r03", "325340-03" )
	ROMX_LOAD( "325340-03.u1", 0x000, 0x800, CRC(f72ea2b6) SHA1(74c15b2cc1f7632bffa37439609cbdb50b82ea92), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *vic1520_t::device_rom_region() const
{
	return ROM_NAME( vic1520 );
}


//-------------------------------------------------
//  ADDRESS_MAP( vic1520_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( vic1520_mem, AS_PROGRAM, 8, vic1520_t )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x000, 0x03f) AM_RAM
	AM_RANGE(0x800, 0xfff) AM_ROM AM_REGION(M6500_1_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( vic1520 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( vic1520 )
	MCFG_CPU_ADD(M6500_1_TAG, M6502, XTAL_2MHz) // M6500/1
	MCFG_CPU_PROGRAM_MAP(vic1520_mem)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor vic1520_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vic1520 );
}


//-------------------------------------------------
//  INPUT_PORTS( vic1520 )
//-------------------------------------------------

static INPUT_PORTS_START( vic1520 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vic1520_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( vic1520 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1520_t - constructor
//-------------------------------------------------

vic1520_t::vic1520_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VIC1520, "VIC-1520", tag, owner, clock, "vic1520", __FILE__),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1520_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic1520_t::device_reset()
{
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void vic1520_t::cbm_iec_atn(int state)
{
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void vic1520_t::cbm_iec_data(int state)
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void vic1520_t::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}
