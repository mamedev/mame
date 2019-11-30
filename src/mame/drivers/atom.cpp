// license:BSD-3-Clause
// copyright-holders:Nathan Woods
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
    some identities are associated with specific functions on the network. Station ID zero is reserved for broadcast signals and should not be used. Station ID 255 is
    reserved at present for the file server, and 235 for the printer server. Wire links must be soldered to each network station card during installation, a suggested
    scheme for number allocation is to number normal user stations from one upwards and to number special stations and servers from 255 downwards.

    2011 June 04  - Phill Harvey-Smith
        Fixed "ERROR" repeating infinite loop, caused by random values in machine_start() being poked into the wrong memory region causing the basic ROM to become
        corrupted. Values are now correctly placed in bytes 0x0008 - 0x000B of RAM.


***************************************************************************/

/*

    TODO:

    - e000 EPROM switching
    - display should be monochrome -- Should be optional, Acorn produced a Colour Card, and there is
        at least one after market Colour card.
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

#include "emu.h"
#include "includes/atom.h"
#include "formats/imageutl.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 1

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    QUICKLOAD_LOAD_MEMBER(atom_state::quickload_cb)
-------------------------------------------------*/

QUICKLOAD_LOAD_MEMBER(atom_state::quickload_cb)
{
	/*

	    The format for the .ATM files is as follows:

	    Offset Size     Description
	    ------ -------- -----------------------------------------------------------
	    0000h  16 BYTEs ATOM filename (if less than 16 BYTEs, rest is 00h bytes)
	    0010h  WORD     Start address for load
	    0012h  WORD     Execution address
	    0014h  WORD     Size of data in BYTEs
	    0016h  Size     Data

	*/

	uint8_t header[0x16] = { 0 };
	void *ptr;

	image.fread(header, 0x16);

	uint16_t start_address = pick_integer_le(header, 0x10, 2);
	uint16_t run_address = pick_integer_le(header, 0x12, 2);
	uint16_t size = pick_integer_le(header, 0x14, 2);

	if (LOG)
	{
		header[16] = 0;
		logerror("ATM filename: %s\n", header);
		logerror("ATM start address: %04x\n", start_address);
		logerror("ATM run address: %04x\n", run_address);
		logerror("ATM size: %04x\n", size);
	}

	ptr = m_maincpu->space(AS_PROGRAM).get_write_ptr(start_address);
	image.fread(ptr, size);

	m_maincpu->set_pc(run_address);

	return image_init_result::PASS;
}

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
     eprom_r - EPROM slot select read
-------------------------------------------------*/

READ8_MEMBER( atomeb_state::eprom_r )
{
	return m_eprom;
}

/*-------------------------------------------------
     eprom_w - EPROM slot select write
-------------------------------------------------*/

WRITE8_MEMBER( atomeb_state::eprom_w )
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

	/* block A and E */
	m_eprom = data;
}

/*-------------------------------------------------
 ext_r - read external roms at 0xa000
 -------------------------------------------------*/

