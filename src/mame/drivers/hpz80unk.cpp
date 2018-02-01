// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

HP Z80-based unknown in a large metal cage

2012-05-25 Skeleton driver [Robbbert]

http://www.classiccmp.org/hp/unknown Z80 computer/

Looks like roms are in 2 banks in range C000-FFFF.
BASIC is included, if we can find out how to access it.

Commands:
Axxxx         Disassemble (. to quit)
DAxxxx,yyyy   Ascii Dump of memory
DBxxxx,yyyy   Binary Dump of memory
DHxxxx,yyyy   Hex Dump of memory
DOxxxx,yyyy   Octal dump of memory
G
H
L
MMxxxx        Modify Memory (. to quit)
Pxx           Binary Display of Port
Pxx,xx        Write to port
RC            ???
RF            ???
RM            ???
RT            ???
UC            Displays 11111111
US            ???
UZ            Displays FFFF
W             Punch papertape
X             choose Q,V,R,P (Q to quit; others ask for ram and prom ranges)
Y nothing
Z nothing

    ToDo:
    - Almost everything; there are a lot of I/O ports used
    - Hook up rom banking

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ay31015.h"
#include "bus/rs232/rs232.h"


class hpz80unk_state : public driver_device
{
public:
	hpz80unk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_rom(*this, "rom")
		, m_uart1(*this, "uart1")
		, m_uart2(*this, "uart2")
		, m_uart3(*this, "uart3")
	{ }

	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port01_r);
	DECLARE_READ8_MEMBER(port02_r);
	DECLARE_READ8_MEMBER(port03_r);
	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_READ8_MEMBER(port0d_r);
	DECLARE_WRITE8_MEMBER(port01_w);
	DECLARE_WRITE8_MEMBER(port04_w);
	DECLARE_WRITE8_MEMBER(port0e_w);
	DECLARE_READ8_MEMBER(portfc_r);

	void hpz80unk(machine_config &config);
	void hpz80unk_io(address_map &map);
	void hpz80unk_mem(address_map &map);
private:
	uint8_t m_port02_data;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_rom;
	required_device<ay31015_device> m_uart1;
	required_device<ay31015_device> m_uart2;
	required_device<ay31015_device> m_uart3;
};

READ8_MEMBER( hpz80unk_state::port00_r )
{
	return (m_uart1->get_output_pin(AY31015_DAV) << 1) | (m_uart1->get_output_pin(AY31015_TBMT)) | 0xfc;
}

READ8_MEMBER( hpz80unk_state::port01_r )
{
	m_uart1->set_input_pin(AY31015_RDAV, 0);
	uint8_t result = m_uart1->get_received_data();
	m_uart1->set_input_pin(AY31015_RDAV, 1);
	return result;
}

WRITE8_MEMBER( hpz80unk_state::port01_w )
{
	m_uart1->set_transmit_data(data);
}

READ8_MEMBER( hpz80unk_state::port02_r )
{
	m_port02_data ^= 1;
	return m_port02_data;
}

READ8_MEMBER( hpz80unk_state::port03_r )
{
	return (m_uart2->get_output_pin(AY31015_DAV) << 1) | (m_uart2->get_output_pin(AY31015_TBMT)) | 0xfc;
}

READ8_MEMBER( hpz80unk_state::port04_r )
{
	m_uart2->set_input_pin(AY31015_RDAV, 0);
	uint8_t result = m_uart2->get_received_data();
	m_uart2->set_input_pin(AY31015_RDAV, 1);
	return result;
}

WRITE8_MEMBER( hpz80unk_state::port04_w )
{
	m_uart2->set_transmit_data(data);
}

READ8_MEMBER( hpz80unk_state::port0d_r )
{
	return (m_uart3->get_output_pin(AY31015_DAV) << 1) | (m_uart3->get_output_pin(AY31015_TBMT)) | 0xfc;
}

WRITE8_MEMBER( hpz80unk_state::port0e_w )
{
	m_uart3->set_transmit_data(data);
}

READ8_MEMBER( hpz80unk_state::portfc_r )
{
	return 0xfe; // or it halts
}

