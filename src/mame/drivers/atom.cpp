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
#include "formats/imageutl.h"
#include "softlist.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 1

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    QUICKLOAD_LOAD_MEMBER( atom_state, atom_atm )
-------------------------------------------------*/

QUICKLOAD_LOAD_MEMBER( atom_state, atom_atm )
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

	UINT8 header[0x16] = { 0 };
	void *ptr;

	image.fread(header, 0x16);

	UINT16 start_address = pick_integer_le(header, 0x10, 2);
	UINT16 run_address = pick_integer_le(header, 0x12, 2);
	UINT16 size = pick_integer_le(header, 0x14, 2);

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

	return IMAGE_INIT_PASS;
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
		return m_ext[m_eprom & 0x0f]->read_rom(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 dor_r - read DOS roms at 0xe000
 -------------------------------------------------*/

READ8_MEMBER( atomeb_state::dos_r )
{
	if (m_e0->exists() && !BIT(m_eprom, 7))
		return m_e0->read_rom(space, offset);
	else if (m_e1->exists() && BIT(m_eprom, 7))
		return m_e1->read_rom(space, offset);
	else
		return 0xff;
}


/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( atom_mem )
-------------------------------------------------*/

static ADDRESS_MAP_START( atom_mem, AS_PROGRAM, 8, atom_state )
	AM_RANGE(0x0000, 0x09ff) AM_RAM
	AM_RANGE(0x0a00, 0x0a03) AM_MIRROR(0x1f8) AM_DEVICE(I8271_TAG, i8271_device, map)
	AM_RANGE(0x0a04, 0x0a04) AM_MIRROR(0x1f8) AM_DEVREADWRITE(I8271_TAG, i8271_device, data_r, data_w)
	AM_RANGE(0x0a05, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x97ff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0x9800, 0x9fff) AM_RAM
//  AM_RANGE(0xa000, 0xafff)        // mapped by the cartslot
	AM_RANGE(0xb000, 0xb003) AM_MIRROR(0x3fc) AM_DEVREADWRITE(INS8255_TAG, i8255_device, read, write)
//  AM_RANGE(0xb400, 0xb403) AM_DEVREADWRITE(MC6854_TAG, mc6854_device, read, write)
//  AM_RANGE(0xb404, 0xb404) AM_READ_PORT("ECONET")
	AM_RANGE(0xb800, 0xb80f) AM_MIRROR(0x3f0) AM_DEVREADWRITE(R6522_TAG, via6522_device, read, write)
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION(SY6502_TAG, 0)
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( atomeb_mem )
-------------------------------------------------*/

static ADDRESS_MAP_START( atomeb_mem, AS_PROGRAM, 8, atomeb_state )
	AM_IMPORT_FROM(atom_mem)
	AM_RANGE(0xa000, 0xafff) AM_READ(ext_r)
	AM_RANGE(0xbfff, 0xbfff) AM_READWRITE(eprom_r, eprom_w)
	AM_RANGE(0xe000, 0xefff) AM_READ(dos_r)
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( atombb_mem )
-------------------------------------------------*/

static ADDRESS_MAP_START( atombb_mem, AS_PROGRAM, 8, atom_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x57ff) AM_RAM AM_SHARE("video_ram")

	AM_RANGE(0x7000, 0x7003) AM_MIRROR(0x3fc) AM_DEVREADWRITE(INS8255_TAG, i8255_device, read, write)
	AM_RANGE(0x7800, 0x780f) AM_MIRROR(0x3f0) AM_DEVREADWRITE(R6522_TAG, via6522_device, read, write)
	AM_RANGE(0x8000, 0xbfff) AM_ROM AM_REGION("basic", 0)
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(SY6502_TAG, 0)
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( prophet2_mem )
-------------------------------------------------*/

//static ADDRESS_MAP_START( prophet2_mem, AS_PROGRAM, 8, atom_state )
//  AM_RANGE(0x0000, 0x09ff) AM_RAM
//  AM_RANGE(0x0a00, 0x7fff) AM_RAM
//  AM_RANGE(0x8000, 0x97ff) AM_RAM AM_SHARE("video_ram")
//  AM_RANGE(0x9800, 0x9fff) AM_RAM
//  AM_RANGE(0xb000, 0xb003) AM_MIRROR(0x3fc) AM_DEVREADWRITE(INS8255_TAG, i8255_device, read, write)
////  AM_RANGE(0xb400, 0xb403) AM_DEVREADWRITE(MC6854_TAG, mc6854_device, read, write)
////  AM_RANGE(0xb404, 0xb404) AM_READ_PORT("ECONET")
//  AM_RANGE(0xb800, 0xb80f) AM_MIRROR(0x3f0) AM_DEVREADWRITE(R6522_TAG, via6522_device, read, write)
//  AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION(SY6502_TAG, 0)
//ADDRESS_MAP_END

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

	UINT8 data = 0xff;

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

	UINT8 data = 0;

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
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		}
	}

	m_previous_i8271_int_state = state;
}