READ8_MEMBER( atomeb_state::ext_r )
{
	if (m_ext[m_eprom & 0x0f]->exists())
		return m_ext[m_eprom & 0x0f]->read_rom(offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 dor_r - read DOS roms at 0xe000
 -------------------------------------------------*/

READ8_MEMBER( atomeb_state::dos_r )
{
	if (m_e0->exists() && !BIT(m_eprom, 7))
		return m_e0->read_rom(offset);
	else if (m_e1->exists() && BIT(m_eprom, 7))
		return m_e1->read_rom(offset);
	else
		return 0xff;
}


/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( atom_mem )
-------------------------------------------------*/

void atom_state::atom_mem(address_map &map)
{
	map(0x0000, 0x09ff).ram();
	map(0x0a00, 0x0a03).mirror(0x1f8).m(m_fdc, FUNC(i8271_device::map));
	map(0x0a04, 0x0a04).mirror(0x1f8).rw(m_fdc, FUNC(i8271_device::data_r), FUNC(i8271_device::data_w));
	map(0x0a05, 0x7fff).ram();
	map(0x8000, 0x97ff).ram().share("video_ram");
	map(0x9800, 0x9fff).ram();
//  map(0xa000, 0xafff)        // mapped by the cartslot
	map(0xb000, 0xb003).mirror(0x3fc).rw(INS8255_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
//  map(0xb400, 0xb403).rw(MC6854_TAG, FUNC(mc6854_device::read), FUNC(mc6854_device::write));
//  map(0xb404, 0xb404).portr("ECONET");
	map(0xb800, 0xb80f).mirror(0x3f0).m(R6522_TAG, FUNC(via6522_device::map));
	map(0xc000, 0xffff).rom().region(SY6502_TAG, 0);
}

/*-------------------------------------------------
    ADDRESS_MAP( atomeb_mem )
-------------------------------------------------*/

void atomeb_state::atomeb_mem(address_map &map)
{
	atom_mem(map);
	map(0xa000, 0xafff).r(FUNC(atomeb_state::ext_r));
	map(0xbfff, 0xbfff).rw(FUNC(atomeb_state::eprom_r), FUNC(atomeb_state::eprom_w));
	map(0xe000, 0xefff).r(FUNC(atomeb_state::dos_r));
}

/*-------------------------------------------------
    ADDRESS_MAP( atombb_mem )
-------------------------------------------------*/

void atom_state::atombb_mem(address_map &map)
{
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0x57ff).ram().share("video_ram");
	map(0x7000, 0x7003).mirror(0x3fc).rw(INS8255_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7800, 0x780f).mirror(0x3f0).m(R6522_TAG, FUNC(via6522_device::map));
	map(0x8000, 0xbfff).rom().region("basic", 0);
	map(0xf000, 0xffff).rom().region(SY6502_TAG, 0);
}

/*-------------------------------------------------
    ADDRESS_MAP( prophet_mem )
-------------------------------------------------*/

void atom_state::prophet_mem(address_map &map)
{
	map(0x0000, 0x09ff).ram();
	map(0x0a00, 0x7fff).ram();
	map(0x8000, 0x97ff).ram().share("video_ram");
	map(0x9800, 0x9fff).ram();
	map(0xa000, 0xafff).rom().region("ic24", 0);
	map(0xb000, 0xb003).mirror(0x3fc).rw(INS8255_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb800, 0xb80f).mirror(0x3f0).m(R6522_TAG, FUNC(via6522_device::map));
	map(0xc000, 0xffff).rom().region(SY6502_TAG, 0);
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_CHANGED_MEMBER( trigger_reset )
-------------------------------------------------*/

INPUT_CHANGED_MEMBER( atom_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

/*-------------------------------------------------
    INPUT_PORTS( atom )
-------------------------------------------------*/

static INPUT_PORTS_START( atom )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC")          PORT_CODE(KEYCODE_TILDE)      PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x87\x95") PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x87\x94") PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK")         PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DELETE")       PORT_CODE(KEYCODE_DEL)        PORT_CHAR(8)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COPY")         PORT_CODE(KEYCODE_TAB)        PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('[')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE")        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(32)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y10")
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
    I8255 interface
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

	uint8_t data = 0xff;

	switch (m_keylatch)
	{
	case 0: data &= m_y0->read(); break;
	case 1: data &= m_y1->read(); break;
	case 2: data &= m_y2->read(); break;
	case 3: data &= m_y3->read(); break;
	case 4: data &= m_y4->read(); break;
	case 5: data &= m_y5->read(); break;
	case 6: data &= m_y6->read(); break;
	case 7: data &= m_y7->read(); break;
	case 8: data &= m_y8->read(); break;
	case 9: data &= m_y9->read(); break;
	}

	data &= m_y10->read();

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

	uint8_t data = 0;

	/* 2400 Hz input */
	data |= m_hz2400 << 4;

	/* cassette input */
	data |= (m_cassette->input() > 0.0) << 5;

	/* keyboard RPT */
	data |= BIT(m_rpt->read(), 0) << 6;

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
	m_speaker->level_w(BIT(data, 2));

	/* MC6847 CSS */
	m_vdg->css_w(BIT(data, 3));
}

/*-------------------------------------------------
    i8271 interface
-------------------------------------------------*/

WRITE_LINE_MEMBER( atom_state::atom_8271_interrupt_callback )
{
	/* I'm assuming that the nmi is edge triggered */
	/* a interrupt from the fdc will cause a change in line state, and
	the nmi will be triggered, but when the state changes because the int
	is cleared this will not cause another nmi */
	/* I'll emulate it like this to be sure */

	if (state!=m_previous_i8271_int_state)
	{
		if (state)
		{
			/* I'll pulse it because if I used hold-line I'm not sure
			it would clear - to be checked */
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}
	}

	m_previous_i8271_int_state = state;
}

