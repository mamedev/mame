// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    CVICNY8080 - Practice-z80 - a homebrew from Czechoslavakia.

    More data at :
                http://www.nostalcomp.cz/cvicny8080.php

    21/OCT/2011 New working driver. [Robbbert]

    Keys:
        0-9,A-F : hexadecimal numbers
        ADR : enter an address to work with. After the 4 digits are entered,
              the data at that address shows, and you can modify the data.
        + (inc) : Enter the data into memory, and increment the address by 1.
        GO : execute the program located at the current address.

    Pasting:
        0-F : as is
        + (inc) : ^
        ADR : -
        GO : X

    Test Paste:
        11^22^33^44^55^66^77^88^99^-0800
        Now press up-arrow to confirm the data has been entered.

    ToDo:
        - When ADR is pressed, sometimes a segment stays alight. Bug?
        - Blank the display if digits aren't being refreshed

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cvicny.lh"

class cvicny_state : public driver_device
{
public:
	cvicny_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_digit_last(0)
	{ }

	required_device<cpu_device> m_maincpu;
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_WRITE8_MEMBER(segment_w );
	UINT8 m_digit;
	UINT8 m_digit_last;
};

WRITE8_MEMBER( cvicny_state::segment_w ) // output segments on the selected digit
{
	if (m_digit != m_digit_last)
		output_set_digit_value(m_digit, data);

	m_digit_last = m_digit;
}

WRITE8_MEMBER( cvicny_state::digit_w ) // set keyboard scanning row; set digit to display
{
	m_digit = data & 7;
}

READ8_MEMBER( cvicny_state::key_r )
{
	UINT8 data;
	char kbdrow[6];
	sprintf(kbdrow,"X%d",m_digit);
	data = ioport(kbdrow)->read();
	return ((data << 4) ^ 0xf0) | data;
}

static ADDRESS_MAP_START( cvicny_mem, AS_PROGRAM, 8, cvicny_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM // 1 x 2716
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_MIRROR(0x400) // 2x 2114 static ram
	AM_RANGE(0x1000, 0x17ff) AM_READ(key_r)
	AM_RANGE(0x1800, 0x1fff) AM_WRITE(digit_w)
	AM_RANGE(0x2000, 0x27ff) AM_WRITE(segment_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( cvicny )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ADR") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x0F, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6")
	PORT_BIT( 0x0F, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7")
	PORT_BIT( 0x0F, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static MACHINE_CONFIG_START( cvicny, cvicny_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(cvicny_mem)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_cvicny)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( cvicny )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("cvicny8080.bin", 0x0000, 0x05ea, CRC(e6119052) SHA1(d03c2cbfd047f0d090a787fbbde6353593cc2dd8) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY     FULLNAME       FLAGS */
COMP( 1984, cvicny, 0,      0,       cvicny,    cvicny, driver_device,  0,     "<unknown>", "Practice-z80", MACHINE_NO_SOUND_HW)
