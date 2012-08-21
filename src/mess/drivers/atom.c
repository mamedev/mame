/***************************************************************************
Acorn Atom:

Memory map.

CPU: 65C02
        0000-00ff Zero page
        0100-01ff Stack
        0200-1fff RAM (expansion)
        0a00-0a04 FDC 8271
        2000-21ff RAM (dos catalogue buffer)
        2200-27ff RAM (dos seq file buffer)
        2800-28ff RAM (float buffer)
        2900-7fff RAM (text RAM)
        8000-97ff VDG 6847
        9800-9fff RAM (expansion)
        a000-afff ROM (extension)
        b000-b003 PPIA 8255
        b003-b7ff NOP
        b800-bbff VIA 6522
        bc00-bfdf NOP
        bfe0-bfe2 MOUSE - extension??
        bfe3-bfff NOP
        c000-cfff ROM (basic)
        d000-dfff ROM (float)
        e000-efff ROM (dos)
        f000-ffff ROM (kernel)

Video:      MC6847

Sound:      Buzzer
Floppy:     FDC8271

Hardware:   PPIA 8255

    output  b000    0 - 3 keyboard row, 4 - 7 graphics mode
            b002    0 cas output, 1 enable 2.4kHz, 2 buzzer, 3 colour set

    input   b001    0 - 5 keyboard column, 6 CTRL key, 7 SHIFT key
            b002    4 2.4kHz input, 5 cas input, 6 REPT key, 7 60 Hz input

            VIA 6522


    DOS:

    The original location of the 8271 memory mapped registers is 0xa00-0x0a04.
    (This is the memory range assigned by Acorn in their design.)

    This is in the middle of the area for expansion RAM. Many Atom owners
    thought this was a bad design and have modified their Atom's and dos rom
    to use a different memory area.

    The atom driver in MESS uses the original memory area.


    http://www.xs4all.nl/~fjkraan/comp/atom/index.html

    ---

    The Econet card for the ATOM is decoded on the ATOM PCB at memory address B400 (hex). The Econet Eurocard has decoding circuits on it which select memory address 1940 (hex).
    There are then five significant addresses above these bases which contain the following registers: -

                ATOM card   Eurocard
    6854    register 1  B400        1940
    6854    register 2  B401        1941
    6854    register 3  B402        1942
    6854    Tx/Rx Data reg. B403        1943
    Station identification  B404        1944

    Station identification

    The identity number of each station is set up in hardware by links to IC 8. IC 8 is an octal buffer which when enabled feeds the cards station ID to the computer bus.
    Each link codes a bit in an eight bit binary number allowing any station ID in the range 0 to 255 to be set up. if a link is left open then the bit is a one, when a
    link is made the bit is a zero. Hence all links open corresponds to station ID 255, and all links made to station ID 0. Each station must have a unique identity and
    some indentities are associated with specific functions on the network. Station ID zero is reserved for broadcast signals and should not be used. Station ID 255 is
    reserved at present for the file server, and 235 for the printer server. Wire links must be soldered to each network station card during installation, a sugested
    scheme for number allocation is to number normal user stations from one upwards and to number special stations and servers from 255 downwards.

    2011 June 04  - Phill Harvey-Smith
        Fixed "ERROR" repeating infinite loop, caused by random values in machine_start() being poked into the wrong memory reigion causing the basic ROM to become
        corrupted. Values are now correctly placed in bytes 0x0008 - 0x000B of RAM.


***************************************************************************/

/*

    TODO:

    - connect to softwarelist
    - e000 EPROM switching
    - display should be monochrome -- Should be optional, Acorn produced a Colour Card, and there is
        at least one aftermarket Colour card.
    - ram expansion
    - tap files
    - mouse
    - color card
    - CP/M card
    - speech synthesis card (SPO256 connected to VIA)
    - econet
    - teletext card
    - Busicomputers Prophet 2
        * The Shift and Return keys are orange and the Return key is large,
        * There is a MODE switch to the top right of the keyboard,
        * There is a VIDEO port in addition to the TV output,
        * An Acorn AtomCalc ROM PCB is installed (is this standard on the Prophet2 or an upgrade?),
        * An Acorn 32K dynamic RAM card is installed,
        * A 5v DC input is added in addition to the standard power in (but this may be a later upgrade),
        * The Utility ROM is labelled P2/FP is installed

*/