WRITE_LINE_MEMBER( atom_state::motor_w )
{
	for (int i=0; i != 2; i++) {
		char devname[8];
		sprintf(devname, "%d", i);
		floppy_connector *con = m_fdc->subdevice<floppy_connector>(devname);
		if (con) {
			con->get_device()->mon_w(!state);
		}
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(atom_state::cassette_output_tick)
{
	int level = !(!(!m_hz2400 && m_pc1) && m_pc0);

	m_cassette->output(level ? -1.0 : +1.0);

	m_hz2400 = !m_hz2400;
}

/*-------------------------------------------------
    mc6847 interface
-------------------------------------------------*/

READ8_MEMBER( atom_state::vdg_videoram_r )
{
	if (offset == ~0) return 0xff;

	m_vdg->as_w(BIT(m_video_ram[offset], 6));
	m_vdg->intext_w(BIT(m_video_ram[offset], 6));
	m_vdg->inv_w(BIT(m_video_ram[offset], 7));

	return m_video_ram[offset];
}

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
	uint8_t *m_baseram = (uint8_t *)m_maincpu->space(AS_PROGRAM).get_write_ptr(0x0000);

	m_baseram[0x08] = machine().rand() & 0x0ff;
	m_baseram[0x09] = machine().rand() & 0x0ff;
	m_baseram[0x0a] = machine().rand() & 0x0ff;
	m_baseram[0x0b] = machine().rand() & 0x0ff;

	if (m_cart.found() && m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xa000, 0xafff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));
}

/*-------------------------------------------------
    MACHINE_START( atomeb )
-------------------------------------------------*/

