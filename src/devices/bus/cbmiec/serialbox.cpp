// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Serial Box 64K Serial Port Buffer emulation

**********************************************************************/

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

const device_type SERIAL_BOX = &device_creator<serial_box_device>;


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

const rom_entry *serial_box_device::device_rom_region() const
{
	return ROM_NAME( serial_box );
}


//-------------------------------------------------
//  ADDRESS_MAP( serial_box_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( serial_box_mem, AS_PROGRAM, 8, serial_box_device )
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( serial_box )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( serial_box )
	MCFG_CPU_ADD(M6502_TAG, M65C02, XTAL_4MHz/4)
	MCFG_CPU_PROGRAM_MAP(serial_box_mem)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor serial_box_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( serial_box );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  serial_box_device - constructor
//-------------------------------------------------

serial_box_device::serial_box_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SERIAL_BOX, "Serial Box", tag, owner, clock, "serbox", __FILE__),
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
