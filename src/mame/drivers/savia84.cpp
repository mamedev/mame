// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Savia 84

    More data at :
        http://www.nostalcomp.cz/pdfka/savia84.pdf
        http://www.nostalcomp.cz/savia.php

    05/02/2011 Skeleton driver.
    11/10/2011 Found a new rom. Working [Robbbert]

    I assume all the LEDs are red ones. The LEDs down the
    left side I assume to be bit 0 through 7 in that order.

    Pasting:
        0-F : as is
        DA : ^
        AD : -
        GO : X

    Here is a test program. Copy the text and Paste into the emulator.
    -1800^3E^55^D3^F9^76^XX1800^

    ToDo:
    - Make better artwork

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "savia84.lh"


class savia84_state : public driver_device
{
public:
	savia84_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_ppi8255(*this, "ppi8255")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi8255;
	DECLARE_READ8_MEMBER(savia84_8255_portc_r);
	DECLARE_WRITE8_MEMBER(savia84_8255_porta_w);
	DECLARE_WRITE8_MEMBER(savia84_8255_portb_w);
	DECLARE_WRITE8_MEMBER(savia84_8255_portc_w);
	UINT8 m_kbd;
	UINT8 m_segment;
	UINT8 m_digit;
	UINT8 m_digit_last;
	virtual void machine_reset() override;
};

static ADDRESS_MAP_START( savia84_mem, AS_PROGRAM, 8, savia84_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x7fff) // A15 not connected at the CPU
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x1800, 0x1fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( savia84_io, AS_IO, 8, savia84_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x07)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi8255", i8255_device, read, write) // ports F8-FB
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( savia84 )
	PORT_START("X0")
	PORT_BIT( 0x9F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("X1")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')

	PORT_START("X2")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')

	PORT_START("X3")
	PORT_BIT( 0x9F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Ex") PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("X4")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BR") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("X5")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AD") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("X6")
	PORT_BIT( 0x9F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')

	PORT_START("X7")
	PORT_BIT( 0x9F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')

	PORT_START("X8")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DA") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
INPUT_PORTS_END


void savia84_state::machine_reset()
{
	m_digit_last = 0;
}

WRITE8_MEMBER( savia84_state::savia84_8255_porta_w ) // OUT F8 - output segments on the selected digit
{
	m_segment = ~data & 0x7f;
	if (m_digit && (m_digit != m_digit_last)) output().set_digit_value(m_digit, m_segment);
	m_digit_last = m_digit;
}

WRITE8_MEMBER( savia84_state::savia84_8255_portb_w ) // OUT F9 - light the 8 leds down the left side
{
	char ledname[8];
	for (int i = 0; i < 8; i++)
	{
		sprintf(ledname,"led%d",i);
		output().set_value(ledname, !BIT(data, i));
	}
}

WRITE8_MEMBER( savia84_state::savia84_8255_portc_w ) // OUT FA - set keyboard scanning row; set digit to display
{
	m_digit = 0;
	m_kbd = data & 15;
	if (m_kbd == 0)
		m_digit = 1;
	else
	if ((m_kbd > 1) && (m_kbd < 9))
		m_digit = m_kbd;
}

READ8_MEMBER( savia84_state::savia84_8255_portc_r ) // IN FA - read keyboard
{
	if (m_kbd < 9)
	{
		char kbdrow[6];
		sprintf(kbdrow,"X%d",m_kbd);
		return ioport(kbdrow)->read();
	}
	else
		return 0xff;
}

static MACHINE_CONFIG_START( savia84, savia84_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz / 2)
	MCFG_CPU_PROGRAM_MAP(savia84_mem)
	MCFG_CPU_IO_MAP(savia84_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_savia84)

	/* Devices */
	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(savia84_state, savia84_8255_porta_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(savia84_state, savia84_8255_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(savia84_state, savia84_8255_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(savia84_state, savia84_8255_portc_w))

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( savia84 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("savia84.bin", 0x0000, 0x0800, CRC(fa8f1fcf) SHA1(b08404469ed988d96c0413416b6a66f3e5b997a3))

	// Note - the below is a bad dump and does not work
	//ROM_LOAD("savia84_1kb.bin", 0x0000, 0x0400, CRC(23a5c15e) SHA1(7e769ed8960d8c591a25cfe4effffcca3077c94b))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY     FULLNAME       FLAGS */
COMP( 1984, savia84,  0,       0,    savia84,   savia84, driver_device, 0,     "JT Hyan", "Savia 84", MACHINE_NO_SOUND_HW)
