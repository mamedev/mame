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

    http://mikenaberezny.com/hardware/c64-128/interpod-ieee-488-interface/

*/

#include "emu.h"
#include "interpod.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define R6502_TAG       "u1"
#define R6532_TAG       "u3"
#define R6522_TAG       "u4"
#define MC6850_TAG      "u5"
#define RS232_TAG       "rs232"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(INTERPOD, interpod_t, "interpod", "Interpod")



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  ROM( interpod )
//-------------------------------------------------

ROM_START( interpod )
	ROM_REGION( 0x800, R6502_TAG, 0 )
	ROM_DEFAULT_BIOS("v16")
	ROM_SYSTEM_BIOS( 0, "v14", "Version 1.4" )
	ROMX_LOAD( "1.4.u2", 0x000, 0x800, CRC(c5b71982) SHA1(614d677b7c6273f6b84fa61affaf91cfdaeed6a6), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v16", "Version 1.6" )
	ROMX_LOAD( "1.6.u2", 0x000, 0x800, CRC(67bb0436) SHA1(7659c45b73f577233f7657c4da9141dcfe8b6d97), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *interpod_t::device_rom_region() const
{
	return ROM_NAME( interpod );
}


//-------------------------------------------------
//  ADDRESS_MAP( interpod_mem )
//-------------------------------------------------

void interpod_t::interpod_mem(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x3b80).m(R6532_TAG, FUNC(mos6532_new_device::ram_map));
	map(0x0400, 0x041f).mirror(0x3be0).m(R6532_TAG, FUNC(mos6532_new_device::io_map));
	map(0x2000, 0x2001).mirror(0x9ffe).rw(MC6850_TAG, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x4000, 0x47ff).mirror(0xb800).rom().region(R6502_TAG, 0);
	map(0x8000, 0x800f).mirror(0x5ff0).rw(R6522_TAG, FUNC(via6522_device::read), FUNC(via6522_device::write));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void interpod_t::device_add_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, 1000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &interpod_t::interpod_mem);

	VIA6522(config, m_via, 1000000);

	MOS6532_NEW(config, m_riot, 1000000);

	ACIA6850(config, m_acia, 0);

	ieee488_device::add_cbm_devices(config, nullptr);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  interpod_t - constructor
//-------------------------------------------------

interpod_t::interpod_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, INTERPOD, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_maincpu(*this, R6502_TAG),
	m_via(*this, R6522_TAG),
	m_riot(*this, R6532_TAG),
	m_acia(*this, MC6850_TAG),
	m_ieee(*this, IEEE488_TAG),
	m_rs232(*this, RS232_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void interpod_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void interpod_t::device_reset()
{
}
