// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    DataBoard 4106 Minifloppy interface emulation

*********************************************************************/

#include "emu.h"
#include "db4106.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG "maincpu"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DATABOARD_4106, databoard_4106_device, "abc_db4106", "DataBoard 4106 Minifloppy interface")


//-------------------------------------------------
//  ROM( databoard_4106 )
//-------------------------------------------------

ROM_START( databoard_4106 )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_LOAD("databoard-4106-a5.12-floppy-cont.bin", 0x000, 0x800, CRC(708f2a2f) SHA1(3f99ea2ef301d9e3415db152dad79171dc62cc2d) ) // single track controller
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *databoard_4106_device::device_rom_region() const
{
	return ROM_NAME( databoard_4106 );
}


//-------------------------------------------------
//  ADDRESS_MAP( databoard_4106_mem )
//-------------------------------------------------

void databoard_4106_device::databoard_4106_mem(address_map &map)
{
	map(0x0000, 0x07ff).rom().region(Z80_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( databoard_4106_io )
//-------------------------------------------------

void databoard_4106_device::databoard_4106_io(address_map &map)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void databoard_4106_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_memory_map(&databoard_4106_device::databoard_4106_mem);
	m_maincpu->set_io_map(&databoard_4106_device::databoard_4106_io);
}


//-------------------------------------------------
//  INPUT_PORTS( databoard_4106 )
//-------------------------------------------------

INPUT_PORTS_START( databoard_4106 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor databoard_4106_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( databoard_4106 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  databoard_4106_device - constructor
//-------------------------------------------------

databoard_4106_device::databoard_4106_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DATABOARD_4106, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void databoard_4106_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void databoard_4106_device::device_reset()
{
	m_cs = false;
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void databoard_4106_device::abcbus_cs(uint8_t data)
{
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

uint8_t databoard_4106_device::abcbus_stat()
{
	uint8_t data = 0xff;

	if (m_cs)
	{
	}

	return data;
}


//-------------------------------------------------
//  abcbus_inp -
//-------------------------------------------------

uint8_t databoard_4106_device::abcbus_inp()
{
	uint8_t data = 0xff;

	if (m_cs)
	{
	}

	return data;
}


//-------------------------------------------------
//  abcbus_out -
//-------------------------------------------------

void databoard_4106_device::abcbus_out(uint8_t data)
{
	if (!m_cs) return;
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void databoard_4106_device::abcbus_c1(uint8_t data)
{
	if (m_cs)
	{
	}
}


//-------------------------------------------------
//  abcbus_c3 -
//-------------------------------------------------

void databoard_4106_device::abcbus_c3(uint8_t data)
{
	if (m_cs)
	{
		m_maincpu->reset();
	}
}