#include "includes/atom.h"

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    bankswitch - EPROM bankswitch
-------------------------------------------------*/

void atom_state::bankswitch()
{
	address_space *program = m_maincpu->memory().space(AS_PROGRAM);

	UINT8 *eprom = memregion(EXTROM_TAG)->base() + (m_eprom << 12);

	program->install_rom(0xa000, 0xafff, eprom);
}

/*-------------------------------------------------
     eprom_r - EPROM slot select read
-------------------------------------------------*/

READ8_MEMBER( atom_state::eprom_r )
{
	return m_eprom;
}

/*-------------------------------------------------
     eprom_w - EPROM slot select write
-------------------------------------------------*/

WRITE8_MEMBER( atom_state::eprom_w )
{
	/*

        bit     description

        0       block A bit 0
        1       block A bit 1
        2       block A bit 2
        3       block A bit 3
        4
        5
        6
        7       block E

    */

	/* block A */
	m_eprom = data & 0x0f;

	/* TODO block E */

	bankswitch();
}

/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( atom_mem )
-------------------------------------------------*/

static ADDRESS_MAP_START( atom_mem, AS_PROGRAM, 8, atom_state )
	AM_RANGE(0x0000, 0x09ff) AM_RAM
	AM_RANGE(0x0a00, 0x0a03) AM_MIRROR(0x1f8) AM_DEVREADWRITE_LEGACY(I8271_TAG, i8271_r, i8271_w)
	AM_RANGE(0x0a04, 0x0a04) AM_MIRROR(0x1f8) AM_DEVREADWRITE_LEGACY(I8271_TAG, i8271_data_r, i8271_data_w)
	AM_RANGE(0x0a05, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x97ff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0x9800, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xafff) AM_ROM AM_REGION(EXTROM_TAG, 0)
	AM_RANGE(0xb000, 0xb003) AM_MIRROR(0x3fc) AM_DEVREADWRITE(INS8255_TAG, i8255_device, read, write)
//  AM_RANGE(0xb400, 0xb403) AM_DEVREADWRITE_LEGACY(MC6854_TAG, mc6854_r, mc6854_w)
//  AM_RANGE(0xb404, 0xb404) AM_READ_PORT("ECONET")
	AM_RANGE(0xb800, 0xb80f) AM_MIRROR(0x3f0) AM_DEVREADWRITE(R6522_TAG, via6522_device, read, write)
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION(SY6502_TAG, 0)
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( atomeb_mem )
-------------------------------------------------*/

static ADDRESS_MAP_START( atomeb_mem, AS_PROGRAM, 8, atom_state )
	AM_IMPORT_FROM(atom_mem)
	AM_RANGE(0xbfff, 0xbfff) AM_READWRITE(eprom_r, eprom_w)
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_CHANGED( trigger_reset )
-------------------------------------------------*/

INPUT_CHANGED_MEMBER( atom_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

/*-------------------------------------------------
    INPUT_PORTS( atom )
-------------------------------------------------*/

static INPUT_PORTS_START( atom )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC")          PORT_CODE(KEYCODE_TILDE)      PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x87\x95") PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x87\x94") PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK")         PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DELETE")       PORT_CODE(KEYCODE_DEL)        PORT_CHAR(8)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COPY")         PORT_CODE(KEYCODE_TAB)        PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('[')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE")        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(32)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL")         PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT")        PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("RPT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REPT")         PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))

	PORT_START("BRK")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK")        PORT_CODE(KEYCODE_ESC)   PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_CHANGED_MEMBER(DEVICE_SELF, atom_state, trigger_reset, 0)

	PORT_START("ECONET")
	// station ID (0-255)
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    I8255_INTERFACE( ppi_intf )
-------------------------------------------------*/

