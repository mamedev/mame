// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Microtek International Inc MICE (Micro-In-Circuit Emulator)

2013-08-27 Skeleton driver.

This is a CPU emulator for development work.

Each emulated CPU has its own plugin "personality" card, containing the CPU
itself, an XTAL clock and enough latches, buffers and gates to allow total
control to be exercised over its bus activity.

In the original MICE system, the MCP-85 main control card could be reused
with different CPU cards after replacing the EPROMs. The more spaciously
designed MICE-II eliminated this as a separate board and moved its primary
components (8085/8255/8251/8155/6116/ROMs/6.144 XTAL) onto the CEP (Control
Emulation Processor) card for each CPU; however, the RTT (Real-time Trace)
board (providing extra I/O ports) and UEM (Universal Emulation Memory)
boards could be shared between CEP cards.

The connection to the outside world is via a RS-232C port to a terminal.
The serial protocol is configurable through a 6-position DIP switch.

There's a mistake in the boot rom: if the test of the 8155 or 8255 fail, it
attempts to write a suitable message to the screen, but as the 8251 hasn't
yet been initialised, it hangs.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "bus/rs232/rs232.h"


namespace {

class mice_state : public driver_device
{
public:
	mice_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void mice2(machine_config &config);
	void mice(machine_config &config);

private:
	void mice2_io(address_map &map) ATTR_COLD;
	void mice2_mem(address_map &map) ATTR_COLD;
	void mice_io(address_map &map) ATTR_COLD;
	void mice_mem(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_maincpu;
};


void mice_state::mice_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().region("mcp", 0);
	map(0x4400, 0x47ff).ram(); //(U13)
	map(0x6000, 0x60ff).rw("rpt", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
}

void mice_state::mice_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x50, 0x51).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x60, 0x67).rw("rpt", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x70, 0x73).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void mice_state::mice2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom().region("cep", 0);
	map(0x9000, 0x90ff).rw("rpt", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xb000, 0xb7ff).ram();
	map(0xe800, 0xe8ff).rw("rtt8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
}

