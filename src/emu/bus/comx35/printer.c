// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Serial/Parallel Printer Card emulation

**********************************************************************/

#include "printer.h"
#include "bus/centronics/printer.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define CENTRONICS_TAG  "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_PRN = &device_creator<comx_prn_device>;


//-------------------------------------------------
//  ROM( comx_prn )
//-------------------------------------------------

ROM_START( comx_prn )
	ROM_REGION( 0x2000, "c000", 0 )
	ROM_SYSTEM_BIOS( 0, "comx", "COMX" )
	ROMX_LOAD( "printer.bin",           0x0000, 0x0800, CRC(3bbc2b2e) SHA1(08bf7ea4174713ab24969c553affd5c1401876b8), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "fm12", "F&M v1.2" )
	ROMX_LOAD( "f&m.printer.1.2.bin",   0x0000, 0x1000, CRC(2feb997d) SHA1(ee9cb91042696c88ff5f2f44d2f702dc93369ba0), ROM_BIOS(2) )
	ROM_LOAD( "rs232.bin",              0x1000, 0x0800, CRC(926ff2d1) SHA1(be02bd388bba0211ea72d4868264a63308e4318d) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *comx_prn_device::device_rom_region() const
{
	return ROM_NAME( comx_prn );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( comx_prn )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( comx_prn )
	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit0))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit1))
	MCFG_CENTRONICS_PERROR_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit2))
	MCFG_CENTRONICS_SELECT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit3))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor comx_prn_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( comx_prn );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_prn_device - constructor
//-------------------------------------------------

comx_prn_device::comx_prn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMX_PRN, "COMX-35 Printer Card", tag, owner, clock, "comx_prn", __FILE__),
	device_comx_expansion_card_interface(mconfig, *this),
	m_centronics(*this, CENTRONICS_TAG),
	m_cent_data_out(*this, "cent_data_out"),
	m_cent_status_in(*this, "cent_status_in"),
	m_rom(*this, "c000")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_prn_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_prn_device::device_reset()
{
}


//-------------------------------------------------
//  comx_mrd_r - memory read
//-------------------------------------------------

UINT8 comx_prn_device::comx_mrd_r(address_space &space, offs_t offset, int *extrom)
{
	UINT8 data = 0;

	if (offset >= 0xc000 && offset < 0xe000)
	{
		data = m_rom->base()[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  comx_io_r - I/O read
//-------------------------------------------------

UINT8 comx_prn_device::comx_io_r(address_space &space, offs_t offset)
{
	/*
	    Parallel:

	    INP 2 for the printer status, where:
	    b0=1: Acknowledge Fault
	    b1=0: Device Busy
	    b2=0: Paper Empty
	    b3=1: Device Not Selected

	    Serial:

	    INP 2 for the printer status and to start a new range of bits for the next byte.
	*/

	/*

	    bit     description

	    0       Acknowledge Fault
	    1       Device Busy
	    2       Paper Empty
	    3       Device Not Selected
	    4
	    5
	    6
	    7

	*/

	return m_cent_status_in->read();
}


//-------------------------------------------------
//  comx_io_w - I/O write
//-------------------------------------------------

void comx_prn_device::comx_io_w(address_space &space, offs_t offset, UINT8 data)
{
	/*
	    Parallel:

	    OUT 2 is used to send a byte to the printer

	    Serial:

	    OUT 2 is used to send a bit to the printer
	*/

	m_cent_data_out->write(space, 0, data);
}