void atomeb_state::machine_start()
{
	atom_state::machine_start();
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

image_init_result atom_state::load_cart(device_image_interface &image, generic_slot_device &slot)
{
	uint32_t size = slot.common_get_size("rom");

	if (size > 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported ROM size");
		return image_init_result::FAIL;
	}

	slot.rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot.common_load_rom(slot.get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

static void atom_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
}

FLOPPY_FORMATS_MEMBER(atom_state::floppy_formats)
	FLOPPY_ATOM_FORMAT
FLOPPY_FORMATS_END0

/*-------------------------------------------------
    MACHINE_DRIVER( atom )
-------------------------------------------------*/

void atom_state::atom(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, X2/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &atom_state::atom_mem);

	/* video hardware */
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, XTAL(4'433'619));
	m_vdg->input_callback().set(FUNC(atom_state::vdg_videoram_r));
	m_vdg->set_screen(SCREEN_TAG);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* devices */
	TIMER(config, "hz2400").configure_periodic(FUNC(atom_state::cassette_output_tick), attotime::from_hz(4806));

	via6522_device &via(VIA6522(config, R6522_TAG, X2/4));
	via.writepa_handler().set("cent_data_out", FUNC(output_latch_device::bus_w));
	via.ca2_handler().set(m_centronics, FUNC(centronics_device::write_strobe));
	via.irq_handler().set_inputline(SY6502_TAG, M6502_IRQ_LINE);

	i8255_device &ppi(I8255(config, INS8255_TAG));
	ppi.out_pa_callback().set(FUNC(atom_state::ppi_pa_w));
	ppi.in_pb_callback().set(FUNC(atom_state::ppi_pb_r));
	ppi.in_pc_callback().set(FUNC(atom_state::ppi_pc_r));
	ppi.out_pc_callback().set(FUNC(atom_state::ppi_pc_w));

	I8271(config, m_fdc, 0);
	m_fdc->intrq_wr_callback().set(FUNC(atom_state::atom_8271_interrupt_callback));
	m_fdc->hdl_wr_callback().set(FUNC(atom_state::motor_w));
	FLOPPY_CONNECTOR(config, I8271_TAG ":0", atom_floppies, "525sssd", atom_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, I8271_TAG ":1", atom_floppies, "525sssd", atom_state::floppy_formats).enable_sound(true);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(R6522_TAG, FUNC(via6522_device::write_ca1));
	m_centronics->busy_handler().set(R6522_TAG, FUNC(via6522_device::write_pa7));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_formats(atom_cassette_formats);
	m_cassette->set_interface("atom_cass");

	QUICKLOAD(config, "quickload", "atm").set_load_callback(FUNC(atom_state::quickload_cb));

	/* utility rom slot */
	GENERIC_CARTSLOT(config, "cartslot", generic_linear_slot, "atom_cart", "bin,rom").set_device_load(FUNC(atom_state::cart_load));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("32K").set_extra_options("2K,4K,6K,8K,10K,12K").set_default_value(0x00);

	/* Software lists */
	SOFTWARE_LIST(config, "rom_list").set_original("atom_rom");
	SOFTWARE_LIST(config, "cass_list").set_original("atom_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("atom_flop");
}

/*-------------------------------------------------
    MACHINE_DRIVER( atomeb )
-------------------------------------------------*/

#define ATOM_ROM(_tag, _load) \
	GENERIC_SOCKET(config, _tag, generic_linear_slot, "atom_cart", "bin,rom").set_device_load(FUNC(atomeb_state::_load)) \

void atomeb_state::atomeb(machine_config &config)
{
	atom(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &atomeb_state::atomeb_mem);

	/* cartridges */
	config.device_remove("cartslot");

	ATOM_ROM("rom_a0", ext_load<0x0>);
	ATOM_ROM("rom_a1", ext_load<0x1>);
	ATOM_ROM("rom_a2", ext_load<0x2>);
	ATOM_ROM("rom_a3", ext_load<0x3>);
	ATOM_ROM("rom_a4", ext_load<0x4>);
	ATOM_ROM("rom_a5", ext_load<0x5>);
	ATOM_ROM("rom_a6", ext_load<0x6>);
	ATOM_ROM("rom_a7", ext_load<0x7>);
	ATOM_ROM("rom_a8", ext_load<0x8>);
	ATOM_ROM("rom_a9", ext_load<0x9>);
	ATOM_ROM("rom_aa", ext_load<0xa>);
	ATOM_ROM("rom_ab", ext_load<0xb>);
	ATOM_ROM("rom_ac", ext_load<0xc>);
	ATOM_ROM("rom_ad", ext_load<0xd>);
	ATOM_ROM("rom_ae", ext_load<0xe>);
	ATOM_ROM("rom_af", ext_load<0xf>);

	ATOM_ROM("rom_e0", e0_load);
	ATOM_ROM("rom_e1", e1_load);
}

/*-------------------------------------------------
    MACHINE_DRIVER( atombb )
-------------------------------------------------*/

void atom_state::atombb(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, X2/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &atom_state::atombb_mem);

	/* video hardware */
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, XTAL(4'433'619));
	m_vdg->input_callback().set(FUNC(atom_state::vdg_videoram_r));
	m_vdg->set_screen(SCREEN_TAG);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* devices */
	TIMER(config, "hz2400").configure_periodic(FUNC(atom_state::cassette_output_tick), attotime::from_hz(4806));

	via6522_device &via(VIA6522(config, R6522_TAG, X2/4));
	via.writepa_handler().set("cent_data_out", FUNC(output_latch_device::bus_w));
	via.ca2_handler().set(m_centronics, FUNC(centronics_device::write_strobe));
	via.irq_handler().set_inputline(SY6502_TAG, M6502_IRQ_LINE);

	i8255_device &ppi(I8255(config, INS8255_TAG));
	ppi.out_pa_callback().set(FUNC(atom_state::ppi_pa_w));
	ppi.in_pb_callback().set(FUNC(atom_state::ppi_pb_r));
	ppi.in_pc_callback().set(FUNC(atom_state::ppi_pc_r));
	ppi.out_pc_callback().set(FUNC(atom_state::ppi_pc_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(R6522_TAG, FUNC(via6522_device::write_ca1));
	m_centronics->busy_handler().set(R6522_TAG, FUNC(via6522_device::write_pa7));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(atom_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("atom_cass");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("16K").set_extra_options("8K,12K");
}

/*-------------------------------------------------
    MACHINE_DRIVER( prophet2 )
-------------------------------------------------*/

//void atom_state::prophet2(machine_config &config)
//{
//  atom(config);
//  /* basic machine hardware */
//  m_maincpu->set_addrmap(AS_PROGRAM, &atom_state::prophet_mem);
//
//  /* fdc */
//  config.device_remove(I8271_TAG);
//  config.device_remove(I8271_TAG ":0");
//  config.device_remove(I8271_TAG ":1");
//
//  /* internal ram */
//  subdevice<ram_device>(RAM_TAG)->set_default_size("32K");

//  /* Software lists */
//  config.device_remove("rom_list");
//  config.device_remove("flop_list");
//}

/*-------------------------------------------------
    MACHINE_DRIVER( prophet3 )
-------------------------------------------------*/

//void atom_state::prophet3(machine_config &config)
//{
//  atom(config);
//  /* basic machine hardware */
//  m_maincpu->set_addrmap(AS_PROGRAM, &atom_state::prophet_mem);
//
//  /* internal ram */
//  subdevice<ram_device>(RAM_TAG)->set_default_size("32K");

//  /* Software lists */
//  config.device_remove("rom_list");
//}

/*-------------------------------------------------
    MACHINE_DRIVER( atommc )
-------------------------------------------------*/

//void atom_state::atommc(machine_config &config)
//{
//  atom(config);
//  /* Software lists */
//  SOFTWARE_LIST(config, "mmc_list").set_original("atom_mmc");
//  config.device_remove("flop_list");
//}

/***************************************************************************
    ROMS
***************************************************************************/

/*-------------------------------------------------
    ROM( atom )
-------------------------------------------------*/

ROM_START( atom )
	ROM_REGION( 0x4000, SY6502_TAG, 0 )
	ROM_LOAD( "abasic.ic20", 0x0000, 0x1000, CRC(289b7791) SHA1(0072c83458a9690a3ea1f6094f0f38cf8e96a445) )
	ROM_CONTINUE(            0x3000, 0x1000 )
	ROM_LOAD( "afloat.ic21", 0x1000, 0x1000, CRC(81d86af7) SHA1(ebcde5b36cb3a3344567cbba4c7b9fde015f4802) )
	ROM_LOAD( "dosrom.u15",  0x2000, 0x1000, CRC(c431a9b7) SHA1(71ea0a4b8d9c3caf9718fc7cc279f4306a23b39c) )
ROM_END

/*-------------------------------------------------
    ROM( atomeb )
-------------------------------------------------*/

ROM_START( atomeb )
	ROM_REGION( 0x4000, SY6502_TAG, 0 )
	ROM_LOAD( "abasic.ic20", 0x0000, 0x1000, CRC(289b7791) SHA1(0072c83458a9690a3ea1f6094f0f38cf8e96a445) )
	ROM_CONTINUE(            0x3000, 0x1000 )
	ROM_LOAD( "afloat.ic21", 0x1000, 0x1000, CRC(81d86af7) SHA1(ebcde5b36cb3a3344567cbba4c7b9fde015f4802) )
	ROM_LOAD( "dosrom.u15",  0x2000, 0x1000, CRC(c431a9b7) SHA1(71ea0a4b8d9c3caf9718fc7cc279f4306a23b39c) )
ROM_END

/*-------------------------------------------------
    ROM( atombb )
-------------------------------------------------*/

ROM_START( atombb )
	ROM_REGION( 0x1000, SY6502_TAG, 0 )
	ROM_LOAD( "mos.rom",0x0000, 0x1000, CRC(20158bd8) SHA1(5ee4c0d2b65be72646e17d69b76fb00a0e5298df) )
	ROM_REGION( 0x4000, "basic", 0)
	ROM_LOAD( "bbcbasic.rom", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281) )
ROM_END

//#define rom_prophet2 rom_atom

//#define rom_prophet3 rom_atom

/*-------------------------------------------------
    ROM( atommc )
-------------------------------------------------*/

//ROM_START( atommc )
//  ROM_REGION( 0x4000, SY6502_TAG, 0 )
//  ROM_LOAD( "abasic.ic20", 0x0000, 0x1000, CRC(289b7791) SHA1(0072c83458a9690a3ea1f6094f0f38cf8e96a445) )
//  ROM_CONTINUE(            0x3000, 0x1000 )
//  ROM_LOAD( "afloat.ic21", 0x1000, 0x1000, CRC(81d86af7) SHA1(ebcde5b36cb3a3344567cbba4c7b9fde015f4802) )
//  ROM_LOAD( "atommc2-2.9-a000.rom", 0x2000, 0x1000, CRC(ba73e36c) SHA1(ea9739e96f3283c90b5306288c796fc01144b771) )
//ROM_END


/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS          INIT         COMPANY          FULLNAME               FLAGS */
COMP( 1979, atom,     0,      0,      atom,     atom,  atom_state,    empty_init,  "Acorn",         "Atom",                0)
COMP( 1979, atomeb,   atom,   0,      atomeb,   atom,  atomeb_state,  empty_init,  "Acorn",         "Atom with Eprom Box", 0)
COMP( 1982, atombb,   atom,   0,      atombb,   atom,  atom_state,    empty_init,  "Acorn",         "Atom with BBC Basic", 0)
//COMP( 1983, prophet2, atom,   0,      prophet2, atom,  driver_device, empty_init,  "Busicomputers", "Prophet 2",           0)
//COMP( 1983, prophet3, atom,   0,      prophet3, atom,  driver_device, empty_init,  "Busicomputers", "Prophet 3",           0)
//COMP( 2011, atommc,   atom,   0,      atommc,   atom,  driver_device, empty_init,  "Acorn",         "Atom with AtoMMC2",   0)
