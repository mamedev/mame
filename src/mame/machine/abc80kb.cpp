// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-80 keyboard emulation

**********************************************************************/

/*

PCB Layout
----------

KTC 65-01870-001 (dated 1978)
PCB-002B

    |---------------|
|---|      CN1      |---------------------------------------------------|
|                                                                       |
|   LS06    LS04    LS04                        LS06    LS00    LS123   |
|   LS74                    PROM        MCU             4051    900C    |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|-----------------------------------------------------------------------|

Notes:
    All IC's shown.

    MCU         - General Instruments 30293B-013 20-04592-013 (?)
    PROM        - Synertek N82S141N 512x8 bipolar PROM "053"
    900C        - Ferranti Interdesign 900C custom (?)
    CN1         - keyboard data connector

*/

/*

PCB Layout
----------

KTC A65-01870-001 (dated 1983)
PCB-201C

    |---------------|
|---|      CN1      |---------------------------------------------------|
|                                                                       |
|   LS06    LS04    LS04                                LS00    LS123   |
|                                       MCU             4051    900C    |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|-----------------------------------------------------------------------|

Notes:
    All IC's shown.

    MCU         - General Instruments 30293B-047 20-04592-047 (?)
    900C        - Ferranti Interdesign 900C custom (?)
    CN1         - keyboard data connector

*/

#include "abc80kb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "i8048"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC80_KEYBOARD = &device_creator<abc80_keyboard_device>;


//-------------------------------------------------
//  ROM( abc80_keyboard )
//-------------------------------------------------

ROM_START( abc80_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	ROM_LOAD( "053.z5", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc80_keyboard_device::device_rom_region() const
{
	return ROM_NAME( abc80_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( abc80_keyboard_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc80_keyboard_io, AS_IO, 8, abc80_keyboard_device )
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( abc80_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc80_keyboard )
	MCFG_CPU_ADD(I8048_TAG, I8048, 4000000)
	MCFG_CPU_IO_MAP(abc80_keyboard_io)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc80_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc80_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( abc80_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( abc80_keyboard )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor abc80_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc80_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc80_keyboard_device - constructor
//-------------------------------------------------

abc80_keyboard_device::abc80_keyboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ABC80_KEYBOARD, "ABC-80 Keyboard", tag, owner, clock, "abc80kb", __FILE__),
		m_write_keydown(*this),
		m_maincpu(*this, I8048_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc80_keyboard_device::device_start()
{
	// resolve callbacks
	m_write_keydown.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc80_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  data_r - keyboard data read
//-------------------------------------------------

UINT8 abc80_keyboard_device::data_r()
{
	return 0;
}
