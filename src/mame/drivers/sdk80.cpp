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
	DECLARE_WRITE_LINE_MEMBER( usart_clock_tick );

	void sdk80_io(address_map &map);
	void sdk80_mem(address_map &map);

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

WRITE_LINE_MEMBER( sdk80_state::usart_clock_tick )
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
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

MACHINE_CONFIG_START(sdk80_state::sdk80)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8080A, XTAL(18'432'000)/9)
	MCFG_DEVICE_PROGRAM_MAP(sdk80_mem)
	MCFG_DEVICE_IO_MAP(sdk80_io)

	I8251(config, m_usart, 0);
	m_usart->txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	MCFG_DEVICE_ADD(I8255A_0_TAG, I8255A, 0)
	MCFG_DEVICE_ADD(I8255A_1_TAG, I8255A, 0)

	MCFG_DEVICE_ADD(RS232_TAG, RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE(I8251A_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(WRITELINE(I8251A_TAG, i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(I8251A_TAG, i8251_device, write_cts))
	MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_DEVICE_ADD("usart_clock", CLOCK, XTAL(18'432'000)/60)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(*this, sdk80_state, usart_clock_tick))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sdk80 )
	ROM_REGION( 0x01400, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mcs80.a14", 0x0000, 0x0400, BAD_DUMP CRC(3ce7bd37) SHA1(04cc67875b53d4cdfefce07041af12be3acedf4f)) // Compiled from manual listing
ROM_END

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT        COMPANY   FULLNAME     FLAGS */
COMP( 1975, sdk80, 0,      0,      sdk80,   sdk80,  sdk80_state, empty_init, "Intel",  "SDK-80",    MACHINE_NO_SOUND_HW )
