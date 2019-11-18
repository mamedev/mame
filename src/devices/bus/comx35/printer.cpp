// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Serial/Parallel Printer Card emulation

**********************************************************************/

#include "emu.h"
#include "printer.h"
#include "bus/centronics/printer.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define CENTRONICS_TAG  "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMX_PRN, comx_prn_device, "comx_prn", "COMX-35 Printer Card")


//-------------------------------------------------
//  ROM( comx_prn )
//-------------------------------------------------

ROM_START( comx_prn )
	ROM_REGION( 0x2000, "c000", 0 )
	ROM_SYSTEM_BIOS( 0, "comx", "COMX" )
	ROMX_LOAD( "printer.bin",           0x0000, 0x0800, CRC(3bbc2b2e) SHA1(08bf7ea4174713ab24969c553affd5c1401876b8), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "fm12", "F&M v1.2" )
	ROMX_LOAD( "f+m.printer.1.2.bin",   0x0000, 0x1000, CRC(2feb997d) SHA1(ee9cb91042696c88ff5f2f44d2f702dc93369ba0), ROM_BIOS(1) )
	ROM_LOAD( "rs232.bin",              0x1000, 0x0800, CRC(926ff2d1) SHA1(be02bd388bba0211ea72d4868264a63308e4318d) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *comx_prn_device::device_rom_region() const
{
	return ROM_NAME( comx_prn );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void comx_prn_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit0));
	m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit1));
	m_centronics->perror_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit2));
	m_centronics->select_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit3));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	INPUT_BUFFER(config, m_cent_status_in);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_prn_device - constructor
//-------------------------------------------------

comx_prn_device::comx_prn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COMX_PRN, tag, owner, clock),
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

uint8_t comx_prn_device::comx_mrd_r(offs_t offset, int *extrom)
{
	uint8_t data = 0;

	if (offset >= 0xc000 && offset < 0xe000)
	{
		data = m_rom->base()[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  comx_io_r - I/O read
//-------------------------------------------------

uint8_t comx_prn_device::comx_io_r(offs_t offset)
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

void comx_prn_device::comx_io_w(offs_t offset, uint8_t data)
{
	/*
	    Parallel:

	    OUT 2 is used to send a byte to the printer

	    Serial:

	    OUT 2 is used to send a bit to the printer
	*/

	m_cent_data_out->write(data);
}