WRITE8_MEMBER( atom_state::ppi_pa_w )
{
	/*

        bit     description

        0       keyboard column 0
        1       keyboard column 1
        2       keyboard column 2
        3       keyboard column 3
        4       MC6847 A/G
        5       MC6847 GM0
        6       MC6847 GM1
        7       MC6847 GM2

    */

	/* keyboard column */
	m_keylatch = data & 0x0f;

	/* MC6847 */
	m_vdg->ag_w(BIT(data, 4));
	m_vdg->gm0_w(BIT(data, 5));
	m_vdg->gm1_w(BIT(data, 6));
	m_vdg->gm2_w(BIT(data, 7));
}

READ8_MEMBER( atom_state::ppi_pb_r )
{
	/*

        bit     description

        0       keyboard row 0
        1       keyboard row 1
        2       keyboard row 2
        3       keyboard row 3
        4       keyboard row 4
        5       keyboard row 5
        6       keyboard CTRL
        7       keyboard SFT

    */

	UINT8 data = 0xff;

	switch (m_keylatch)
	{
	case 0: data &= ioport("KEY0")->read(); break;
	case 1: data &= ioport("KEY1")->read(); break;
	case 2: data &= ioport("KEY2")->read(); break;
	case 3: data &= ioport("KEY3")->read(); break;
	case 4: data &= ioport("KEY4")->read(); break;
	case 5: data &= ioport("KEY5")->read(); break;
	case 6: data &= ioport("KEY6")->read(); break;
	case 7: data &= ioport("KEY7")->read(); break;
	case 8: data &= ioport("KEY8")->read(); break;
	case 9: data &= ioport("KEY9")->read(); break;
	}

	data &= ioport("KEY10")->read();

	return data;
}

READ8_MEMBER( atom_state::ppi_pc_r )
{
	/*

        bit     description

        0       O/P 1, cassette output 0
        1       O/P 2, cassette output 1
        2       O/P 3, speaker output
        3       O/P 4, MC6847 CSS
        4       2400 Hz input
        5       cassette input
        6       keyboard RPT
        7       MC6847 FS

    */

	UINT8 data = 0;

	/* 2400 Hz input */
	data |= m_hz2400 << 4;

	/* cassette input */
	data |= (m_cassette->input() > 0.0) << 5;

	/* keyboard RPT */
	data |= BIT(ioport("RPT")->read(), 0) << 6;

	/* MC6847 FS */
	data |= (m_vdg->fs_r() ? 1 : 0) << 7;

	return data;
}

WRITE8_MEMBER( atom_state::ppi_pc_w )
{
	/*

        bit     description

        0       O/P 1, cassette output 0
        1       O/P 2, cassette output 1
        2       O/P 3, speaker output
        3       O/P 4, MC6847 CSS
        4       2400 Hz input
        5       cassette input
        6       keyboard RPT
        7       MC6847 FS

    */

	/* cassette output */
	m_pc0 = BIT(data, 0);
	m_pc1 = BIT(data, 1);

	/* speaker output */
	speaker_level_w(m_speaker, BIT(data, 2));

	/* MC6847 CSS */
	m_vdg->css_w(BIT(data, 3));
}

static I8255_INTERFACE( ppi_intf )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(atom_state, ppi_pa_w),
	DEVCB_DRIVER_MEMBER(atom_state, ppi_pb_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(atom_state, ppi_pc_r),
	DEVCB_DRIVER_MEMBER(atom_state, ppi_pc_w)
};

/*-------------------------------------------------
    via6522_interface via_intf
-------------------------------------------------*/

READ8_MEMBER( atom_state::printer_busy )
{
	return m_centronics->busy_r() << 7;
}

WRITE8_MEMBER( atom_state::printer_data )
{
	m_centronics->write(space, 0, data & 0x7f);
}

static const via6522_interface via_intf =
{
	DEVCB_DRIVER_MEMBER(atom_state, printer_busy),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(atom_state, printer_data),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(CENTRONICS_TAG, centronics_device, strobe_w),
	DEVCB_NULL,
	DEVCB_CPU_INPUT_LINE(SY6502_TAG, INPUT_LINE_IRQ0)
};

/*-------------------------------------------------
    i8271_interface fdc_intf
-------------------------------------------------*/

