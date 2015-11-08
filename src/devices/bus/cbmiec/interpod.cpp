// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Oxford Computer Systems Interpod IEC to IEEE interface emulation

*********************************************************************/

/*

PCB Layout
----------

INTERPOD 1000 ISS.3

|---------------------------------------|
|                                       |
|       ACIA        ROM         CPU     |
|                                       |
|CN1    75188   LS73    LS04    RIOT    |
|       75189                           |
|                       7417    VIA     |
|CN2                                    |
|                   3446  3446  3446    |
|       CN3 CN4         LD1             |
|---------------------------|   CN5   |-|
                            |---------|

Notes:
    All IC's shown.

    ROM     - 2716 "1.4"
    CPU     - Rockwell R6502P
    RIOT    - Rockwell R6532AP
    VIA     - Rockwell R6522P
    ACIA    - Thomson-CSF EF6850P
    3446    - Motorola MC3446AP
    CN1     - DB25 serial connector
    CN2     - power connector
    CN3     - DIN5 IEC connector
    CN4     - DIN5 IEC connector
    CN5     - 2x12 PCB edge IEEE-488 connector
    LD1     - LED

*/

/*

    TODO:

    - everything

        0 OPEN 2,4,31 : INPUT #2, A$ : CLOSE #2
        1 PRINT A$

    http://mikenaberezny.com/hardware/projects/interpod-ieee-488-interface/

*/

#include "interpod.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define R6502_TAG       "u1"
#define R6532_TAG       "u3"
#define R6522_TAG       "u4"
#define MC6850_TAG      "u5"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type INTERPOD = &device_creator<interpod_device>;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  ROM( interpod )
//-------------------------------------------------

ROM_START( interpod )
	ROM_REGION( 0x800, R6502_TAG, 0 )
	ROM_LOAD( "1.4.u2", 0x000, 0x800, CRC(c5b71982) SHA1(614d677b7c6273f6b84fa61affaf91cfdaeed6a6) )
	ROM_LOAD( "1.6.u2", 0x000, 0x800, CRC(67bb0436) SHA1(7659c45b73f577233f7657c4da9141dcfe8b6d97) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *interpod_device::device_rom_region() const
{
	return ROM_NAME( interpod );
}


//-------------------------------------------------
//  ADDRESS_MAP( interpod_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( interpod_mem, AS_PROGRAM, 8, interpod_device )
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x3b80) AM_DEVICE(R6532_TAG, mos6532_t, ram_map)
	AM_RANGE(0x0400, 0x041f) AM_MIRROR(0x3be0) AM_DEVICE(R6532_TAG, mos6532_t, io_map)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x9ffe) AM_DEVREADWRITE(MC6850_TAG, acia6850_device, status_r, control_w)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x9ffe) AM_DEVREADWRITE(MC6850_TAG, acia6850_device, data_r, data_w)
	AM_RANGE(0x4000, 0x47ff) AM_MIRROR(0xb800) AM_ROM AM_REGION(R6502_TAG, 0)
	AM_RANGE(0x8000, 0x800f) AM_MIRROR(0x5ff0) AM_DEVREADWRITE(R6522_TAG, via6522_device, read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( interpod )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( interpod )
	MCFG_CPU_ADD(R6502_TAG, M6502, 1000000)
	MCFG_CPU_PROGRAM_MAP(interpod_mem)

	MCFG_DEVICE_ADD(R6522_TAG, VIA6522, 1000000)
	MCFG_DEVICE_ADD(R6532_TAG, MOS6532n, 1000000)
	MCFG_DEVICE_ADD(MC6850_TAG, ACIA6850, 0)

	MCFG_CBM_IEEE488_ADD(NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor interpod_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( interpod );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  interpod_device - constructor
//-------------------------------------------------

interpod_device::interpod_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, INTERPOD, "Interpod", tag, owner, clock, "interpod", __FILE__),
		device_cbm_iec_interface(mconfig, *this),
		m_maincpu(*this, R6502_TAG),
		m_via(*this, R6522_TAG),
		m_riot(*this, R6532_TAG),
		m_acia(*this, MC6850_TAG),
		m_ieee(*this, IEEE488_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void interpod_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void interpod_device::device_reset()
{
}
