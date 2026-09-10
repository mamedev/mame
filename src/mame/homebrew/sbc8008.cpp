// license:BSD-3-Clause copyright-holders:Jeremy English
//
// MAME driver for Jim Loos's 8008-SBC
// https://github.com/jim11662418/8008-SBC
//
// The SBC contains:
//
// 8008 CPU at 1 Mhz / 4
// 2 x G22V10 PLDs for logic
// Winbond W27E257 EEPROM
// 6264 RAM
// MAX233 for bit banged serial at 2400 bps N-8-1
// and a handful of TTL chips
//
// From the PLD file:
//    When START is high (on power-up and reset), ROMA14 and ROMA13 are low to
//    force the SBC to execute code in the first 8K segment of the EPROM. when
//    START is low, ROMA14 and ROMA13 are controlled by the Q3 and Q2 inputs from
//    port 10 allowing any one of the four 8K sections of the EPROM to be selected
//    for code.
//
// TODO get file transfer working for both hex and binary
// TODO what are outputs 1,2,3,4,5,6,7 doing?
// TODO test importing from other ports


#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8008/i8008.h"
#include "machine/ram.h"

#include "sbc8008.lh"

namespace
{

#define SBC8008_ROM_SIZE (0x2000)

// State class - derives from driver_device
class sbc8008_state : public driver_device
{
public:
	sbc8008_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		  , m_maincpu(*this, "maincpu")
		  , m_ram(*this, "ram")
		  , m_rom(*this, "rom")
		  , m_rom_bank(*this, "bank")
		  , m_rs232(*this, "rs232")
		  , m_leds(*this, "led%u", 0U)
		  , m_run_led(*this, "run_led")
		  , m_txd_led(*this, "txd_led")
		  , m_rxd_led(*this, "rxd_led")
		  , m_view(*this, "bootview")
{ }