static void atom_8271_interrupt_callback(device_t *device, int state)
{
	atom_state *drvstate = device->machine().driver_data<atom_state>();
	/* I'm assuming that the nmi is edge triggered */
	/* a interrupt from the fdc will cause a change in line state, and
    the nmi will be triggered, but when the state changes because the int
    is cleared this will not cause another nmi */
	/* I'll emulate it like this to be sure */

	if (state!=drvstate->m_previous_i8271_int_state)
	{
		if (state)
		{
			/* I'll pulse it because if I used hold-line I'm not sure
            it would clear - to be checked */
			drvstate->m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		}
	}

	drvstate->m_previous_i8271_int_state = state;
}

static const i8271_interface fdc_intf =
{
	atom_8271_interrupt_callback,
	NULL,
	{ FLOPPY_0, FLOPPY_1 }
};

/*-------------------------------------------------
    centronics_interface atom_centronics_config
-------------------------------------------------*/

static const centronics_interface atom_centronics_config =
{
	DEVCB_DEVICE_LINE_MEMBER(R6522_TAG, via6522_device, write_ca1),
	DEVCB_NULL,
	DEVCB_NULL
};

/*-------------------------------------------------
    LEGACY_FLOPPY_OPTIONS( atom )
-------------------------------------------------*/

static LEGACY_FLOPPY_OPTIONS_START( atom )
	LEGACY_FLOPPY_OPTION(atom, "dsk,40t", "Atom disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([10])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

/*-------------------------------------------------
    floppy_interface atom_floppy_interface
-------------------------------------------------*/

static const floppy_interface atom_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_SSDD_40,
	LEGACY_FLOPPY_OPTIONS_NAME(atom),
	"floppy_5_25",
	NULL
};

/*-------------------------------------------------
    cassette_interface atom_cassette_interface
-------------------------------------------------*/

static TIMER_DEVICE_CALLBACK( cassette_output_tick )
{
	atom_state *state = timer.machine().driver_data<atom_state>();

	int level = !(!(!state->m_hz2400 && state->m_pc1) && state->m_pc0);

	state->m_cassette->output(level ? -1.0 : +1.0);

	state->m_hz2400 = !state->m_hz2400;
}

