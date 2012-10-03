/*

Sega SG-1000

PCB Layout
----------

171-5078 (C) SEGA 1983
171-5046 REV. A (C) SEGA 1983

|---------------------------|                              |----------------------------|
|   SW1     CN2             |   |------|---------------|   |    SW2     CN4             |
|                           |---|         CN3          |---|                            |
|  CN1                                                                              CN5 |
|                                                                                       |
|   10.738635MHz            |------------------------------|                7805        |
|   |---|                   |------------------------------|                            |
|   |   |                                 CN6                                           |
|   | 9 |                                                                               |
|   | 9 |                                                                       LS32    |
|   | 1 |       |---------|                                                             |
|   | 8 |       | TMM2009 |                                                     LS139   |
|   | A |       |---------|             |------------------|                            |
|   |   |                               |       Z80        |                            |
|   |---|                               |------------------|                            |
|                                                                                       |
|                                                                                       |
|       MB8118  MB8118  MB8118  MB8118              SN76489A            SW3             |
|           MB8118  MB8118  MB8118  MB8118                          LS257   LS257       |
|---------------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    Z80     - NEC D780C-1 / Zilog Z8400A (REV.A) Z80A CPU @ 3.579545
    TMS9918A- Texas Instruments TMS9918ANL Video Display Processor @ 10.738635MHz
    MB8118  - Fujitsu MB8118-12 16K x 1 Dynamic RAM
    TMM2009 - Toshiba TMM2009P-A / TMM2009P-B (REV.A)
    SN76489A- Texas Instruments SN76489AN Digital Complex Sound Generator @ 3.579545
    CN1     - player 1 joystick connector
    CN2     - RF video connector
    CN3     - keyboard connector
    CN4     - power connector (+9VDC)
    CN5     - player 2 joystick connector
    CN6     - cartridge connector
    SW1     - TV channel select switch
    SW2     - power switch
    SW3     - hold switch

*/

/*

    TODO:

    - slot interface for cartridges
    - SC-3000 return instruction referenced by R when reading ports 60-7f,e0-ff
    - connect PSG /READY signal to Z80 WAIT
    - accurate video timing
    - SP-400 serial printer
    - SH-400 racing controller
    - SF-7000 serial comms

*/


#include "includes/sg1000.h"



/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

/*

    Terebi Oekaki (TV Draw)

    Address Access  Bits
                    7       6   5   4   3   2   1   0
    $6000   W       -       -   -   -   -   -   -   AXIS
    $8000   R       BUSY    -   -   -   -   -   -   PRESS
    $A000   R/W     DATA

    AXIS: write 0 to select X axis, 1 to select Y axis.
    BUSY: reads 1 when graphic board is busy sampling position, else 0.
    PRESS: reads 0 when pen is touching graphic board, else 1.
    DATA: when pen is touching graphic board, return 8-bit sample position for currently selected axis (X is in the 0-255 range, Y in the 0-191 range). Else, return 0.

*/

/*-------------------------------------------------
    tvdraw_axis_w - TV Draw axis select
-------------------------------------------------*/

WRITE8_MEMBER( sg1000_state::tvdraw_axis_w )
{
	if (data & 0x01)
	{
		m_tvdraw_data = ioport("TVDRAW_X")->read();

		if (m_tvdraw_data < 4) m_tvdraw_data = 4;
		if (m_tvdraw_data > 251) m_tvdraw_data = 251;
	}
	else
	{
		m_tvdraw_data = ioport("TVDRAW_Y")->read() + 32;
	}
}

/*-------------------------------------------------
    tvdraw_status_r - TV Draw status read
-------------------------------------------------*/

READ8_MEMBER( sg1000_state::tvdraw_status_r )
{
	return ioport("TVDRAW_PEN")->read();
}

/*-------------------------------------------------
    tvdraw_data_r - TV Draw data read
-------------------------------------------------*/

READ8_MEMBER( sg1000_state::tvdraw_data_r )
{
	return m_tvdraw_data;
}

/*-------------------------------------------------
    joysel_r -
-------------------------------------------------*/

READ8_MEMBER( sg1000_state::joysel_r )
{
	return 0x80;
}

