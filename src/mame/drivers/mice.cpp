// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
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

class mice_state : public driver_device
{
public:
	mice_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void mice2(machine_config &config);
	void mice(machine_config &config);
	void mice2_io(address_map &map);
	void mice2_mem(address_map &map);
	void mice_io(address_map &map);
	void mice_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};


ADDRESS_MAP_START(mice_state::mice_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("mcp", 0)
	AM_RANGE(0x4400, 0x47ff) AM_RAM //(U13)
	AM_RANGE(0x6000, 0x60ff) AM_DEVREADWRITE("rpt", i8155_device, memory_r, memory_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(mice_state::mice_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x50, 0x50) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x51, 0x51) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x60, 0x67) AM_DEVREADWRITE("rpt", i8155_device, io_r, io_w)
	AM_RANGE(0x70, 0x73) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END

ADDRESS_MAP_START(mice_state::mice2_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("cep", 0)
	AM_RANGE(0x9000, 0x90ff) AM_DEVREADWRITE("rpt", i8155_device, memory_r, memory_w)
	AM_RANGE(0xb000, 0xb7ff) AM_RAM
	AM_RANGE(0xe800, 0xe8ff) AM_DEVREADWRITE("rtt8155", i8155_device, memory_r, memory_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(mice_state::mice2_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x81, 0x81) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x90, 0x97) AM_DEVREADWRITE("rpt", i8155_device, io_r, io_w)
	AM_RANGE(0xa0, 0xa3) AM_DEVREADWRITE("ppi", i8255_device, read, write)
	AM_RANGE(0xc0, 0xc3) AM_DEVREADWRITE("rttppi1", i8255_device, read, write)
	AM_RANGE(0xc8, 0xcb) AM_DEVREADWRITE("rttppi2", i8255_device, read, write)
	AM_RANGE(0xd0, 0xd3) AM_DEVREADWRITE("rttppi3", i8255_device, read, write)
	AM_RANGE(0xd8, 0xdb) AM_DEVREADWRITE("rttppi4", i8255_device, read, write)
	AM_RANGE(0xe0, 0xe3) AM_DEVREADWRITE("rttppi5", i8255_device, read, write)
	AM_RANGE(0xe8, 0xed) AM_DEVREADWRITE("rtt8155", i8155_device, io_r, io_w)
ADDRESS_MAP_END

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
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( mice2_terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END


MACHINE_CONFIG_START(mice_state::mice)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, 6.144_MHz_XTAL)
	MCFG_CPU_PROGRAM_MAP(mice_mem)
	MCFG_CPU_IO_MAP(mice_io)

	MCFG_DEVICE_ADD("uart", I8251, 6.144_MHz_XTAL / 2)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_I8251_TXRDY_HANDLER(INPUTLINE("maincpu", I8085_RST65_LINE))
	MCFG_I8251_RXRDY_HANDLER(INPUTLINE("maincpu", I8085_RST75_LINE))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart", i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", mice_terminal)

	MCFG_DEVICE_ADD("rpt", I8155, 6.144_MHz_XTAL / 2)
	MCFG_I8155_IN_PORTC_CB(IOPORT("BAUD"))
	MCFG_I8155_OUT_TIMEROUT_CB(DEVWRITELINE("uart", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("ppi", I8255, 0)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(mice_state::mice2)
	mice(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mice2_mem)
	MCFG_CPU_IO_MAP(mice2_io)

	MCFG_DEVICE_MODIFY("rs232")
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", mice2_terminal)

	MCFG_DEVICE_ADD("rttppi1", I8255, 0)
	MCFG_DEVICE_ADD("rttppi2", I8255, 0)
	MCFG_DEVICE_ADD("rttppi3", I8255, 0)
	MCFG_DEVICE_ADD("rttppi4", I8255, 0)
	MCFG_DEVICE_ADD("rttppi5", I8255, 0)
	MCFG_DEVICE_ADD("rtt8155", I8155, 0)
MACHINE_CONFIG_END

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

/* Driver */

//    YEAR  NAME        PARENT     COMPAT   MACHINE   INPUT   CLASS       INIT  COMPANY                   FULLNAME                   FLAGS
COMP( 1981, mice_6502,  0,         0,       mice,     mice,   mice_state, 0,    "Microtek International", "MICE 6502 (Rev-A)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1984, mice2_z80,  0,         0,       mice2,    micev3, mice_state, 0,    "Microtek International", "MICE-II Z80 (Rev-F)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1987, mice2_6502, mice2_z80, 0,       mice2,    micev3, mice_state, 0,    "Microtek International", "MICE-II 6502 (Rev-F)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1987, mice2_8085, mice2_z80, 0,       mice2,    micev3, mice_state, 0,    "Microtek International", "MICE-II 8085 (Rev-M)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1987, mice2_6809, mice2_z80, 0,       mice2,    micev3, mice_state, 0,    "Microtek International", "MICE-II 6809(E) (Rev-L)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