static const cassette_interface atom_cassette_interface =
{
	atom_cassette_formats,
	NULL,
	(cassette_state) (CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

/*-------------------------------------------------
    mc6847_interface vdg_intf
-------------------------------------------------*/

READ8_MEMBER( atom_state::vdg_videoram_r )
{
	if (offset == ~0) return 0xff;

	m_vdg->as_w(BIT(m_video_ram[offset], 6));
	m_vdg->intext_w(BIT(m_video_ram[offset], 6));
	m_vdg->inv_w(BIT(m_video_ram[offset], 7));

	return m_video_ram[offset];
}

static const mc6847_interface vdg_intf =
{
	SCREEN_TAG,
	DEVCB_DRIVER_MEMBER(atom_state, vdg_videoram_r)
};

/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    MACHINE_START( atom )
-------------------------------------------------*/

void atom_state::machine_start()
{
	/* this is temporary */
	/* Kees van Oss mentions that address 8-b are used for the random number
    generator. I don't know if this is hardware, or random data because the
    ram chips are not cleared at start-up. So at this time, these numbers
    are poked into the memory to simulate it. When I have more details I will fix it */
	UINT8 *m_baseram = (UINT8 *)m_maincpu->memory().space(AS_PROGRAM)->get_write_ptr(0x0000);

	m_baseram[0x08] = machine().rand() & 0x0ff;
	m_baseram[0x09] = machine().rand() & 0x0ff;
	m_baseram[0x0a] = machine().rand() & 0x0ff;
	m_baseram[0x0b] = machine().rand() & 0x0ff;
}

/*-------------------------------------------------
    MACHINE_START( atomeb )
-------------------------------------------------*/

void atomeb_state::machine_start()
{
	atom_state::machine_start();

	bankswitch();
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

struct atom_cart_range
{
	const char *tag;
	int offset;
	const char *region;
};

static const struct atom_cart_range atom_cart_table[] =
{
	{ "cart", 0x0000, "a000" },
	{ "a0",   0x0000, "a000" },
	{ "a1",   0x1000, "a000" },
	{ "a2",   0x2000, "a000" },
	{ "a3",   0x3000, "a000" },
	{ "a4",   0x4000, "a000" },
	{ "a5",   0x5000, "a000" },
	{ "a6",   0x6000, "a000" },
	{ "a7",   0x7000, "a000" },
	{ "a8",   0x8000, "a000" },
	{ "a9",   0x9000, "a000" },
	{ "aa",   0xa000, "a000" },
	{ "ab",   0xb000, "a000" },
	{ "ac",   0xc000, "a000" },
	{ "ad",   0xd000, "a000" },
	{ "ae",   0xe000, "a000" },
	{ "af",   0xf000, "a000" },
	{ "e0",   0x0000, "e000" },
	{ "e1",   0x1000, "e000" },
	{ 0 }
};

static DEVICE_IMAGE_LOAD( atom_cart )
{
	UINT32 size;
	UINT8 *temp_copy;
	int mirror, i;
	const struct atom_cart_range *atom_cart = &atom_cart_table[0], *this_cart;

	/* First, determine where this cart has to be loaded */
	while (atom_cart->tag)
	{
		if (strcmp(atom_cart->tag, image.device().tag()) == 0)
			break;

		atom_cart++;
	}

	this_cart = atom_cart;

	if (image.software_entry() == NULL)
	{
		size = image.length();
		temp_copy = auto_alloc_array(image.device().machine(), UINT8, size);

		if (size > 0x1000)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
			auto_free(image.device().machine(), temp_copy);
			return IMAGE_INIT_FAIL;
		}

		if (image.fread(temp_copy, size) != size)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to fully read from file");
			auto_free(image.device().machine(), temp_copy);
			return IMAGE_INIT_FAIL;
		}
	}
	else
	{
		size = image.get_software_region_length( "rom");
		temp_copy = auto_alloc_array(image.device().machine(), UINT8, size);
		memcpy(temp_copy, image.get_software_region("rom"), size);
	}

	mirror = 0x1000 / size;

	/* With the following, we mirror the cart in the whole memory region */
	for (i = 0; i < mirror; i++)
		memcpy(image.device().machine().root_device().memregion(this_cart->region)->base() + this_cart->offset + i * size, temp_copy, size);

	auto_free(image.device().machine(), temp_copy);

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
    MACHINE_DRIVER( atom )
-------------------------------------------------*/

#define MCFG_ATOM_CARTSLOT_ADD(_tag) \
	MCFG_CARTSLOT_ADD(_tag) \
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom") \
	MCFG_CARTSLOT_INTERFACE("atom_cart") \
	MCFG_CARTSLOT_LOAD(atom_cart)


static MACHINE_CONFIG_START( atom, atom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(SY6502_TAG, M6502, X2/4)
	MCFG_CPU_PROGRAM_MAP(atom_mem)

	/* video hardware */
    MCFG_SCREEN_MC6847_PAL_ADD(SCREEN_TAG, MC6847_TAG)
	MCFG_MC6847_ADD(MC6847_TAG, MC6847_PAL, XTAL_4_433619MHz, vdg_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_TIMER_ADD_PERIODIC("hz2400", cassette_output_tick, attotime::from_hz(4806)) // X2/4/416
	MCFG_VIA6522_ADD(R6522_TAG, X2/4, via_intf)
	MCFG_I8255_ADD(INS8255_TAG, ppi_intf)
	MCFG_I8271_ADD(I8271_TAG, fdc_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(atom_floppy_interface)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, atom_centronics_config)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, atom_cassette_interface)
	MCFG_QUICKLOAD_ADD("quickload", atom_atm, "atm", 0)

	/* cartridge */
	MCFG_ATOM_CARTSLOT_ADD("cart")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2K")
	MCFG_RAM_EXTRA_OPTIONS("4K,6K,8K,10K,12K,32K")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","atom")
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_DRIVER( atomeb )
-------------------------------------------------*/

static MACHINE_CONFIG_DERIVED_CLASS( atomeb, atom, atomeb_state )
	MCFG_CPU_MODIFY(SY6502_TAG)
	MCFG_CPU_PROGRAM_MAP(atomeb_mem)

	/* cartridges */
	MCFG_DEVICE_REMOVE("cart")
	MCFG_ATOM_CARTSLOT_ADD("a0")
	MCFG_ATOM_CARTSLOT_ADD("a1")
	MCFG_ATOM_CARTSLOT_ADD("a2")
	MCFG_ATOM_CARTSLOT_ADD("a3")
	MCFG_ATOM_CARTSLOT_ADD("a4")
	MCFG_ATOM_CARTSLOT_ADD("a5")
	MCFG_ATOM_CARTSLOT_ADD("a6")
	MCFG_ATOM_CARTSLOT_ADD("a7")
	MCFG_ATOM_CARTSLOT_ADD("a8")
	MCFG_ATOM_CARTSLOT_ADD("a9")
	MCFG_ATOM_CARTSLOT_ADD("aa")
	MCFG_ATOM_CARTSLOT_ADD("ab")
	MCFG_ATOM_CARTSLOT_ADD("ac")
	MCFG_ATOM_CARTSLOT_ADD("ad")
	MCFG_ATOM_CARTSLOT_ADD("ae")
	MCFG_ATOM_CARTSLOT_ADD("af")

	MCFG_ATOM_CARTSLOT_ADD("e0")
	MCFG_ATOM_CARTSLOT_ADD("e1")
MACHINE_CONFIG_END

/***************************************************************************
    ROMS
***************************************************************************/

/*-------------------------------------------------
    ROM( atom )
-------------------------------------------------*/

ROM_START( atom )
	ROM_REGION( 0x4000, SY6502_TAG, 0 )
	ROM_LOAD( "abasic.ic20", 0x0000, 0x1000, CRC(289b7791) SHA1(0072c83458a9690a3ea1f6094f0f38cf8e96a445) )
	ROM_CONTINUE(			 0x3000, 0x1000 )
	ROM_LOAD( "afloat.ic21", 0x1000, 0x1000, CRC(81d86af7) SHA1(ebcde5b36cb3a3344567cbba4c7b9fde015f4802) )
	ROM_LOAD( "dosrom.u15",  0x2000, 0x1000, CRC(c431a9b7) SHA1(71ea0a4b8d9c3caf9718fc7cc279f4306a23b39c) )

	ROM_REGION( 0x1000, "a000", ROMREGION_ERASEFF )
ROM_END

/*-------------------------------------------------
    ROM( atomeb )
-------------------------------------------------*/

ROM_START( atomeb )
	ROM_REGION( 0x4000, SY6502_TAG, 0 )
	ROM_LOAD( "abasic.ic20", 0x0000, 0x1000, CRC(289b7791) SHA1(0072c83458a9690a3ea1f6094f0f38cf8e96a445) )
	ROM_CONTINUE(			 0x3000, 0x1000 )
	ROM_LOAD( "afloat.ic21", 0x1000, 0x1000, CRC(81d86af7) SHA1(ebcde5b36cb3a3344567cbba4c7b9fde015f4802) )
	ROM_LOAD( "dosrom.u15",  0x2000, 0x1000, CRC(c431a9b7) SHA1(71ea0a4b8d9c3caf9718fc7cc279f4306a23b39c) )

	ROM_REGION( 0x10000, EXTROM_TAG, ROMREGION_ERASEFF )

	ROM_REGION( 0x2000, DOSROM_TAG, ROMREGION_ERASEFF )
ROM_END

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT      COMPANY   FULLNAME */
COMP( 1979, atom,     0,        0,		atom,     atom, driver_device,     0,        "Acorn",  "Atom" , 0)
COMP( 1979, atomeb,   atom,     0,		atomeb,   atom, driver_device,     0,        "Acorn",  "Atom with Eprom Box" , 0)
//COMP( 1983, prophet2, atom,     0,        atom,     atom, driver_device,     0,        "Busicomputers",  "Prophet 2" , 0)