WRITE_LINE_MEMBER( atom_state::motor_w )
{
	for (int i=0; i != 2; i++) {
		char devname[1];
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
	UINT8 *m_baseram = (UINT8 *)m_maincpu->space(AS_PROGRAM).get_write_ptr(0x0000);

	m_baseram[0x08] = machine().rand() & 0x0ff;
	m_baseram[0x09] = machine().rand() & 0x0ff;
	m_baseram[0x0a] = machine().rand() & 0x0ff;
	m_baseram[0x0b] = machine().rand() & 0x0ff;

	if (m_cart && m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xa000, 0xafff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));
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

int atom_state::load_cart(device_image_interface &image, generic_slot_device *slot)
{
	UINT32 size = slot->common_get_size("rom");

	if (size > 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	slot->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}

static SLOT_INTERFACE_START(atom_floppies)
	SLOT_INTERFACE("525sssd", FLOPPY_525_SSSD)
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER(atom_state::floppy_formats)
	FLOPPY_ATOM_FORMAT
FLOPPY_FORMATS_END0

/*-------------------------------------------------
    MACHINE_DRIVER( atom )
-------------------------------------------------*/

static MACHINE_CONFIG_START( atom, atom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(SY6502_TAG, M6502, X2/4)
	MCFG_CPU_PROGRAM_MAP(atom_mem)

	/* video hardware */
	MCFG_SCREEN_MC6847_PAL_ADD(SCREEN_TAG, MC6847_TAG)

	MCFG_DEVICE_ADD(MC6847_TAG, MC6847_PAL, XTAL_4_433619MHz)
	MCFG_MC6847_INPUT_CALLBACK(READ8(atom_state, vdg_videoram_r))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("hz2400", atom_state, cassette_output_tick, attotime::from_hz(4806))

	MCFG_DEVICE_ADD(R6522_TAG, VIA6522, X2/4)
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_strobe))
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE(SY6502_TAG, m6502_device, irq_line))

	MCFG_DEVICE_ADD(INS8255_TAG, I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(atom_state, ppi_pa_w))
	MCFG_I8255_IN_PORTB_CB(READ8(atom_state, ppi_pb_r))
	MCFG_I8255_IN_PORTC_CB(READ8(atom_state, ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(atom_state, ppi_pc_w))

	MCFG_DEVICE_ADD(I8271_TAG, I8271 , 0)
	MCFG_I8271_IRQ_CALLBACK(WRITELINE(atom_state, atom_8271_interrupt_callback))
	MCFG_I8271_HDL_CALLBACK(WRITELINE(atom_state, motor_w))
	MCFG_FLOPPY_DRIVE_ADD(I8271_TAG ":0", atom_floppies, "525sssd", atom_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(I8271_TAG ":1", atom_floppies, "525sssd", atom_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE(R6522_TAG, via6522_device, write_ca1))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE(R6522_TAG, via6522_device, write_pa7))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(atom_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)
	MCFG_CASSETTE_INTERFACE("atom_cass")

	MCFG_QUICKLOAD_ADD("quickload", atom_state, atom_atm, "atm", 0)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "atom_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(atom_state, cart_load)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("2K,4K,6K,8K,10K,12K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("rom_list","atom_rom")
	MCFG_SOFTWARE_LIST_ADD("cass_list","atom_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_list","atom_flop")
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_DRIVER( atomeb )
-------------------------------------------------*/

#define MCFG_ATOM_ROM_ADD(_tag, _load) \
	MCFG_GENERIC_SOCKET_ADD(_tag, generic_linear_slot, "atom_cart") \
	MCFG_GENERIC_EXTENSIONS("bin,rom") \
	MCFG_GENERIC_LOAD(atomeb_state, _load)

static MACHINE_CONFIG_DERIVED_CLASS( atomeb, atom, atomeb_state )
	MCFG_CPU_MODIFY(SY6502_TAG)
	MCFG_CPU_PROGRAM_MAP(atomeb_mem)

	/* cartridges */
	MCFG_DEVICE_REMOVE("cartslot")

	MCFG_ATOM_ROM_ADD("rom_a0", a0_load)
	MCFG_ATOM_ROM_ADD("rom_a1", a1_load)
	MCFG_ATOM_ROM_ADD("rom_a2", a2_load)
	MCFG_ATOM_ROM_ADD("rom_a3", a3_load)
	MCFG_ATOM_ROM_ADD("rom_a4", a4_load)
	MCFG_ATOM_ROM_ADD("rom_a5", a5_load)
	MCFG_ATOM_ROM_ADD("rom_a6", a6_load)
	MCFG_ATOM_ROM_ADD("rom_a7", a7_load)
	MCFG_ATOM_ROM_ADD("rom_a8", a8_load)
	MCFG_ATOM_ROM_ADD("rom_a9", a9_load)
	MCFG_ATOM_ROM_ADD("rom_aa", aa_load)
	MCFG_ATOM_ROM_ADD("rom_ab", ab_load)
	MCFG_ATOM_ROM_ADD("rom_ac", ac_load)
	MCFG_ATOM_ROM_ADD("rom_ad", ad_load)
	MCFG_ATOM_ROM_ADD("rom_ae", ae_load)
	MCFG_ATOM_ROM_ADD("rom_af", af_load)

	MCFG_ATOM_ROM_ADD("rom_e0", e0_load)
	MCFG_ATOM_ROM_ADD("rom_e1", e1_load)
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_DRIVER( atombb )
-------------------------------------------------*/

static MACHINE_CONFIG_START( atombb, atom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(SY6502_TAG, M6502, X2/4)
	MCFG_CPU_PROGRAM_MAP(atombb_mem)

	/* video hardware */
	MCFG_SCREEN_MC6847_PAL_ADD(SCREEN_TAG, MC6847_TAG)

	MCFG_DEVICE_ADD(MC6847_TAG, MC6847_PAL, XTAL_4_433619MHz)
	MCFG_MC6847_INPUT_CALLBACK(READ8(atom_state, vdg_videoram_r))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("hz2400", atom_state, cassette_output_tick, attotime::from_hz(4806))

	MCFG_DEVICE_ADD(R6522_TAG, VIA6522, X2/4)
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_strobe))
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE(SY6502_TAG, m6502_device, irq_line))

	MCFG_DEVICE_ADD(INS8255_TAG, I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(atom_state, ppi_pa_w))
	MCFG_I8255_IN_PORTB_CB(READ8(atom_state, ppi_pb_r))
	MCFG_I8255_IN_PORTC_CB(READ8(atom_state, ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(atom_state, ppi_pc_w))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE(R6522_TAG, via6522_device, write_ca1))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE(R6522_TAG, via6522_device, write_pa7))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(atom_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)
	MCFG_CASSETTE_INTERFACE("atom_cass")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("8K,12K")

MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_DRIVER( prophet2 )
-------------------------------------------------*/

//static MACHINE_CONFIG_DERIVED( prophet2, atom )
//  /* basic machine hardware */
//  MCFG_CPU_MODIFY(SY6502_TAG)
//  MCFG_CPU_PROGRAM_MAP(prophet2_mem)
//
//  /* fdc */
//  MCFG_DEVICE_REMOVE(I8271_TAG)
//  MCFG_DEVICE_REMOVE(I8271_TAG ":0")
//  MCFG_DEVICE_REMOVE(I8271_TAG ":1")
//
//  /* Software lists */
//  MCFG_SOFTWARE_LIST_REMOVE("flop_list")
//MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_DRIVER( prophet3 )
-------------------------------------------------*/

//static MACHINE_CONFIG_DERIVED( prophet3, atom )
//
//MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_DRIVER( atommc )
-------------------------------------------------*/

//static MACHINE_CONFIG_DERIVED( atommc, atom )
//  /* Software lists */
//  MCFG_SOFTWARE_LIST_ADD("mmc_list","atom_mmc")
//  MCFG_SOFTWARE_LIST_REMOVE("flop_list")
//MACHINE_CONFIG_END

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

DRIVER_INIT_MEMBER(atomeb_state, atomeb)
{
	// these have to be set here, so that we can pass m_ext[*] to device_image_load!
	char str[8];
	for (int i = 0; i < 16; i++)
	{
		sprintf(str,"rom_a%x", i);
		m_ext[i] = machine().device<generic_slot_device>(str);
	}
}


/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT CLASS         INIT           COMPANY          FULLNAME                FLAGS */
COMP( 1979, atom,     0,        0,      atom,     atom, driver_device,     0,        "Acorn",         "Atom"                , 0)
COMP( 1979, atomeb,   atom,     0,      atomeb,   atom, atomeb_state, atomeb,        "Acorn",         "Atom with Eprom Box" , 0)
COMP( 1982, atombb,   atom,     0,      atombb,   atom, driver_device,     0,        "Acorn",         "Atom with BBC Basic" , 0)
//COMP( 1983, prophet2, atom,     0,      prophet2, atom, driver_device,     0,        "Busicomputers", "Prophet 2"           , 0)
//COMP( 1983, prophet3, atom,     0,      prophet3, atom, driver_device,     0,        "Busicomputers", "Prophet 3"           , 0)
//COMP( 2011, atommc,   atom,     0,      atommc,   atom, driver_device,     0,        "Acorn",         "Atom with AtoMMC2"   , 0)