void mice_state::mice2_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x81).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x90, 0x97).rw("rpt", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xa0, 0xa3).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc0, 0xc3).rw("rttppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc8, 0xcb).rw("rttppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xd0, 0xd3).rw("rttppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xd8, 0xdb).rw("rttppi4", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe0, 0xe3).rw("rttppi5", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe8, 0xed).rw("rtt8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

/* Input ports */
static INPUT_PORTS_START( mice )
	PORT_START("BAUD")
	PORT_DIPNAME(0x07, 0x02, "Baud Rate") PORT_DIPLOCATION("DSW7:1,2,3")
	PORT_DIPSETTING(0x07, "110")
	PORT_DIPSETTING(0x06, "150")
	PORT_DIPSETTING(0x05, "300")
	PORT_DIPSETTING(0x04, "600")
	PORT_DIPSETTING(0x03, "1200")
	PORT_DIPSETTING(0x02, "2400")
	PORT_DIPSETTING(0x01, "4800")
	PORT_DIPSETTING(0x00, "9600")
	PORT_DIPNAME(0x08, 0x00, "Data Bits") PORT_DIPLOCATION("DSW7:4")
	PORT_DIPSETTING(0x00, "7")
	PORT_DIPSETTING(0x08, "8")
	PORT_DIPNAME(0x30, 0x30, "Parity") PORT_DIPLOCATION("DSW7:5,6")
	PORT_DIPSETTING(0x00, DEF_STR(None))
	PORT_DIPSETTING(0x30, "Even")
	PORT_DIPSETTING(0x10, "Odd")
	// "The number of stop bits is permanently set to one; and the communication is full duplex." (manual, p. 6)
INPUT_PORTS_END

static INPUT_PORTS_START( micev3 )
	PORT_START("BAUD")
	PORT_DIPNAME(0x07, 0x02, "Baud Rate") PORT_DIPLOCATION("U17:1,2,3")
	PORT_DIPSETTING(0x06, "150")
	PORT_DIPSETTING(0x05, "300")
	PORT_DIPSETTING(0x04, "600")
	PORT_DIPSETTING(0x03, "1200")
	PORT_DIPSETTING(0x02, "2400")
	PORT_DIPSETTING(0x01, "4800")
	PORT_DIPSETTING(0x00, "9600")
	PORT_DIPSETTING(0x07, "19200") // 110 baud on older firmware versions
	PORT_DIPNAME(0x08, 0x00, "Data Bits") PORT_DIPLOCATION("U17:4")
	PORT_DIPSETTING(0x00, "7")
	PORT_DIPSETTING(0x08, "8")
	PORT_DIPNAME(0x30, 0x30, "Parity") PORT_DIPLOCATION("U17:5,6")
	PORT_DIPSETTING(0x00, DEF_STR(None))
	PORT_DIPSETTING(0x30, "Even")
	PORT_DIPSETTING(0x10, "Odd")
	// "The number of stop bits is permanently set at two; and the communication is full duplex." (manual, p. 2-7)
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( mice_terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( mice2_terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END


void mice_state::mice(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mice_state::mice_mem);
	m_maincpu->set_addrmap(AS_IO, &mice_state::mice_io);

	i8251_device &uart(I8251(config, "uart", 6.144_MHz_XTAL / 2));
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	uart.txrdy_handler().set_inputline("maincpu", I8085_RST65_LINE);
	uart.rxrdy_handler().set_inputline("maincpu", I8085_RST75_LINE);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("uart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart", FUNC(i8251_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(mice_terminal));

	i8155_device &rpt(I8155(config, "rpt", 6.144_MHz_XTAL / 2));
	rpt.in_pc_callback().set_ioport("BAUD");
	rpt.out_to_callback().set("uart", FUNC(i8251_device::write_txc));
	rpt.out_to_callback().append("uart", FUNC(i8251_device::write_rxc));

	I8255(config, "ppi");
}

void mice_state::mice2(machine_config &config)
{
	mice(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mice_state::mice2_mem);
	m_maincpu->set_addrmap(AS_IO, &mice_state::mice2_io);

	subdevice<rs232_port_device>("rs232")->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(mice2_terminal));

	I8255(config, "rttppi1");
	I8255(config, "rttppi2");
	I8255(config, "rttppi3");
	I8255(config, "rttppi4");
	I8255(config, "rttppi5");
	I8155(config, "rtt8155", 0);
}

/* ROM definitions */
ROM_START( mice_6502 )
	ROM_REGION( 0x4000, "mcp", 0 )
	ROM_LOAD( "6502_u10_v.2.0", 0x2000, 0x1000, CRC(496c53a7) SHA1(f28cddef18ab3e0eca1fea125dd678a54817c9df) )
	ROM_LOAD( "6502_u11_v.2.0", 0x1000, 0x1000, CRC(8d655bd2) SHA1(94936553f1692ede0934e3c7b599f3ad6adb6aec) )
	ROM_LOAD( "6502_u12_v.2.0", 0x0000, 0x1000, CRC(cee810ee) SHA1(ab642cda73f4b3f715ddc2909ba2b48cbd474d4d) )
ROM_END

ROM_START( mice2_z80 )
	ROM_REGION( 0x8000, "cep", 0 )
	ROM_LOAD( "z80_u2_v.3.0",   0x4000, 0x2000, CRC(992b1b53) SHA1(f7b66c49ab26a9f97b2e6ebe45d162daa66d8a67) )
	ROM_LOAD( "z80_u3_v.3.0",   0x2000, 0x2000, CRC(48d0be9b) SHA1(602af21868b1b5e6d488706a831259d78fefad6f) )
	ROM_LOAD( "z80_u4_v.3.0",   0x0000, 0x2000, CRC(4fe2d08d) SHA1(902b98357b8f2e61f68dd171478368a3ac47af6e) )
ROM_END

ROM_START( mice2_6502 )
	ROM_REGION( 0x8000, "cep", 0 )
	ROM_LOAD( "6502_u1_v.3.2",  0x6000, 0x2000, CRC(0ba10943) SHA1(e7590e2c1d9d2b1ff8cca0f5da366650ea4d50e3) )
	ROM_LOAD( "6502_u2_v.3.2",  0x4000, 0x2000, CRC(f3169423) SHA1(a588a2e1894f523cf11c34d036beadbfe5b10538) )
	ROM_LOAD( "6502_u3_v.3.2",  0x2000, 0x2000, CRC(d5c77c3f) SHA1(71439735ed62db07bee713775ee2189120d1a1e7) )
	ROM_LOAD( "6502_u4_v.3.2",  0x0000, 0x2000, CRC(6acfc3a1) SHA1(3572a4798873c21a247a43da8419e7b9a181c67d) )
ROM_END

ROM_START( mice2_8085 )
	ROM_REGION( 0x8000, "cep", 0 )
	ROM_LOAD( "8085_u2_v.3.1",  0x4000, 0x2000, CRC(2fce00a5) SHA1(0611f928be663a9279781d9f496fc950fd4ee7e2) )
	ROM_LOAD( "8085_u3_v.3.1",  0x2000, 0x2000, CRC(16ee3018) SHA1(9e215504bcea2c5ebfb7578ecf371eec45cbe5d7) )
	ROM_LOAD( "8085_u4_v.3.1",  0x0000, 0x2000, CRC(5798f2b5) SHA1(e0fe9411394bded8a77bc6a0f71519aad7800125) )
ROM_END

ROM_START( mice2_6809 )
	ROM_REGION( 0x8000, "cep", 0 )
	ROM_LOAD( "6809_u1_v.3.4",  0x0000, 0x8000, CRC(b94d043d) SHA1(822697485f064286155f2a66cdbdcb0bd66ddb8c) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME        PARENT     COMPAT  MACHINE  INPUT   CLASS       INIT        COMPANY                   FULLNAME                   FLAGS
COMP( 1981, mice_6502,  0,         0,      mice,    mice,   mice_state, empty_init, "Microtek International", "MICE 6502 (Rev-A)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1984, mice2_z80,  0,         0,      mice2,   micev3, mice_state, empty_init, "Microtek International", "MICE-II Z80 (Rev-F)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1987, mice2_6502, mice2_z80, 0,      mice2,   micev3, mice_state, empty_init, "Microtek International", "MICE-II 6502 (Rev-F)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1987, mice2_8085, mice2_z80, 0,      mice2,   micev3, mice_state, empty_init, "Microtek International", "MICE-II 8085 (Rev-M)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1987, mice2_6809, mice2_z80, 0,      mice2,   micev3, mice_state, empty_init, "Microtek International", "MICE-II 6809(E) (Rev-L)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
