// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Mera-Elzab Konin

It's an industrial computer used in Poland

No information has been found. All code is guesswork.

2011-12-29 Skeleton driver.
2016-07-15 Added terminal and uart.

'maincpu' (0384): unmapped i/o memory write to 00F8 = 56 & FF
'maincpu' (0388): unmapped i/o memory write to 00F8 = B6 & FF
'maincpu' (038C): unmapped i/o memory write to 0024 = 00 & FF
'maincpu' (0A0B): unmapped i/o memory write to 0080 = BE & FF
'maincpu' (0A0F): unmapped i/o memory write to 0080 = 08 & FF
'maincpu' (0A13): unmapped i/o memory write to 0080 = 0C & FF
'maincpu' (0A15): unmapped i/o memory read from 0082 & FF
'maincpu' (0A19): unmapped i/o memory write to 0080 = 05 & FF
'maincpu' (04DE): unmapped i/o memory write to 00F6 = 27 & FF
'maincpu' (04E2): unmapped i/o memory write to 00F6 = 40 & FF
'maincpu' (04E6): unmapped i/o memory write to 00F6 = CE & FF
'maincpu' (04EA): unmapped i/o memory write to 00F6 = 27 & FF
'maincpu' (043B): unmapped i/o memory write to 00F8 = B6 & FF
'maincpu' (043F): unmapped i/o memory write to 00F6 = 27 & FF
'maincpu' (2AA3): unmapped i/o memory write to 00F8 = 14 & FF
'maincpu' (2AA7): unmapped i/o memory write to 00FB = C0 & FF
'maincpu' (2AC2): unmapped i/o memory write to 00F8 = 56 & FF
'maincpu' (2AC6): unmapped i/o memory write to 00FA = 03 & FF
'maincpu' (0082): unmapped i/o memory write to 0024 = 06 & FF

Debug stuff:
- Start it up
- Write FF to 7D57 to see some messages
- Write 00 to 7D57 to silence it

Even though it gives an input prompt, there's no code to accept anything

Terminal settings: 8 data bits, 2 stop bits, no parity @ 9600

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"

class konin_state : public driver_device
{
public:
	konin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
	{ }

	DECLARE_WRITE_LINE_MEMBER(clock_w);

private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
};

static ADDRESS_MAP_START( konin_mem, AS_PROGRAM, 8, konin_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0x5000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( konin_io, AS_IO, 8, konin_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf6, 0xf6) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0xf7, 0xf7) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( konin )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( konin )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END


void konin_state::machine_reset()
{
}

WRITE_LINE_MEMBER( konin_state::clock_w )
{
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}

static MACHINE_CONFIG_START( konin, konin_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(konin_mem)
	MCFG_CPU_IO_MAP(konin_io)

	MCFG_DEVICE_ADD("uart", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart", i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", konin )

	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(konin_state, clock_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( konin )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "001.bin", 0x0000, 0x0800, CRC(0b13208a) SHA1(38ea17be591b729158d601c03bfd9954f32e0e67))
	ROM_LOAD( "008.bin", 0x0800, 0x0800, CRC(f003e407) SHA1(11f79ef3b90788cf627ee39705bbbd04dbf45f50))
	ROM_LOAD( "007.bin", 0x1000, 0x0800, CRC(3d390c03) SHA1(ac2fe31c065e8f630381d6cebd2eb58b403c1e02))
	ROM_LOAD( "006.bin", 0x1800, 0x0800, CRC(68c9732e) SHA1(f40a79719dca485a2db29be5c0c781f559c2551c))
	ROM_LOAD( "005.bin", 0x2000, 0x0800, CRC(14548ac4) SHA1(8987e528b3e479c4c5941366628f34f086d06838))
	ROM_LOAD( "004.bin", 0x2800, 0x0800, CRC(8a354cff) SHA1(24d9f1fb15458fc96f5265f79d54e030b68d9fc9))
	ROM_LOAD( "002.bin", 0x3000, 0x0800, CRC(791fb30d) SHA1(8dfbe0edb741e02cfdd138432999f89480b20471))
	ROM_LOAD( "003.bin", 0x3800, 0x0800, CRC(27dc9864) SHA1(0d3da7fd1db895883c106f5133f8c7228333ecc8))
	ROM_LOAD( "009.bin", 0x4000, 0x0800, CRC(80947d15) SHA1(0757fb191913d79f306874684f9fc082ce18a28e))
	ROM_LOAD( "010.bin", 0x4800, 0x0800, CRC(f0157e0c) SHA1(60ace1eaf0ba01a45987c2286e18f3d56441c994))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY       FULLNAME       FLAGS */
COMP( 198?, konin,  0,      0,       konin,     konin, driver_device,   0,    "Mera-Elzab",   "Konin", MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
