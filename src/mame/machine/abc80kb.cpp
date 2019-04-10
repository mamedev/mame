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

#include "emu.h"
#include "abc80kb.h"

#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "i8048"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC80_KEYBOARD, abc80_keyboard_device, "abc80kb", "ABC-80 Keyboard")


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

const tiny_rom_entry *abc80_keyboard_device::device_rom_region() const
{
	return ROM_NAME( abc80_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( abc80_keyboard_io )
//-------------------------------------------------

void abc80_keyboard_device::abc80_keyboard_io(address_map &map)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc80_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_IO, &abc80_keyboard_device::abc80_keyboard_io);
	m_maincpu->set_disable();
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

abc80_keyboard_device::abc80_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ABC80_KEYBOARD, tag, owner, clock)
	, m_write_keydown(*this)
	, m_maincpu(*this, I8048_TAG)
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

uint8_t abc80_keyboard_device::data_r()
{
	return 0;
}