/*-------------------------------------------------
    ADDRESS_MAP( sg1000_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( sg1000_map, AS_PROGRAM, 8, sg1000_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank2")
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( sg1000_io_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( sg1000_io_map, AS_IO, 8, sg1000_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_DEVWRITE(SN76489AN_TAG, sn76489a_device, write)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3e) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, vram_read, vram_write)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x3e) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, register_read, register_write)
	AM_RANGE(0xdc, 0xdc) AM_READ_PORT("PA7")
	AM_RANGE(0xdd, 0xdd) AM_READ_PORT("PB7")
	AM_RANGE(0xde, 0xde) AM_READ(joysel_r) AM_WRITENOP
	AM_RANGE(0xdf, 0xdf) AM_NOP
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( omv_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( omv_map, AS_PROGRAM, 8, sg1000_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_MIRROR(0x3800) AM_RAM
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( omv_io_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( omv_io_map, AS_IO, 8, sg1000_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_DEVWRITE(SN76489AN_TAG, sn76489a_device, write)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3e) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, vram_read, vram_write)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x3e) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x38) AM_READ_PORT("C0")
	AM_RANGE(0xc1, 0xc1) AM_MIRROR(0x38) AM_READ_PORT("C1")
	AM_RANGE(0xc2, 0xc2) AM_MIRROR(0x38) AM_READ_PORT("C2")
	AM_RANGE(0xc3, 0xc3) AM_MIRROR(0x38) AM_READ_PORT("C3")
	AM_RANGE(0xc4, 0xc4) AM_MIRROR(0x3a) AM_READ_PORT("C4")
	AM_RANGE(0xc5, 0xc5) AM_MIRROR(0x3a) AM_READ_PORT("C5")
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( sc3000_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( sc3000_map, AS_PROGRAM, 8, sg1000_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank2")
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( sc3000_io_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( sc3000_io_map, AS_IO, 8, sg1000_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x7f, 0x7f) AM_DEVWRITE(SN76489AN_TAG, sn76489a_device, write)
	AM_RANGE(0xbe, 0xbe) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, vram_read, vram_write)
	AM_RANGE(0xbf, 0xbf) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, register_read, register_write)
	AM_RANGE(0xdc, 0xdf) AM_DEVREADWRITE(UPD9255_TAG, i8255_device, read, write)
ADDRESS_MAP_END

/* This is how the I/O ports are really mapped, but MAME does not support overlapping ranges
static ADDRESS_MAP_START( sc3000_io_map, AS_IO, 8, sg1000_state )
    ADDRESS_MAP_GLOBAL_MASK(0xff)
    AM_RANGE(0x00, 0x00) AM_MIRROR(0xdf) AM_DEVREADWRITE(UPD9255_TAG, i8255_device, read, write)
    AM_RANGE(0x00, 0x00) AM_MIRROR(0x7f) AM_DEVWRITE(SN76489AN_TAG, sn76489a_device, write)
    AM_RANGE(0x00, 0x00) AM_MIRROR(0xae) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, vram_read, vram_write)
    AM_RANGE(0x01, 0x01) AM_MIRROR(0xae) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, register_read, register_write)
    AM_RANGE(0x60, 0x60) AM_MIRROR(0x9f) AM_READ(sc3000_r_r)
ADDRESS_MAP_END
*/

/*-------------------------------------------------
    ADDRESS_MAP( sf7000_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( sf7000_map, AS_PROGRAM, 8, sf7000_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2")
	AM_RANGE(0x4000, 0xffff) AM_RAM
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( sf7000_io_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( sf7000_io_map, AS_IO, 8, sf7000_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x7f, 0x7f) AM_DEVWRITE(SN76489AN_TAG, sn76489a_device, write)
	AM_RANGE(0xbe, 0xbe) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, vram_read, vram_write)
	AM_RANGE(0xbf, 0xbf) AM_DEVREADWRITE(TMS9918A_TAG, tms9918a_device, register_read, register_write)
	AM_RANGE(0xdc, 0xdf) AM_DEVREADWRITE(UPD9255_0_TAG, i8255_device, read, write)
	AM_RANGE(0xe0, 0xe0) AM_DEVREAD_LEGACY(UPD765_TAG, upd765_status_r)
	AM_RANGE(0xe1, 0xe1) AM_DEVREADWRITE_LEGACY(UPD765_TAG, upd765_data_r, upd765_data_w)
	AM_RANGE(0xe4, 0xe7) AM_DEVREADWRITE(UPD9255_1_TAG, i8255_device, read, write)
	AM_RANGE(0xe8, 0xe8) AM_DEVREADWRITE(UPD8251_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0xe9, 0xe9) AM_DEVREADWRITE(UPD8251_TAG, i8251_device, status_r, control_w)
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_CHANGED_MEMBER( trigger_nmi )
-------------------------------------------------*/

