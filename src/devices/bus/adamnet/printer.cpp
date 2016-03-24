// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam printer emulation

**********************************************************************/

#include "printer.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6801_TAG       "u2"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ADAM_PRN = &device_creator<adam_printer_device>;


//-------------------------------------------------
//  ROM( adam_prn )
//-------------------------------------------------

ROM_START( adam_prn )
	ROM_REGION( 0x800, M6801_TAG, 0 )
	ROM_LOAD( "printer.u2", 0x000, 0x800, CRC(e8db783b) SHA1(32b40679749ad0317c2c9ee9ca619fad6d850ce7) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *adam_printer_device::device_rom_region() const
{
	return ROM_NAME( adam_prn );
}


//-------------------------------------------------
//  ADDRESS_MAP( adam_prn_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( adam_prn_mem, AS_PROGRAM, 8, adam_printer_device )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE(M6801_TAG, m6801_cpu_device, m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6801_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( adam_prn_io )
//-------------------------------------------------

static ADDRESS_MAP_START( adam_prn_io, AS_IO, 8, adam_printer_device )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_WRITE(p1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(p2_r, p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3) AM_READ(p3_r)
	AM_RANGE(M6801_PORT4, M6801_PORT4) AM_READWRITE(p4_r, p4_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( adam_prn )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( adam_prn )
	MCFG_CPU_ADD(M6801_TAG, M6801, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(adam_prn_mem)
	MCFG_CPU_IO_MAP(adam_prn_io)
	MCFG_DEVICE_DISABLE() // TODO
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor adam_printer_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( adam_prn );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_printer_device - constructor
//-------------------------------------------------

adam_printer_device::adam_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADAM_PRN, "Adam printer", tag, owner, clock, "adam_prn", __FILE__),
		device_adamnet_card_interface(mconfig, *this),
		m_maincpu(*this, M6801_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_printer_device::device_start()
{
}


//-------------------------------------------------
//  adamnet_reset_w -
//-------------------------------------------------

void adam_printer_device::adamnet_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_printer_device::p1_w )
{
	/*

	    bit     description

	    0       M2 phase D
	    1       M2 phase B
	    2       M2 phase C
	    3       M2 phase A
	    4       M3 phase B
	    5       M3 phase D
	    6       M3 phase A
	    7       M3 phase C

	*/
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_printer_device::p2_r )
{
	/*

	    bit     description

	    0       mode bit 0
	    1       mode bit 1
	    2       mode bit 2
	    3       NET RXD
	    4       NET TXD

	*/

	UINT8 data = M6801_MODE_7;

	// NET RXD
	data |= m_bus->rxd_r(this) << 3;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_printer_device::p2_w )
{
	/*

	    bit     description

	    0       mode bit 0
	    1       mode bit 1
	    2       mode bit 2
	    3       NET RXD
	    4       NET TXD

	*/

	m_bus->txd_w(this, BIT(data, 4));
}


//-------------------------------------------------
//  p3_r -
//-------------------------------------------------

READ8_MEMBER( adam_printer_device::p3_r )
{
	return 0xff;
}


//-------------------------------------------------
//  p4_r -
//-------------------------------------------------

READ8_MEMBER( adam_printer_device::p4_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       left margin
	    5       platen detent
	    6       wheel home
	    7       self-test

	*/

	return 0x80;
}


//-------------------------------------------------
//  p4_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_printer_device::p4_w )
{
	/*

	    bit     description

	    0       print hammer solenoid
	    1       ribbon advance solenoid
	    2       platen motor advance
	    3       platen motor break
	    4
	    5
	    6
	    7

	*/
}
