// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    DataBoard 4112-23 floppy disk controller emulation

*********************************************************************/

#include "emu.h"
#include "db4112.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG "maincpu"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DATABOARD_4112, abc_databoard_4112_device, "abc_db4112", "DataBoard 4112")


//-------------------------------------------------
//  ROM( databoard_4112 )
//-------------------------------------------------

ROM_START( databoard_4112 )
	ROM_DEFAULT_BIOS( "fd55" )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "119", "CONT 4112 Ver 1.19 85-07-04" )
	ROMX_LOAD("fpy_int_4112-23_4112_ver1.19.bin", 0x0000, 0x2000, CRC(74c355e5) SHA1(cc434af31897cf326cb2bb2a630f74a8d8174c3c), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "fd55", "TEAC FD55 (860702)" )
	ROMX_LOAD("fpy_int_4112-23_teac_fd55.bin", 0x0000, 0x2000, CRC(9175ceb8) SHA1(95c150d3152df318abd9267915d5669d2ec33895), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc_databoard_4112_device::device_rom_region() const
{
	return ROM_NAME( databoard_4112 );
}


//-------------------------------------------------
//  ADDRESS_MAP( databoard_4112_mem )
//-------------------------------------------------

void abc_databoard_4112_device::databoard_4112_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region(Z80_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( databoard_4112_io )
//-------------------------------------------------

void abc_databoard_4112_device::databoard_4112_io(address_map &map)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc_databoard_4112_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_memory_map(&abc_databoard_4112_device::databoard_4112_mem);
	m_maincpu->set_io_map(&abc_databoard_4112_device::databoard_4112_io);
}


//-------------------------------------------------
//  INPUT_PORTS( databoard_4112 )
//-------------------------------------------------

INPUT_PORTS_START( databoard_4112 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor abc_databoard_4112_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( databoard_4112 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_databoard_4112_device - constructor
//-------------------------------------------------

abc_databoard_4112_device::abc_databoard_4112_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DATABOARD_4112, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_databoard_4112_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_databoard_4112_device::device_reset()
{
	m_cs = false;
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void abc_databoard_4112_device::abcbus_cs(uint8_t data)
{
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

uint8_t abc_databoard_4112_device::abcbus_stat()
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

uint8_t abc_databoard_4112_device::abcbus_inp()
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

void abc_databoard_4112_device::abcbus_out(uint8_t data)
{
	if (!m_cs) return;
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void abc_databoard_4112_device::abcbus_c1(uint8_t data)
{
	if (m_cs)
	{
	}
}


//-------------------------------------------------
//  abcbus_c3 -
//-------------------------------------------------

void abc_databoard_4112_device::abcbus_c3(uint8_t data)
{
	if (m_cs)
	{
		m_maincpu->reset();
	}
}
