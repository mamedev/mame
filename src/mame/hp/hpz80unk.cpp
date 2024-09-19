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
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


namespace {

class hpz80unk_state : public driver_device
{
public:
	hpz80unk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_rom(*this, "rom")
		, m_uart(*this, "uart%u", 1U)
	{ }

	void hpz80unk(machine_config &config);

private:
	u8 port00_r();
	u8 port02_r();
	u8 port03_r();
	u8 port0d_r();
	u8 portfc_r();

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_port02_data = 0U;
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_p_rom;
	required_device_array<ay51013_device, 3> m_uart;
};

u8 hpz80unk_state::port00_r()
{
	return (m_uart[0]->dav_r() << 1) | (m_uart[0]->tbmt_r()) | 0xfc;
}

u8 hpz80unk_state::port02_r()
{
	m_port02_data ^= 1;
	return m_port02_data;
}

u8 hpz80unk_state::port03_r()
{
	return (m_uart[1]->dav_r() << 1) | (m_uart[1]->tbmt_r()) | 0xfc;
}

u8 hpz80unk_state::port0d_r()
{
	return (m_uart[2]->dav_r() << 1) | (m_uart[2]->tbmt_r()) | 0xfc;
}

u8 hpz80unk_state::portfc_r()
{
	return 0xfe; // or it halts
}

void hpz80unk_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xbfff).ram();
	map(0xc000, 0xffff).rom().share("rom");
}

void hpz80unk_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(hpz80unk_state::port00_r)); // uart1 status
	map(0x01, 0x01).rw("uart1", FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit)); // uart1 data
	map(0x02, 0x02).r(FUNC(hpz80unk_state::port02_r));
	map(0x03, 0x03).r(FUNC(hpz80unk_state::port03_r)); // uart2 status
	map(0x04, 0x04).rw("uart2", FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit)); // uart2 data
	map(0x0d, 0x0d).r(FUNC(hpz80unk_state::port0d_r)); // uart3 status
	map(0x0e, 0x0e).w("uart3", FUNC(ay31015_device::transmit)); // uart3 data
	map(0x1d, 0x1e); // top of memory is written here, big-endian
	map(0x1f, 0x1f).portr("DSW"); // select which uarts to use
	map(0xfc, 0xfc).r(FUNC(hpz80unk_state::portfc_r));
}

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


void hpz80unk_state::machine_start()
{
	save_item(NAME(m_port02_data));
}

void hpz80unk_state::machine_reset()
{
	u8* user1 = memregion("user1")->base();
	memcpy((u8*)m_p_rom, user1, 0x4000);
	m_maincpu->set_pc(0xc000);

	// no idea if these are hard-coded, or programmable
	for (auto &uart : m_uart)
	{
		uart->write_xr(0);
		uart->write_xr(1);
		uart->write_swe(0);
		uart->write_np(1);
		uart->write_tsb(0);
		uart->write_nb1(1);
		uart->write_nb2(1);
		uart->write_eps(1);
		uart->write_cs(1);
		uart->write_cs(0);
	}

	// this should be rom/ram banking
}


void hpz80unk_state::hpz80unk(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &hpz80unk_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hpz80unk_state::io_map);

	AY51013(config, m_uart[0]); // COM2502
	m_uart[0]->read_si_callback().set("rs232a", FUNC(rs232_port_device::rxd_r));
	m_uart[0]->write_so_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_uart[0]->set_auto_rdav(true);
	RS232_PORT(config, "rs232a", default_rs232_devices, "terminal");

	AY51013(config, m_uart[1]); // COM2502
	m_uart[1]->read_si_callback().set("rs232b", FUNC(rs232_port_device::rxd_r));
	m_uart[1]->write_so_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_uart[1]->set_auto_rdav(true);
	RS232_PORT(config, "rs232b", default_rs232_devices, nullptr);

	AY51013(config, m_uart[2]); // COM2502
	m_uart[2]->read_si_callback().set("rs232c", FUNC(rs232_port_device::rxd_r));
	m_uart[2]->write_so_callback().set("rs232c", FUNC(rs232_port_device::write_txd));
	m_uart[2]->set_auto_rdav(true);
	RS232_PORT(config, "rs232c", default_rs232_devices, nullptr);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set(m_uart[0], FUNC(ay51013_device::write_tcp));
	uart_clock.signal_handler().append(m_uart[0], FUNC(ay51013_device::write_rcp));
	uart_clock.signal_handler().append(m_uart[1], FUNC(ay51013_device::write_tcp));
	uart_clock.signal_handler().append(m_uart[1], FUNC(ay51013_device::write_rcp));
	uart_clock.signal_handler().append(m_uart[2], FUNC(ay51013_device::write_tcp));
	uart_clock.signal_handler().append(m_uart[2], FUNC(ay51013_device::write_rcp));
}

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

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY            FULLNAME                       FLAGS
COMP( 1977, hpz80unk, 0,      0,      hpz80unk, hpz80unk, hpz80unk_state, empty_init, "Hewlett-Packard", "unknown Z80-based mainframe", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
