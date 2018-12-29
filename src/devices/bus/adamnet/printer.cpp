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
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void adam_printer_device::device_add_mconfig(machine_config &config)
{
	M6801(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &adam_printer_device::adam_prn_mem);
	m_maincpu->out_p1_cb().set(FUNC(adam_printer_device::p1_w));
	m_maincpu->in_p2_cb().set(FUNC(adam_printer_device::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(adam_printer_device::p2_w));
	m_maincpu->in_p3_cb().set(FUNC(adam_printer_device::p3_r));
	m_maincpu->in_p4_cb().set(FUNC(adam_printer_device::p4_r));
	m_maincpu->out_p4_cb().set(FUNC(adam_printer_device::p4_w));
	m_maincpu->set_disable(); // TODO
}



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
