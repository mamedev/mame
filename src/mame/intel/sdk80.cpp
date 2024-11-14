// license:BSD-3-Clause
// copyright-holders:Nigel Barnes,Ryan Holtz
/***************************************************************************

        Intel SDK-80

The 8080 System Design Kit (SDK-80) is a complete microcomputer system in
kit form, and provides a excellent prototype vehicle for evaluation of the
8080 microcomputer system (MCS-80).

An extensive system monitor is included in a pre-programmed ROM for general
software utilities and system diagnostics.

Download the User Manual to get the operating procedures.

Monitor Commands:
D<low address>,<high address>                Display memory contents
G<entry point>                               Go to address (execute program at address)
I<address>                                   Insert instructions into RAM
M<low address>,<high address>,<destination>  Move blocks of memory
S<address>                                   Substitute memory locations
X<register identifier>                       Examine and modify registers

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


namespace {

#define I8255A_0_TAG    "ppi8255_0"
#define I8255A_1_TAG    "ppi8255_1"
#define I8251A_TAG      "usart"
#define I8251A_BAUD_TAG "usart_baud"
#define RS232_TAG       "rs232"

class sdk80_state : public driver_device
{
public:
	sdk80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_usart(*this, I8251A_TAG)
		, m_ppi_0(*this, I8255A_0_TAG)
		, m_ppi_1(*this, I8255A_1_TAG)
		, m_rs232(*this, RS232_TAG)
		, m_usart_baud_rate(*this, I8251A_BAUD_TAG)
		, m_usart_divide_counter(0)
		, m_usart_clock_state(0)
	{ }

	void sdk80(machine_config &config);

private:
	void usart_clock_tick(int state);

	void sdk80_io(address_map &map) ATTR_COLD;
	void sdk80_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_usart;
	required_device<i8255_device> m_ppi_0;
	required_device<i8255_device> m_ppi_1;
	required_device<rs232_port_device> m_rs232;
	required_ioport m_usart_baud_rate;

	uint8_t m_usart_divide_counter;
	uint8_t m_usart_clock_state;
};

void sdk80_state::sdk80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x13ff).ram();
}

void sdk80_state::sdk80_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xec, 0xef).rw(m_ppi_1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf4, 0xf7).rw(m_ppi_0, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfa, 0xfb).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
}

static INPUT_PORTS_START( sdk80 )
	PORT_START(I8251A_BAUD_TAG)
	PORT_DIPNAME( 0x3f, 0x01, "i8251 Baud Rate" )
	PORT_DIPSETTING(    0x01, "4800")
	PORT_DIPSETTING(    0x02, "2400")
	PORT_DIPSETTING(    0x04, "1200")
	PORT_DIPSETTING(    0x08, "600")
	PORT_DIPSETTING(    0x10, "300")
	PORT_DIPSETTING(    0x20, "150")
	PORT_DIPSETTING(    0x40, "75")
INPUT_PORTS_END

void sdk80_state::usart_clock_tick(int state)
{
	uint8_t old_counter = m_usart_divide_counter;
	m_usart_divide_counter++;

	uint8_t transition = (old_counter ^ m_usart_divide_counter) & m_usart_baud_rate->read();
	if (transition)
	{
		m_usart->write_txc(m_usart_clock_state);
		m_usart->write_rxc(m_usart_clock_state);
		m_usart_clock_state ^= 1;
	}
}

static DEVICE_INPUT_DEFAULTS_START( terminal ) // set up terminal to default to 4800
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void sdk80_state::sdk80(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, XTAL(18'432'000)/9);
	m_maincpu->set_addrmap(AS_PROGRAM, &sdk80_state::sdk80_mem);
	m_maincpu->set_addrmap(AS_IO, &sdk80_state::sdk80_io);

	I8251(config, m_usart, 0);
	m_usart->txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	I8255A(config, m_ppi_0);
	I8255A(config, m_ppi_1);

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
	m_rs232->cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	clock_device &clock(CLOCK(config, "usart_clock", XTAL(18'432'000)/60));
	clock.signal_handler().set(FUNC(sdk80_state::usart_clock_tick));
}

/* ROM definition */
ROM_START( sdk80 )
	ROM_REGION( 0x01400, "maincpu", ROMREGION_ERASEFF )
	//ROM_LOAD( "mcs80.a14", 0x0000, 0x0400, BAD_DUMP CRC(3ce7bd37) SHA1(04cc67875b53d4cdfefce07041af12be3acedf4f)) // Compiled from manual listing
	ROM_LOAD( "mcs80.a14", 0x0000, 0x0400, BAD_DUMP CRC(9bb1c268) SHA1(e84e358f81f181f40f4f8d4c4f76370b7d82e615) ) // Compiled from corrected listing - see issue #6324
ROM_END

} // anonymous namespace


/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT        COMPANY   FULLNAME     FLAGS */
COMP( 1975, sdk80, 0,      0,      sdk80,   sdk80,  sdk80_state, empty_init, "Intel",  "SDK-80",    MACHINE_NO_SOUND_HW )
