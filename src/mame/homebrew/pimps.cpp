// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

P.I.M.P.S. (Personal Interactive MicroProcessor System)

2009-12-06 Skeleton driver.

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

 Use 'S' instead of 'SP', eg: LXI S,$1000

 Only EQU, DB, DW and END directives are supported. An END statement is
 REQUIRED (otherwise you get the message '?tab-ful' as it fills the symbol
 table with garbage occuring in memory after the end of the file).

 RST instructions are implemented as 8 separate 'RST0'-'RST8' memonics.

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/i8251.h"


namespace {

class pimps_state : public driver_device
{
public:
	pimps_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "roms")
		, m_ram(*this, "mainram")
	{ }

	void pimps(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	memory_passthrough_handler m_rom_shadow_tap;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
};


void pimps_state::mem_map(address_map &map)
{
	map(0x0000, 0xefff).ram().share("mainram");
	map(0xf000, 0xffff).rom().region("roms", 0);
}

void pimps_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0, 0xf1).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf2, 0xf3).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
}

/* Input ports */
static INPUT_PORTS_START( pimps )
INPUT_PORTS_END


void pimps_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf000, 0xf7ff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

// baud is not documented, we will use 9600
static DEVICE_INPUT_DEFAULTS_START( terminal ) // set up terminal to default to 9600, 7 bits, even parity
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void pimps_state::pimps(machine_config &config)
{
	I8085A(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pimps_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pimps_state::io_map);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 153'600));
	uart_clock.signal_handler().set("uart1", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart1", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_rxc));

	i8251_device &uart1(I8251(config, "uart1", 0));
	uart1.txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set("rs232a", FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set("uart1", FUNC(i8251_device::write_cts));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be exactly here

	i8251_device &uart2(I8251(config, "uart2", 0));
	uart2.txd_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	uart2.dtr_handler().set("rs232b", FUNC(rs232_port_device::write_dtr));
	uart2.rts_handler().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("uart2", FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set("uart2", FUNC(i8251_device::write_dsr));
	rs232b.cts_handler().set("uart2", FUNC(i8251_device::write_cts));
}

/* ROM definition */
ROM_START( pimps )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "pimps.bin", 0x0000, 0x1000, CRC(5da1898f) SHA1(d20e31d0981a1f54c83186dbdfcf4280e49970d0))
ROM_END

} // anonymous namespace

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  STATE        INIT        COMPANY          FULLNAME      FLAGS */
COMP( 197?, pimps, 0,      0,      pimps,   pimps, pimps_state, empty_init, "Henry Colford", "P.I.M.P.S.", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE ) // terminal beeps
