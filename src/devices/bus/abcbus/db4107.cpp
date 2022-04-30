// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    DataBoard 4107 Winchester interface emulation

*********************************************************************/

#include "emu.h"
#include "db4107.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG     "maincpu"
#define Z80DMA_TAG  "dma"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DATABOARD_4107, databoard_4107_device, "abc_db4107", "DataBoard 4107 Winchester interface")


//-------------------------------------------------
//  ROM( databoard_4107 )
//-------------------------------------------------

ROM_START( databoard_4107 )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "w511", "Micropolis W511" )
	ROMX_LOAD("micr.w511_6490398-2.bin", 0x000, 0x1000, CRC(cd8f16ff) SHA1(52966142bbed07459f6eb272213b677ec49864d3), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "nec5126", "NEC 5126)" )
	ROMX_LOAD("nec_1.2_6490396-1.bin", 0x000, 0x1000, CRC(88fa6ed2) SHA1(9faa144591c620c5d7dcc15feada350f6a14f02a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "nec6490", "NEC 6490" )
	ROMX_LOAD("nec_6490.bin", 0x000, 0x1000, CRC(17c247e7) SHA1(7339738b87751655cb4d6414422593272fe72f5d), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "ro202", "Rodime RO-202" )
	ROMX_LOAD("ro202_1.00_6490320-01.bin", 0x000, 0x800, CRC(337b4dcf) SHA1(791ebeb4521ddc11fb9742114018e161e1849bdf), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "st4038", "Seagate ST4038 (CHS: 733,5,17,512)" )
	ROMX_LOAD("st4038.bin", 0x000, 0x800, CRC(4c803b87) SHA1(1141bb51ad9200fc32d92a749460843dc6af8953), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "st406", "Seagate ST406" )
	ROMX_LOAD("st406.bin", 0x000, 0x800, CRC(3cdd154f) SHA1(b1464163370e9037a8ce1d535935ee55fbf68ee1), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "st514", "Seagate ST514" )
	ROMX_LOAD("st514_15_5.bin", 0x000, 0x800, CRC(6e25c645) SHA1(8b3ca02decef5aafdb46dbfdf84371057d06799e), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "w504", "W504" )
	ROMX_LOAD("w504_10m.bin", 0x000, 0x800, CRC(277ee4a5) SHA1(a3bf6f2b9f75ef46fd3b1aca6e4a42191d702b82), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "st412", "Seagate ST412" )
	ROMX_LOAD("w507_st412.bin", 0x000, 0x800, CRC(0d3c92d2) SHA1(345c5f8154f0ec9d66ef19ce9793176aa257707d), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS( 9, "w514", "W514" )
	ROMX_LOAD("w514.bin", 0x000, 0x800, CRC(8ec3f70e) SHA1(ca60a48d591e42526fcc301ce94e861bf8368aa4), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS( 10, "b6185b", "BASF 6185B" )
	ROMX_LOAD("basf_6185b.bin", 0x000, 0x800, CRC(06f8fe2e) SHA1(e81f2a47c854e0dbb096bee3428d79e63591059d), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS( 11, "m1325", "Micropolis 1325" )
	ROMX_LOAD("micr1325.bin", 0x000, 0x800, CRC(084af409) SHA1(342b8e214a8c4c2b014604e53c45ef1bd1c69ea3), ROM_BIOS(11) )
	ROM_SYSTEM_BIOS( 12, "st225", "W515 (CHS: 615,4,17,512)" )
	ROMX_LOAD("w515_st225.bin", 0x000, 0x800, CRC(c9f68f81) SHA1(7ff8b2a19f71fe0279ab3e5a0a5fffcb6030360c), ROM_BIOS(12) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *databoard_4107_device::device_rom_region() const
{
	return ROM_NAME( databoard_4107 );
}


//-------------------------------------------------
//  ADDRESS_MAP( databoard_4107_mem )
//-------------------------------------------------

void databoard_4107_device::databoard_4107_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region(Z80_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( databoard_4107_io )
//-------------------------------------------------

void databoard_4107_device::databoard_4107_io(address_map &map)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void databoard_4107_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_memory_map(&databoard_4107_device::databoard_4107_mem);
	m_maincpu->set_io_map(&databoard_4107_device::databoard_4107_io);

	Z80DMA(config, m_dma, 4000000);
}


//-------------------------------------------------
//  INPUT_PORTS( databoard_4107 )
//-------------------------------------------------

INPUT_PORTS_START( databoard_4107 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor databoard_4107_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( databoard_4107 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  databoard_4107_device - constructor
//-------------------------------------------------

databoard_4107_device::databoard_4107_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DATABOARD_4107, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG),
	m_dma(*this, Z80DMA_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void databoard_4107_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void databoard_4107_device::device_reset()
{
	m_cs = false;
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void databoard_4107_device::abcbus_cs(uint8_t data)
{
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

uint8_t databoard_4107_device::abcbus_stat()
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

uint8_t databoard_4107_device::abcbus_inp()
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

void databoard_4107_device::abcbus_out(uint8_t data)
{
	if (!m_cs) return;
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void databoard_4107_device::abcbus_c1(uint8_t data)
{
	if (m_cs)
	{
	}
}


//-------------------------------------------------
//  abcbus_c3 -
//-------------------------------------------------

void databoard_4107_device::abcbus_c3(uint8_t data)
{
	if (m_cs)
	{
		m_maincpu->reset();
	}
}