INPUT_CHANGED_MEMBER( sg1000_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

/*-------------------------------------------------
    INPUT_PORTS( tvdraw )
-------------------------------------------------*/

static INPUT_PORTS_START( tvdraw )
	PORT_START("TVDRAW_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("TVDRAW_Y")
	PORT_BIT( 0xff, 0x60, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0, 191) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("TVDRAW_PEN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Pen")
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( sg1000 )
-------------------------------------------------*/

static INPUT_PORTS_START( sg1000 )
	PORT_START("PA7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)

	PORT_START("PB7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("PAUSE") PORT_CODE(KEYCODE_P) PORT_CHANGED_MEMBER(DEVICE_SELF, sg1000_state, trigger_nmi, 0)

	PORT_INCLUDE( tvdraw )
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( omv )
-------------------------------------------------*/

static INPUT_PORTS_START( omv )
	PORT_START("C0")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C1")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9 #") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0 *") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("S-1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("S-2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)

	PORT_START("C5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( tvdraw )
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( sk1100 )
-------------------------------------------------*/

INPUT_PORTS_START( sk1100 )
	PORT_START("PA0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENG DIER'S") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("PA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPC") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("PA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME CLR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')

	PORT_START("PA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xcf\x80") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(0x03c0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')

	PORT_START("PA4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')

	PORT_START("PA5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)

	PORT_START("PB0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('^')
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xc2\xa5") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x00a5)
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNC") PORT_CODE(KEYCODE_TAB)

	PORT_START("PB6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("PB7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, sg1000_state, trigger_nmi, 0)
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( sc3000 )
-------------------------------------------------*/

static INPUT_PORTS_START( sc3000 )
	PORT_INCLUDE( sk1100 )
	PORT_INCLUDE( tvdraw )
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( sf7000 )
-------------------------------------------------*/

static INPUT_PORTS_START( sf7000 )
	PORT_INCLUDE( sk1100 )

	PORT_START("BAUD")
	PORT_CONFNAME( 0x05, 0x05, "Baud rate")
	PORT_CONFSETTING( 0x00, "9600 baud" )
	PORT_CONFSETTING( 0x01, "4800 baud" )
	PORT_CONFSETTING( 0x02, "2400 baud" )
	PORT_CONFSETTING( 0x03, "1200 baud" )
	PORT_CONFSETTING( 0x04, "600 baud" )
	PORT_CONFSETTING( 0x05, "300 baud" )
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    TMS9928a_interface tms9928a_interface
-------------------------------------------------*/

WRITE_LINE_MEMBER(sg1000_state::sg1000_vdp_interrupt)
{
	machine().device(Z80_TAG)->execute().set_input_line(INPUT_LINE_IRQ0, state);
}

static TMS9928A_INTERFACE(sg1000_tms9918a_interface)
{
	"screen",
	0x4000,
	DEVCB_DRIVER_LINE_MEMBER(sg1000_state,sg1000_vdp_interrupt)
};

/*-------------------------------------------------
    I8255_INTERFACE( sc3000_ppi_intf )
-------------------------------------------------*/

READ8_MEMBER( sc3000_state::ppi_pa_r )
{
	/*
        Signal  Description

        PA0     Keyboard input
        PA1     Keyboard input
        PA2     Keyboard input
        PA3     Keyboard input
        PA4     Keyboard input
        PA5     Keyboard input
        PA6     Keyboard input
        PA7     Keyboard input
    */

	static const char *const keynames[] = { "PA0", "PA1", "PA2", "PA3", "PA4", "PA5", "PA6", "PA7" };

	return ioport(keynames[m_keylatch])->read();
}

READ8_MEMBER( sc3000_state::ppi_pb_r )
{
	/*
        Signal  Description

        PB0     Keyboard input
        PB1     Keyboard input
        PB2     Keyboard input
        PB3     Keyboard input
        PB4     /CONT input from cartridge terminal B-11
        PB5     FAULT input from printer
        PB6     BUSY input from printer
        PB7     Cassette tape input
    */

	static const char *const keynames[] = { "PB0", "PB1", "PB2", "PB3", "PB4", "PB5", "PB6", "PB7" };

	/* keyboard */
	UINT8 data = ioport(keynames[m_keylatch])->read();

	/* cartridge contact */
	data |= 0x10;

	/* printer */
	data |= 0x60;

	/* tape input */
	if ((m_cassette)->input() > +0.0) data |= 0x80;

	return data;
}

WRITE8_MEMBER( sc3000_state::ppi_pc_w )
{
	/*
        Signal  Description

        PC0     Keyboard raster output
        PC1     Keyboard raster output
        PC2     Keyboard raster output
        PC3     not connected
        PC4     Cassette tape output
        PC5     DATA to printer
        PC6     /RESET to printer
        PC7     /FEED to printer
    */

	/* keyboard */
	m_keylatch = data & 0x07;

	/* cassette */
	m_cassette->output( BIT(data, 4) ? +1.0 : -1.0);

	/* TODO printer */
}

I8255_INTERFACE( sc3000_ppi_intf )
{
	DEVCB_DRIVER_MEMBER(sc3000_state, ppi_pa_r),	// Port A read
	DEVCB_NULL,										// Port A write
	DEVCB_DRIVER_MEMBER(sc3000_state, ppi_pb_r),	// Port B read
	DEVCB_NULL,										// Port B write
	DEVCB_NULL,										// Port C read
	DEVCB_DRIVER_MEMBER(sc3000_state, ppi_pc_w),	// Port C write
};

/*-------------------------------------------------
    cassette_interface sc3000_cassette_interface
-------------------------------------------------*/

const cassette_interface sc3000_cassette_interface =
{
	sc3000_cassette_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED),
	NULL,
	NULL
};

/***************************************************************************
    CARTRIDGE LOADING
***************************************************************************/

/*-------------------------------------------------
    sg1000_install_cartridge -
-------------------------------------------------*/

void sg1000_state::install_cartridge(UINT8 *ptr, int size)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	switch (size)
	{
	case 40 * 1024:
		program.install_read_bank(0x8000, 0x9fff, "bank1");
		program.unmap_write(0x8000, 0x9fff);
		membank("bank1")->configure_entry(0, memregion(Z80_TAG)->base() + 0x8000);
		membank("bank1")->set_entry(0);
		break;

	case 48 * 1024:
		program.install_read_bank(0x8000, 0xbfff, "bank1");
		program.unmap_write(0x8000, 0xbfff);
		membank("bank1")->configure_entry(0, memregion(Z80_TAG)->base() + 0x8000);
		membank("bank1")->set_entry(0);
		break;

	default:
		if (IS_CARTRIDGE_TV_DRAW(ptr))
		{
			program.install_write_handler(0x6000, 0x6000, 0, 0, write8_delegate(FUNC(sg1000_state::tvdraw_axis_w), this), 0);
			program.install_read_handler(0x8000, 0x8000, 0, 0, read8_delegate(FUNC(sg1000_state::tvdraw_status_r), this), 0);
			program.install_read_handler(0xa000, 0xa000, 0, 0, read8_delegate(FUNC(sg1000_state::tvdraw_data_r), this), 0);
			program.nop_write(0xa000, 0xa000);
		}
		else if (IS_CARTRIDGE_THE_CASTLE(ptr))
		{
			program.install_readwrite_bank(0x8000, 0x9fff, "bank1");
		}
		break;
	}
}

/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( sg1000_cart )
-------------------------------------------------*/

static DEVICE_IMAGE_LOAD( sg1000_cart )
{
	running_machine &machine = image.device().machine();
	sg1000_state *state = machine.driver_data<sg1000_state>();
	address_space &program = machine.device(Z80_TAG)->memory().space(AS_PROGRAM);
	UINT8 *ptr = state->memregion(Z80_TAG)->base();
	UINT32 ram_size = 0x400;
	bool install_2000_ram = false;
	UINT32 size;

	if (image.software_entry() == NULL)
	{
		size = image.length();
		if (image.fread( ptr, size) != size)
			return IMAGE_INIT_FAIL;
	}
	else
	{
		size = image.get_software_region_length("rom");
		memcpy(ptr, image.get_software_region("rom"), size);

		const char *needs_addon = image.get_feature("needs_addon");

		//
		// The Dahjee (Type A) RAM cartridge had 9KB of RAM. 1KB replaces
		// the main unit's system ram (0xC000-0xC3FF) and 8K which appears
		// at 0x2000-0x3FFF in the memory map.
		//
		if ( needs_addon && ! strcmp( needs_addon, "dahjee_type_a" ) )
		{
			install_2000_ram = true;
		}

		//
		// The Dahjee (Type B) RAM cartridge had 8KB of RAM which
		// replaces the main unit RAM in the memory map. (0xC000-0xDFFF area)
		//
		if ( needs_addon && ! strcmp( needs_addon, "dahjee_type_b" ) )
		{
			ram_size = 0x2000;
		}
	}

	// Try to auto-detect special features
	if ( ! install_2000_ram && ram_size == 0x400 )
	{
		if ( size >= 0x8000 )
		{
			int x2000_3000 = 0, xd000_e000_f000 = 0, x2000_ff = 0;

			for ( int i = 0; i < 0x8000; i++ )
			{
				if ( ptr[i] == 0x32 )
				{
					UINT16 addr = ptr[i+1] | ( ptr[i+2] << 8 );

					switch ( addr & 0xF000 )
					{
					case 0x2000:
					case 0x3000:
						i += 2;
						x2000_3000++;
						break;

					case 0xD000:
					case 0xE000:
					case 0xF000:
						i += 2;
						xd000_e000_f000++;
						break;
					}
				}
			}
			for ( int i = 0x2000; i < 0x4000; i++ )
			{
				if ( ptr[i] == 0xFF )
				{
					x2000_ff++;
				}
			}
			if ( x2000_ff == 0x2000 && ( xd000_e000_f000 > 10 || x2000_3000 > 10 ) )
			{
				if ( xd000_e000_f000 > x2000_3000 )
				{
					// Type B
					ram_size = 0x2000;
				}
				else
				{
					// Type A
					install_2000_ram = true;
				}
			}
		}
	}

	/* cartridge ROM banking */
	state->install_cartridge(ptr, size);

	if ( install_2000_ram )
	{
		program.install_ram(0x2000, 0x3FFF);
	}

	/* work RAM banking */
	program.install_readwrite_bank(0xc000, 0xc000 + ram_size - 1, 0, 0x4000 - ram_size, "bank2");

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( omv_cart )
-------------------------------------------------*/

static DEVICE_IMAGE_LOAD( omv_cart )
{
	running_machine &machine = image.device().machine();
	sg1000_state *state = machine.driver_data<sg1000_state>();
	UINT32 size;
	UINT8 *ptr = state->memregion(Z80_TAG)->base();

	if (image.software_entry() == NULL)
	{
		size = image.length();
		if (image.fread( ptr, size) != size)
			return IMAGE_INIT_FAIL;
	}
	else
	{
		size = image.get_software_region_length("rom");
		memcpy(ptr, image.get_software_region("rom"), size);
	}

	/* cartridge ROM banking */
	state->install_cartridge(ptr, size);

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
    sc3000_install_cartridge -
-------------------------------------------------*/

void sc3000_state::install_cartridge(UINT8 *ptr, int size)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* include SG-1000 mapping */
	sg1000_state::install_cartridge(ptr, size);

	if (IS_CARTRIDGE_BASIC_LEVEL_III(ptr))
	{
		program.install_readwrite_bank(0x8000, 0xbfff, "bank1");
		program.install_readwrite_bank(0xc000, 0xffff, "bank2");
	}
	else if (IS_CARTRIDGE_MUSIC_EDITOR(ptr))
	{
		program.install_readwrite_bank(0x8000, 0x9fff, "bank1");
		program.install_readwrite_bank(0xc000, 0xc7ff, 0, 0x3800, "bank2");
	}
	else
	{
		/* regular cartridges */
		program.install_readwrite_bank(0xc000, 0xc7ff, 0, 0x3800, "bank2");
	}
}

/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( sc3000_cart )
-------------------------------------------------*/

static DEVICE_IMAGE_LOAD( sc3000_cart )
{
	running_machine &machine = image.device().machine();
	sc3000_state *state = machine.driver_data<sc3000_state>();
	UINT8 *ptr = state->memregion(Z80_TAG)->base();
	UINT32 size;

	if (image.software_entry() == NULL)
	{
		size = image.length();
		if (image.fread( ptr, size) != size)
			return IMAGE_INIT_FAIL;
	}
	else
	{
		size = image.get_software_region_length("rom");
		memcpy(ptr, image.get_software_region("rom"), size);
	}

	/* cartridge ROM and work RAM banking */
	state->install_cartridge(ptr, size);

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
    I8255_INTERFACE( sf7000_ppi_intf )
-------------------------------------------------*/

READ8_MEMBER( sf7000_state::ppi_pa_r )
{
	/*
        Signal  Description

        PA0     INT from FDC
        PA1     BUSY from Centronics printer
        PA2     INDEX from FDD
        PA3
        PA4
        PA5
        PA6
        PA7
    */

	UINT8 data = 0;

	data |= m_fdc_irq;
	data |= m_centronics->busy_r() << 1;
	data |= m_fdc_index << 2;

	return data;
}

WRITE8_MEMBER( sf7000_state::ppi_pc_w )
{
	/*
        Signal  Description

        PC0     /INUSE signal to FDD
        PC1     /MOTOR ON signal to FDD
        PC2     TC signal to FDC
        PC3     RESET signal to FDC
        PC4     not connected
        PC5     not connected
        PC6     /ROM SEL (switch between IPL ROM and RAM)
        PC7     /STROBE to Centronics printer
    */

	/* floppy motor */
	floppy_mon_w(m_floppy0, BIT(data, 1));
	floppy_drive_set_ready_state(m_floppy0, 1, 1);

	/* FDC terminal count */
	upd765_tc_w(m_fdc, BIT(data, 2));

	/* FDC reset */
	if (BIT(data, 3))
	{
		upd765_reset(m_fdc, 0);
	}

	/* ROM selection */
	membank("bank1")->set_entry(BIT(data, 6));

	/* printer strobe */
	m_centronics->strobe_w(BIT(data, 7));
}

static I8255_INTERFACE( sf7000_ppi_intf )
{
	DEVCB_DRIVER_MEMBER(sf7000_state, ppi_pa_r),				// Port A read
	DEVCB_NULL,													// Port A write
	DEVCB_NULL,													// Port B read
	DEVCB_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, write),	// Port B write
	DEVCB_NULL,													// Port C read
	DEVCB_DRIVER_MEMBER(sf7000_state, ppi_pc_w)					// Port C write
};

/*-------------------------------------------------
    upd765_interface sf7000_upd765_interface
-------------------------------------------------*/

WRITE_LINE_MEMBER( sf7000_state::fdc_intrq_w )
{
	m_fdc_irq = state;
}

static const struct upd765_interface sf7000_upd765_interface =
{
	DEVCB_DRIVER_LINE_MEMBER(sf7000_state, fdc_intrq_w),
	DEVCB_NULL,
	NULL,
	UPD765_RDY_PIN_CONNECTED,
	{ FLOPPY_0, NULL, NULL, NULL }
};

/*-------------------------------------------------
    LEGACY_FLOPPY_OPTIONS( sf7000 )
-------------------------------------------------*/

static LEGACY_FLOPPY_OPTIONS_START( sf7000 )
	LEGACY_FLOPPY_OPTION(sf7000, "sf7", "SF7 disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

/*-------------------------------------------------
    sf7000_fdc_index_callback -
-------------------------------------------------*/

WRITE_LINE_MEMBER(sf7000_state::sf7000_fdc_index_callback)
{
	m_fdc_index = state;
}

/*-------------------------------------------------
    floppy_interface sf7000_floppy_interface
-------------------------------------------------*/

static const floppy_interface sf7000_floppy_interface =
{
	DEVCB_DRIVER_LINE_MEMBER(sf7000_state,sf7000_fdc_index_callback),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(sf7000),
	"floppy_3",
	NULL
};

/*-------------------------------------------------
    sn76496_config psg_intf
-------------------------------------------------*/

static const sn76496_config psg_intf =
{
	DEVCB_NULL
};

/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    TIMER_CALLBACK_MEMBER( lightgun_tick )
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER(sg1000_state::lightgun_tick)
{
	UINT8 *rom = machine().root_device().memregion(Z80_TAG)->base();

	if (IS_CARTRIDGE_TV_DRAW(rom))
	{
		/* enable crosshair for TV Draw */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable crosshair for other cartridges */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_NONE);
	}
}

/*-------------------------------------------------
    MACHINE_START( sg1000 )
-------------------------------------------------*/

void sg1000_state::machine_start()
{
	/* toggle light gun crosshair */
	machine().scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(sg1000_state::lightgun_tick),this));

	/* register for state saving */
	save_item(NAME(m_tvdraw_data));
}

/*-------------------------------------------------
    MACHINE_START( sc3000 )
-------------------------------------------------*/

void sc3000_state::machine_start()
{
	/* toggle light gun crosshair */
	machine().scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(sg1000_state::lightgun_tick),this));

	/* register for state saving */
	save_item(NAME(m_tvdraw_data));
	save_item(NAME(m_keylatch));
}


/*-------------------------------------------------
    MACHINE_START( sf7000 )
-------------------------------------------------*/

void sf7000_state::machine_start()
{
	/* configure memory banking */
	membank("bank1")->configure_entry(0, memregion(Z80_TAG)->base());
	membank("bank1")->configure_entry(1, m_ram->pointer());
	membank("bank2")->configure_entry(0, m_ram->pointer());

	/* register for state saving */
	save_item(NAME(m_keylatch));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_index));
}

/*-------------------------------------------------
    MACHINE_RESET( sf7000 )
-------------------------------------------------*/

void sf7000_state::machine_reset()
{
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/*-------------------------------------------------
    MACHINE_CONFIG_START( sg1000, sg1000_state )
-------------------------------------------------*/

static MACHINE_CONFIG_START( sg1000, sg1000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_10_738635MHz/3)
	MCFG_CPU_PROGRAM_MAP(sg1000_map)
	MCFG_CPU_IO_MAP(sg1000_io_map)

    /* video hardware */
	MCFG_TMS9928A_ADD( TMS9918A_TAG, TMS9918A, sg1000_tms9918a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9918A_TAG, tms9918a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489AN_TAG, SN76489A, XTAL_10_738635MHz/3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_CONFIG(psg_intf)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("sg,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("sg1000_cart")
	MCFG_CARTSLOT_LOAD(sg1000_cart)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","sg1000")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1K")
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_DERIVED( omv, sg1000 )
-------------------------------------------------*/

static MACHINE_CONFIG_DERIVED( omv, sg1000 )
	MCFG_CPU_MODIFY(Z80_TAG)
	MCFG_CPU_PROGRAM_MAP(omv_map)
	MCFG_CPU_IO_MAP(omv_io_map)

	MCFG_CARTSLOT_MODIFY("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("sg,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(omv_cart)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2K")
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_START( sc3000, sc3000_state )
-------------------------------------------------*/

static MACHINE_CONFIG_START( sc3000, sc3000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_10_738635MHz/3) // LH0080A
	MCFG_CPU_PROGRAM_MAP(sc3000_map)
	MCFG_CPU_IO_MAP(sc3000_io_map)

    /* video hardware */
	MCFG_TMS9928A_ADD( TMS9918A_TAG, TMS9918A, sg1000_tms9918a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9918A_TAG, tms9918a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489AN_TAG, SN76489A, XTAL_10_738635MHz/3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_CONFIG(psg_intf)

	/* devices */
	MCFG_I8255_ADD(UPD9255_TAG, sc3000_ppi_intf)
//  MCFG_PRINTER_ADD("sp400") /* serial printer */
	MCFG_CASSETTE_ADD(CASSETTE_TAG, sc3000_cassette_interface)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("sg,sc,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("sg1000_cart")
	MCFG_CARTSLOT_LOAD(sc3000_cart)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","sg1000")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2K")
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_START( sf7000, sf7000_state )
-------------------------------------------------*/

static MACHINE_CONFIG_START( sf7000, sf7000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_10_738635MHz/3)
	MCFG_CPU_PROGRAM_MAP(sf7000_map)
	MCFG_CPU_IO_MAP(sf7000_io_map)

    /* video hardware */
	MCFG_TMS9928A_ADD( TMS9918A_TAG, TMS9918A, sg1000_tms9918a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9918A_TAG, tms9918a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489AN_TAG, SN76489A, XTAL_10_738635MHz/3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_CONFIG(psg_intf)

	/* devices */
	MCFG_I8255_ADD(UPD9255_0_TAG, sc3000_ppi_intf)
	MCFG_I8255_ADD(UPD9255_1_TAG, sf7000_ppi_intf)
	MCFG_I8251_ADD(UPD8251_TAG, default_i8251_interface)
	MCFG_UPD765A_ADD(UPD765_TAG, sf7000_upd765_interface)
	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, sf7000_floppy_interface)
//  MCFG_PRINTER_ADD("sp400") /* serial printer */
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, standard_centronics)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, sc3000_cassette_interface)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_list","sf7000")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( sg1000 )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASE00 )
ROM_END

#define rom_sg1000m2 rom_sg1000

#define rom_sc3000 rom_sg1000

#define rom_sc3000h rom_sg1000

ROM_START( sf7000 )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASE00 )
	ROM_LOAD( "ipl.rom", 0x0000, 0x2000, CRC(d76810b8) SHA1(77339a6db2593aadc638bed77b8e9bed5d9d87e3) )
ROM_END

ROM_START( omv1000 )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASE00 )
	ROM_LOAD( "omvbios.bin", 0x0000, 0x4000, BAD_DUMP CRC(c5a67b95) SHA1(6d7c64dd60dee4a33061d3d3a7c2ed190d895cdb) )	// The BIOS comes from a Multivision FG-2000. It is still unknown if the FG-1000 BIOS differs
ROM_END

ROM_START( omv2000 )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASE00 )
	ROM_LOAD( "omvbios.bin", 0x0000, 0x4000, CRC(c5a67b95) SHA1(6d7c64dd60dee4a33061d3d3a7c2ed190d895cdb) )
ROM_END

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT      MACHINE     INPUT       INIT    COMPANY             FULLNAME                                    FLAGS */
CONS( 1983,	sg1000,     0,          0,          sg1000,     sg1000, driver_device,     0,      "Sega",             "SG-1000",                                  GAME_SUPPORTS_SAVE )
CONS( 1984,	sg1000m2,   sg1000,     0,          sc3000,     sc3000, driver_device,     0,      "Sega",             "SG-1000 II",                               GAME_SUPPORTS_SAVE )
COMP( 1983,	sc3000,     0,          sg1000,     sc3000,     sc3000, driver_device,     0,      "Sega",             "SC-3000",                                  GAME_SUPPORTS_SAVE )
COMP( 1983,	sc3000h,    sc3000,     0,          sc3000,     sc3000, driver_device,     0,      "Sega",             "SC-3000H",                                 GAME_SUPPORTS_SAVE )
COMP( 1983,	sf7000,     sc3000,     0,          sf7000,     sf7000, driver_device,     0,      "Sega",             "SC-3000/Super Control Station SF-7000",    GAME_SUPPORTS_SAVE )
CONS( 1984,	omv1000,    sg1000,     0,          omv,        omv, driver_device,        0,      "Tsukuda Original", "Othello Multivision FG-1000",              GAME_SUPPORTS_SAVE )
CONS( 1984,	omv2000,    sg1000,     0,          omv,        omv, driver_device,        0,      "Tsukuda Original", "Othello Multivision FG-2000",              GAME_SUPPORTS_SAVE )
