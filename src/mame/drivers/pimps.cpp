// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

P.I.M.P.S. (Personal Interactive MicroProcessor System)

06/12/2009 Skeleton driver.

No schematics or hardware info available.

http://www.classiccmp.org/dunfield/pimps/index.htm

Commands:
A xxxx xxxx   - Assemble (from editor file): origin destination
D xxxx,xxxx   - Dump memory to host port
E             - Enter editor
 A            - Append at end of file
 C            - Clear memory file (reqires ESCAPE to confirm)
 D xx         - Delete line number xx
 I xx         - Insert at line xx
 L xx xx      - List range of lines
 Q            - Query: Display highest used address
 X            - eXit editor (requires ESCAPE to confirm)
 $ xxxx xxxx  - exit directly to assembler
F             - set for Full-duplex host operation
G xxxx        - Go (execute) at address
H             - set for Half-duplex host operation
M xxxx,xxxx   - Display a range of memory (hex dump)
P xx xx-[xx.] - display/Edit I/O Port
S xxxx        - Substitute data into memory
T             - Transparent link to host (Ctrl-A exits)
U             - set host Uart parameters (8251 control registers)
V             - Virtual memory (Not used on PIMPS board)

Notes:

The 'D'ump command outputs memory to the host in some form of Intel
HEX records, and waits for line-feed from the host before proceeding.

The 'V'irtual memory function was to control an EPROM emulator which
was part of the original design (see Chucks notes below) and was not
used on the PIMPS board.

Editor:
 Operates in HEXIDECIMAL line numbers. Only supports up to 256 lines
 (01-00). You must enter full two-digit number when prompted.
 You MUST 'C'lear the memory file before you begin, otherwise you will
 be editing random memory content.

Assembler:
 Comment is an INSTRUCTION! - this means that you need to put at least
 one space before and after ';' when entering a line comment.

 Does not understand DECIMAL numbers. It understands character constants
 ('c' and 'cc') and hex numbers ($xx or $xxxx).

 8-bit values MUST contain two hex digits or one quoted character. 16-bit
 constants MUST contain four hex digits or two quoted characters.

 Use 'S' instead if 'SP', eg: LXI S,$1000

 Only EQU, DB, DW and END directives are supported. An END statement is
 REQUIRED (otherwise you get the message '?tab-ful' as it fills the symbol
 table with garbage occuring in memory after the end of the file).

 RST instructions are implemented as 8 separate 'RST0'-'RST8' memonics.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"


class pimps_state : public driver_device
{
public:
	pimps_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void pimps(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};


ADDRESS_MAP_START(pimps_state::mem_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(pimps_state::io_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	AM_RANGE(0xf2, 0xf2) AM_DEVREADWRITE("uart2", i8251_device, data_r, data_w)
	AM_RANGE(0xf3, 0xf3) AM_DEVREADWRITE("uart2", i8251_device, status_r, control_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pimps )
INPUT_PORTS_END


void pimps_state::machine_reset()
{
	m_maincpu->set_pc(0xf000);
}

// baud is not documented, we will use 9600
static DEVICE_INPUT_DEFAULTS_START( terminal ) // set up terminal to default to 9600, 7 bits, even parity
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

MACHINE_CONFIG_START(pimps_state::pimps)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A, XTAL(2'000'000))
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("uart1", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart1", i8251_device, write_rxc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart2", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart2", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("uart1", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart1", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart1", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart1", i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal) // must be exactly here

	MCFG_DEVICE_ADD("uart2", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart2", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart2", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart2", i8251_device, write_cts))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pimps )
	ROM_REGION( 0x1000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "pimps.bin", 0x0000, 0x1000, CRC(5da1898f) SHA1(d20e31d0981a1f54c83186dbdfcf4280e49970d0))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE         INIT  COMPANY          FULLNAME      FLAGS */
COMP( 197?, pimps,  0,      0,       pimps,     pimps, pimps_state,  0,    "Henry Colford", "P.I.M.P.S.", MACHINE_NO_SOUND_HW) // terminal beeps
