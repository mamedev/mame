// license:BSD-3-Clause
// copyright-holders: Chris Swan
// based on gsz80.cpp by Frank Palazzolo

// All the common emulator stuff is here
#include "emu.h"

// Two member devices referenced in state class, a Z80 and a UART
#include "cpu/z80/z80.h"
#include "machine/6850acia.h"

// Two more devices needed, a clock device for the UART, and RS-232 devices
#include "machine/clock.h"
#include "bus/rs232/rs232.h"

// State class - derives from driver_device
class rc2014mini_state : public driver_device
{
public:
	rc2014mini_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")   // Tag name for Z80 is "maincpu"
		, m_acia(*this, "acia")         // Tag name for UART is "acia"
	{ }

	// This function sets up the machine configuration
	void rc2014mini(machine_config &config);

private:
	// address maps for program memory and io memory
	void rc2014mini_mem(address_map &map);
	void rc2014mini_io(address_map &map);

	// two member devices required here
	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia;
};

// Trivial memory map for program memory
void rc2014mini_state::rc2014mini_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0xffff).ram();
}

void rc2014mini_state::rc2014mini_io(address_map &map)
{
	map.global_mask(0xff);  // use 8-bit ports
	map.unmap_value_high(); // unmapped addresses return 0xff
	map(0x80, 0xbf).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
}

// This is here only to configure our terminal for interactive use
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void rc2014mini_state::rc2014mini(machine_config &config)
{
	/* basic machine hardware */

	// Configure member Z80 (via m_maincpu)
	Z80(config, m_maincpu, XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_PROGRAM, &rc2014mini_state::rc2014mini_mem);
	m_maincpu->set_addrmap(AS_IO, &rc2014mini_state::rc2014mini_io);

	// Configure UART (via m_acia)
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set_inputline("maincpu", INPUT_LINE_IRQ0); // Connect interrupt pin to our Z80 INT line

	// Create a clock device to connect to the transmit and receive clock on the 6850
	clock_device &acia_clock(CLOCK(config, "acia_clock", 7'372'800));
	acia_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));

	// Configure a "default terminal" to connect to the 6850, so we have a console
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(acia6850_device::write_dcd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be below the DEVICE_INPUT_DEFAULTS_START block
}

// ROM mapping is trivial, this binary was created from the K0000000.hex file
// from https://github.com/RC2014Z80/RC2014/tree/master/ROMs/Factory
// `dd skip=56 count=8 if=R0000009.BIN of=rc2014mini_scm.bin bs=1024`

ROM_START(rc2014mini)
	ROM_REGION(0x2000, "maincpu",0)
	ROM_LOAD("rc2014mini_scm.bin",   0x0000, 0x2000, CRC(e8745176) SHA1(d71afa985c4dcc25536b6597a099dabc815a8eb2))
ROM_END

// This ties everything together
//    YEAR  NAME            PARENT    COMPAT    MACHINE        INPUT    CLASS             INIT           COMPANY      FULLNAME                  FLAGS
COMP( 2015, rc2014mini,     0,        0,        rc2014mini,    0,       rc2014mini_state, empty_init,    "Z80Kits",   "RC2014 Mini SCM",        MACHINE_NO_SOUND_HW )