	// This function sets up the machine configuration
	void sbc8008(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_bank m_rom_bank;
	required_device<rs232_port_device> m_rs232;
	output_finder<8> m_leds;
	output_finder<> m_run_led;
	output_finder<> m_txd_led;
	output_finder<> m_rxd_led;
	memory_view m_view;

	uint8_t bitbang_read();
	void bitbang_write(uint8_t data);
	uint8_t port_1_read();
	void port_9_write(uint8_t data);
	void port_10_write(uint8_t data);

	// address maps for program memory and io memory
	void sbc8008_mem(address_map &map);
	void sbc8008_io(address_map &map);

	virtual void machine_start() override ATTR_COLD;
};

void sbc8008_state::machine_start()
{
	m_leds.resolve();
	m_run_led.resolve();
	m_txd_led.resolve();
	m_rxd_led.resolve();

	m_run_led = 1;

	m_rom_bank->configure_entry(0, m_rom->base() + 0x2000);
	m_rom_bank->configure_entry(1, m_rom->base() + 0x4000);

	m_rom_bank->set_entry(0);
	m_view.select(0);
}

uint8_t sbc8008_state::bitbang_read()
{
	uint8_t result = m_rs232->rxd_r();
	m_rxd_led = BIT(result, 0);
	return result;
}

void sbc8008_state::bitbang_write(uint8_t data)
{
	m_txd_led = BIT(data, 0);
	m_rs232->write_txd(BIT(data, 0));
}

// Comment from the PLD file:
//
//   simulated SR flip-flop made up of cross-connected NAND gates.
//   the flip-flop is set when the reset signal from the DS1233 goes low
//   (power-on-reset) and cleared when input port 1 is accessed.
//   when set, the flip-flop forces all memory accesses to select the
//   EPROM. when reset, the flip-flop permits the normal memory map.

uint8_t sbc8008_state::port_1_read()
{
	m_view.select(1);
	return 0;
}

void sbc8008_state::port_9_write(uint8_t data)
{
	for(int i = 0; i < 8; i++)
	{
		m_leds[i] = BIT(data, 7-i);
	}
}

void sbc8008_state::port_10_write(uint8_t data)
{
	if (data < 2)
	{
		m_rom_bank->set_entry(data);
	}
}

void sbc8008_state::sbc8008_mem(address_map &map)
{
	// Comment from monitor.asm
	//
	//   when the reset pushbutton is pressed, the flip-flop is set which generates an interrupt
	//   and clears the address latches. thus, the first instruction is thus always fetched from
	//   address 0. the instruction at address 0 must be a single byte transfer instruction in
	//   order to set the program counter. i.e., it must be one of the RST opcodes.
	//

	map(0x0000, 0x3fff).view(m_view);
	m_view[0](0x0000, 0x1fff).bankr("bank").mirror(0x2000);
	m_view[1](0x0000, 0x1fff).ram();
	m_view[1](0x2000, 0x3fff).bankr("bank");
}

void sbc8008_state::sbc8008_io(address_map &map)
{
	// Description of IO ports from monitor.asm
	//
	// serial I/O at 2400 bps N-8-1
	//
	// INPORT      equ 0           ; serial input port address
	// OUTPORT     equ 08H         ; serial output port address
	//
	// out 10                      ; clear the EPROM bank switch address outputs A13 and A14
	// out 09                      ; turn off orange LEDs
	// out 08                      ; set serial output high (mark)
	// in 1                        ; reset the bootstrap flip-flop internal to GAL22V10 #2


	map.global_mask(0xff);  // use 8-bit ports
	map.unmap_value_high(); // unmapped addresses return 0xff
	map(0x00, 0x00).r(FUNC(sbc8008_state::bitbang_read));
	map(0x01, 0x01).r(FUNC(sbc8008_state::port_1_read));
	map(0x08, 0x08).w(FUNC(sbc8008_state::bitbang_write));
	map(0x09, 0x09).w(FUNC(sbc8008_state::port_9_write));
	map(0x0a, 0x0a).w(FUNC(sbc8008_state::port_10_write));
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
	DEVICE_INPUT_DEFAULTS( "TERM_CONF", 0x080, 0x080 ) // Auto LF on CR
DEVICE_INPUT_DEFAULTS_END

void sbc8008_state::sbc8008(machine_config &config)
{
	// The original 1 Mhz crystal goes to a flip-flop which divides by two and then that feeds a PLD which
	// produces phase 1 and 2 signals.  Simluating the logic gives me this timing diagram.  Dividing by 4 provides
	// the correct timing for the monitor to bitbang rs232 successfully.
	//
	//
	//         +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+
	//         |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
	// clk   : +  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--
	//                  +--+        +--+        +--+        +--+
	//                  |  |        |  |        |  |        |  |
	// phase1: ---------+  +--------+  +--------+  +--------+  +-----
	//            +--+        +--+        +--+        +--+        +--
	//            |  |        |  |        |  |        |  |        |
	// phase2: ---+  +--------+  +--------+  +--------+  +--------+


	I8008(config, m_maincpu, XTAL(1'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &sbc8008_state::sbc8008_mem);
	m_maincpu->set_addrmap(AS_IO, &sbc8008_state::sbc8008_io);

	config.set_default_layout(layout_sbc8008);

	// To provide a console, configure a "default terminal" to connect to the serial port
	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	// must be below the DEVICE_INPUT_DEFAULTS_START block
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	RAM(config, m_ram).set_default_size("8K");
}

// These are the ROM files from Jim Loos's github page
//
// The combinded eprom is not used since we can load the monitor and basic to different areas using the mapper.
//
// 067c016c
// a9b87bc2322404ecbb97842a4fc74c07c9b5535f  eprom.bin
//
// 0f3aa663
// 27679a370b45050b504a2c8f640d20e39afd78d6  monitor.bin
//
// 3d25b65a
// e1db1ba610ed0103d142f889a1995a5d95883c79  scelbal-in-eprom.bin

ROM_START(sbc8008)
	//For the addresses to makes since, setup a huge rom chip and load the roms to the cooresponding machine addresses
	ROM_REGION(0x10000, "rom",0)
	//         Name                   offset  Length   hash
	ROM_LOAD("monitor.bin",          0x2000, SBC8008_ROM_SIZE, CRC(0f3aa663) SHA1(27679a370b45050b504a2c8f640d20e39afd78d6))
	ROM_LOAD("scelbal-in-eprom.bin", 0x4000, SBC8008_ROM_SIZE, CRC(3d25b65a) SHA1(e1db1ba610ed0103d142f889a1995a5d95883c79))
ROM_END

} // anonymous namespace


// This ties everything together
//    YEAR  NAME            PARENT    COMPAT    MACHINE        INPUT          CLASS             INIT           COMPANY           FULLNAME                FLAGS
COMP( 2024, sbc8008,          0,        0,        sbc8008,         0,             sbc8008_state,      empty_init,    "Jim Loos",   "8008-SBC",  MACHINE_NO_SOUND_HW )
