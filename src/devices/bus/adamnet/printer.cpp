// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam printer emulation

**********************************************************************/

#include "emu.h"
#include "printer.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6801_TAG       "u2"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADAM_PRN, adam_printer_device, "adam_prn", "Adam printer")


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

const tiny_rom_entry *adam_printer_device::device_rom_region() const
{
	return ROM_NAME( adam_prn );
}


//-------------------------------------------------
//  ADDRESS_MAP( adam_prn_mem )
//-------------------------------------------------

void adam_printer_device::adam_prn_mem(address_map &map)
{
	map(0x0000, 0x001f).rw(M6801_TAG, FUNC(m6801_cpu_device::m6801_io_r), FUNC(m6801_cpu_device::m6801_io_w));
	map(0x0080, 0x00ff).ram();
	map(0xf800, 0xffff).rom().region(M6801_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( adam_prn_io )
//-------------------------------------------------

void adam_printer_device::adam_prn_io(address_map &map)
{
	map(M6801_PORT1, M6801_PORT1).w(FUNC(adam_printer_device::p1_w));
	map(M6801_PORT2, M6801_PORT2).rw(FUNC(adam_printer_device::p2_r), FUNC(adam_printer_device::p2_w));
	map(M6801_PORT3, M6801_PORT3).r(FUNC(adam_printer_device::p3_r));
	map(M6801_PORT4, M6801_PORT4).rw(FUNC(adam_printer_device::p4_r), FUNC(adam_printer_device::p4_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(adam_printer_device::device_add_mconfig)
	MCFG_DEVICE_ADD(M6801_TAG, M6801, XTAL(4'000'000))
	MCFG_DEVICE_PROGRAM_MAP(adam_prn_mem)
	MCFG_DEVICE_IO_MAP(adam_prn_io)
	MCFG_DEVICE_DISABLE() // TODO
MACHINE_CONFIG_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_printer_device - constructor
//-------------------------------------------------

adam_printer_device::adam_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ADAM_PRN, tag, owner, clock),
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

	uint8_t data = M6801_MODE_7;

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
