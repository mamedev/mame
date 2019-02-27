// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Serial Box 64K Serial Port Buffer emulation

**********************************************************************/

#include "emu.h"
#include "serialbox.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "u1"
#define M8520_TAG       "u5"
#define WD1770_TAG      "u4"


enum
{
	LED_POWER = 0,
	LED_ACT
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SERIAL_BOX, serial_box_device, "serbox", "Serial Box")


//-------------------------------------------------
//  ROM( serial_box )
//-------------------------------------------------

ROM_START( serial_box )
	ROM_REGION( 0x1000, M6502_TAG, 0 )
	ROM_LOAD( "serialbx.bin", 0x0000, 0x1000, CRC(d0e0218c) SHA1(9b922f1e9f9b71e771361c52d4df2aa5695488a5) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *serial_box_device::device_rom_region() const
{
	return ROM_NAME( serial_box );
}


//-------------------------------------------------
//  ADDRESS_MAP( serial_box_mem )
//-------------------------------------------------

void serial_box_device::serial_box_mem(address_map &map)
{
	map(0xf000, 0xffff).rom().region(M6502_TAG, 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void serial_box_device::device_add_mconfig(machine_config &config)
{
	M65C02(config, m_maincpu, XTAL(4'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &serial_box_device::serial_box_mem);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  serial_box_device - constructor
//-------------------------------------------------

serial_box_device::serial_box_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERIAL_BOX, tag, owner, clock),
		device_cbm_iec_interface(mconfig, *this),
		m_maincpu(*this, M6502_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void serial_box_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void serial_box_device::device_reset()
{
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void serial_box_device::cbm_iec_atn(int state)
{
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void serial_box_device::cbm_iec_data(int state)
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void serial_box_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}