ADDRESS_MAP_START(hpz80unk_state::hpz80unk_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_SHARE("rom")
ADDRESS_MAP_END

ADDRESS_MAP_START(hpz80unk_state::hpz80unk_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(port00_r) // uart1 status
	AM_RANGE(0x01, 0x01) AM_READWRITE(port01_r,port01_w) // uart1 data
	AM_RANGE(0x02, 0x02) AM_READ(port02_r)
	AM_RANGE(0x03, 0x03) AM_READ(port03_r) // uart2 status
	AM_RANGE(0x04, 0x04) AM_READWRITE(port04_r,port04_w) // uart2 data
	AM_RANGE(0x0d, 0x0d) AM_READ(port0d_r) // uart3 status
	AM_RANGE(0x0e, 0x0e) AM_WRITE(port0e_w) // uart3 data
	AM_RANGE(0x1d, 0x1e) // top of memory is written here, big-endian
	AM_RANGE(0x1f, 0x1f) AM_READ_PORT("DSW") // select which uarts to use
	AM_RANGE(0xfc, 0xfc) AM_READ(portfc_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( hpz80unk )
	// this is a theoretical switch
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, "UART selection")
	PORT_DIPSETTING(    0x00, "In UART1, Out UART1")
	PORT_DIPSETTING(    0x01, "In UART1, Out UART2")
	PORT_DIPSETTING(    0x02, "In UART1, Out UART3")
	PORT_DIPSETTING(    0x03, "In UART2, Out UART1")
INPUT_PORTS_END


void hpz80unk_state::machine_reset()
{
	uint8_t* user1 = memregion("user1")->base();
	memcpy((uint8_t*)m_p_rom, user1, 0x4000);
	m_maincpu->set_pc(0xc000);

	// no idea if these are hard-coded, or programmable
	m_uart1->set_input_pin(AY31015_XR, 0);
	m_uart1->set_input_pin(AY31015_XR, 1);
	m_uart1->set_input_pin(AY31015_SWE, 0);
	m_uart1->set_input_pin(AY31015_NP, 1);
	m_uart1->set_input_pin(AY31015_TSB, 0);
	m_uart1->set_input_pin(AY31015_NB1, 1);
	m_uart1->set_input_pin(AY31015_NB2, 1);
	m_uart1->set_input_pin(AY31015_EPS, 1);
	m_uart1->set_input_pin(AY31015_CS, 1);
	m_uart1->set_input_pin(AY31015_CS, 0);

	m_uart2->set_input_pin(AY31015_XR, 0);
	m_uart2->set_input_pin(AY31015_XR, 1);
	m_uart2->set_input_pin(AY31015_SWE, 0);
	m_uart2->set_input_pin(AY31015_NP, 1);
	m_uart2->set_input_pin(AY31015_TSB, 0);
	m_uart2->set_input_pin(AY31015_NB1, 1);
	m_uart2->set_input_pin(AY31015_NB2, 1);
	m_uart2->set_input_pin(AY31015_EPS, 1);
	m_uart2->set_input_pin(AY31015_CS, 1);
	m_uart2->set_input_pin(AY31015_CS, 0);

	m_uart3->set_input_pin(AY31015_XR, 0);
	m_uart3->set_input_pin(AY31015_XR, 1);
	m_uart3->set_input_pin(AY31015_SWE, 0);
	m_uart3->set_input_pin(AY31015_NP, 1);
	m_uart3->set_input_pin(AY31015_TSB, 0);
	m_uart3->set_input_pin(AY31015_NB1, 1);
	m_uart3->set_input_pin(AY31015_NB2, 1);
	m_uart3->set_input_pin(AY31015_EPS, 1);
	m_uart3->set_input_pin(AY31015_CS, 1);
	m_uart3->set_input_pin(AY31015_CS, 0);
	// this should be rom/ram banking
}


MACHINE_CONFIG_START(hpz80unk_state::hpz80unk)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL(4'000'000))
	MCFG_CPU_PROGRAM_MAP(hpz80unk_mem)
	MCFG_CPU_IO_MAP(hpz80unk_io)

	/* video hardware */
	MCFG_DEVICE_ADD("uart1", AY51013, 0) // COM2502
	MCFG_AY51013_TX_CLOCK(153600)
	MCFG_AY51013_RX_CLOCK(153600)
	MCFG_AY51013_READ_SI_CB(DEVREADLINE("rs232a", rs232_port_device, rxd_r))
	MCFG_AY51013_WRITE_SO_CB(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, "terminal")

	MCFG_DEVICE_ADD("uart2", AY51013, 0) // COM2502
	MCFG_AY51013_TX_CLOCK(153600)
	MCFG_AY51013_RX_CLOCK(153600)
	MCFG_AY51013_READ_SI_CB(DEVREADLINE("rs232b", rs232_port_device, rxd_r))
	MCFG_AY51013_WRITE_SO_CB(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, nullptr)

	MCFG_DEVICE_ADD("uart3", AY51013, 0) // COM2502
	MCFG_AY51013_TX_CLOCK(153600)
	MCFG_AY51013_RX_CLOCK(153600)
	MCFG_AY51013_READ_SI_CB(DEVREADLINE("rs232c", rs232_port_device, rxd_r))
	MCFG_AY51013_WRITE_SO_CB(DEVWRITELINE("rs232c", rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD("rs232c", default_rs232_devices, nullptr)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( hpz80unk )
	ROM_REGION( 0x8000, "user1", 0 )
	// 1st bank
	ROM_LOAD( "u1",        0x0000, 0x0800, CRC(080cd04a) SHA1(42004af65d44e3507a4e0f343c5bf385b6377c40) )
	ROM_LOAD( "u3",        0x0800, 0x0800, CRC(694075e1) SHA1(3db62645ade6a7f454b2d505aecc1661284c8ce2) )
	ROM_LOAD( "u5",        0x1000, 0x0800, CRC(5573bd05) SHA1(68c8f02b3fe9d77ecb83df407ca78430e118004a) )
	ROM_LOAD( "u7",        0x1800, 0x0800, CRC(d18a304a) SHA1(69dd0486bb6e4c2a22ab9da863bfb962016a321b) )
	ROM_LOAD( "u9",        0x2000, 0x0800, CRC(f7a8665c) SHA1(e39d0ba4ce2dc773622d411a25f40a6a24b45449) )
	ROM_LOAD( "u11",       0x2800, 0x0800, CRC(6c1ac77a) SHA1(50ca04ff0a11bd1c7d96f4731cef50978266ecca) )
	ROM_LOAD( "u13",       0x3000, 0x0800, CRC(8b166911) SHA1(4301dcd6840d37ccfa5bff998a0d88bebe99dc31) )
	ROM_LOAD( "u15",       0x3800, 0x0800, CRC(c6300499) SHA1(1b62d2a85c8f0b6a817e4be73ee34e0d90515c00) )
	// 2nd bank
	ROM_LOAD( "u2",        0x4000, 0x0800, CRC(080cd04a) SHA1(42004af65d44e3507a4e0f343c5bf385b6377c40) )
	ROM_LOAD( "u4",        0x4800, 0x0800, CRC(66c3745c) SHA1(d79fe764312a222ac64d325bf5f4abc7ca401d0f) )
	ROM_LOAD( "u6",        0x5000, 0x0800, CRC(80761b4c) SHA1(5f6a12fbba533308b9fe7067c67a836be436a6f0) )
	ROM_LOAD( "u8",        0x5800, 0x0800, CRC(64a2be18) SHA1(b11c08fdc9dc126038559462493f458ecdc78532) )
	ROM_LOAD( "u10",       0x6000, 0x0800, CRC(40244d09) SHA1(106f8f978de36df9f3ebbe1e2c959b60e53273a2) )
	ROM_LOAD( "u12",       0x6800, 0x0800, CRC(6eb01765) SHA1(66f9036a9f86cf3a79493330bbc06fb6932ab771) )
	ROM_LOAD( "u14",       0x7000, 0x0800, CRC(3410e682) SHA1(30d94c0c0b6478dab202a603edaccca943008e35) )
	ROM_LOAD( "u16",       0x7800, 0x0800, CRC(c03fdcab) SHA1(1081d787085add489c6e2a1d450e1a5790d18885) )
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT     STATE           INIT  COMPANY            FULLNAME                       FLAGS
COMP( 1977, hpz80unk, 0,      0,       hpz80unk,  hpz80unk, hpz80unk_state, 0,    "Hewlett-Packard", "unknown Z80-based mainframe", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
