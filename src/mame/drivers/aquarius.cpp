// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    Mattel Aquarius


    TODO:

    - slot interface for cartridges
    - hand controllers
    - scramble RAM also
    - CAQ tape support
    - memory mapper
    - proper video timings
    - PAL mode
    - floppy support (I/O 0xe6-0xe7 = drive 1, 0xea-0xeb = drive 2)
    - modem
    - "old" version of BASIC ROM
    - Aquarius II

***************************************************************************/

#include "includes/aquarius.h"
#include "softlist.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define XTAL1   8866000
#define XTAL2   XTAL_7_15909MHz


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/



/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

/*
    To read stored data from cassette the program should look at bit zero of
    the cassette input port and measure the time difference between leading
    edges or trailing edges. This is to prevent DC level shifting from altering
    pulse width of data. The program should then look for sync bytes for data
    synchronisation before data block transfer. If there is any task that must
    be performed during cassette loading, the maximum allowable time to do the
    job after one byte from cassette, must be less than 80% of the period of a
    mark cycle. Control must be returned at that time to the cassette routine
    in order to maintain data integrity.
*/
READ8_MEMBER(aquarius_state::cassette_r)
{
	return ((m_cassette)->input() < +0.0) ? 0 : 1;
}


/*
    Sound and cassette port use a common pin. Therefore the signal to cassette
    will appear on audio output. Sound port is a simple one bit I/O and therefore
    it must be toggled at a specific rate under software control.
*/
WRITE8_MEMBER(aquarius_state::cassette_w)
{
	m_speaker->level_w(BIT(data, 0));
	m_cassette->output(BIT(data, 0) ? +1.0 : -1.0);
}


/*
    The current state of the vertical sync will appear on bit 0 during a read of
    this port. The waveform and timing spec is shown as follows:

        |<-    Active scan period   ->|V.sync |<-
        |      12.8 ms                |3.6 ms (PAL)
        |                             |2.8 ms (NTSC)
        +++++++++++++++++++++++++++++++       ++++++++++++
        +                             +       +
        +                             +       +
    +++++                             +++++++++
*/
READ8_MEMBER(aquarius_state::vsync_r)
{
	return m_screen->vblank() ? 0 : 1;
}


/*
    Bit D0 of this port controls the swapping of the lower 16K block in the memory
    map with the upper 16K. A 1 in this bit indicates swapping. This bit is reset
    after power up initialization.
*/
WRITE8_MEMBER(aquarius_state::mapper_w)
{
}


/*
    Printer handshaking port (read) Port 0xFE when read, presents the clear
    to send status from PRNHASK pin at bit D0. A 1 indicates printer is ready,
    0 means not ready.
*/
READ8_MEMBER(aquarius_state::printer_r)
{
	return 1; /* ready */
}


/*
    This is a single bit I/O at D0, it will perform as a serial output
    port under software control. Since timing is done by software the
    baudrate is variable. In BASIC this is a 1200 baud printer port for
    the 40 column thermal printer.
*/
WRITE8_MEMBER(aquarius_state::printer_w)
{
}


/*
    This port is 6 bits wide, when read, it returns the row data from the
    keyboard matrix. The keyboard is usually scanned in the following manner:

    The keyboard is a 6 row by 8 column matrix. The column is connected to
    the higher order address bus A15-A8. When Z80 executes its input
    instruction sets, either the current content of the accumulator (A) or
    the content of register (B) will go to the higher order address bus.
    Therefore the keyboard can be scanned by placing a specific scanning
    pattern in (A) or (B) and reading the result returned on rows.
*/
READ8_MEMBER(aquarius_state::keyboard_r)
{
	UINT8 result = 0xff;

	if (!BIT(offset,  8)) result &= m_y0->read();
	if (!BIT(offset,  9)) result &= m_y1->read();
	if (!BIT(offset, 10)) result &= m_y2->read();
	if (!BIT(offset, 11)) result &= m_y3->read();
	if (!BIT(offset, 12)) result &= m_y4->read();
	if (!BIT(offset, 13)) result &= m_y5->read();
	if (!BIT(offset, 14)) result &= m_y6->read();
	if (!BIT(offset, 15)) result &= m_y7->read();

	return result;
}


/*
    Software lock: Writing this port with an 8 bit value will set the software
    scrambler pattern. The data that appears on the output side will be the
    result of the input bus EX-ORed with this pattern, bit by bit. The software
    lock is a scrambler built between the CPU and external interface. The
    scrambling pattern is contained in port 0xFF and is not readable. CPU data
    output to external bus will be XORed with this pattern in a bit by bit
    fashion to generate the real data on external bus. By the same mechanism,
    data from external bus is also XORed with this pattern and read by CPU.

    Therefore it the external device is RAM, the software lock simply has no
    effect as long as the scrambling pattern remains unchanged. For I/O
    operation the pattern is gated to 0 and thus scrambling action is nullified.

    In BASIC operation the scrambling pattern is generated by a random number
    routine. For game cartridge the lock pattern is generated from data in the
    game cartridge itself.
*/
WRITE8_MEMBER(aquarius_state::scrambler_w)
{
	m_scrambler = data;
}

READ8_MEMBER(aquarius_state::cartridge_r)
{
	UINT8 data = 0;
	if (m_cart->exists())
		data = m_cart->read_rom(space, offset);

	return data ^ m_scrambler;
}


/***************************************************************************
    DRIVER INIT
***************************************************************************/

DRIVER_INIT_MEMBER(aquarius_state,aquarius)
{
	/* install expansion memory if available */
	if (m_ram->size() > 0x1000)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		space.install_readwrite_bank(0x4000, 0x4000 + m_ram->size() - 0x1000 - 1, "bank1");
		membank("bank1")->set_base(m_ram->pointer());
	}
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( aquarius_mem, AS_PROGRAM, 8, aquarius_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_RAM_WRITE(aquarius_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x3400, 0x37ff) AM_RAM_WRITE(aquarius_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x3800, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0xbfff) AM_NOP /* expansion ram */
	AM_RANGE(0xc000, 0xffff) AM_READ(cartridge_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( aquarius_io, AS_IO, 8, aquarius_state )
//  AM_RANGE(0x7e, 0x7f) AM_MIRROR(0xff00) AM_READWRITE(modem_r, modem_w)
	AM_RANGE(0xf6, 0xf6) AM_MIRROR(0xff00) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, data_w)
	AM_RANGE(0xf7, 0xf7) AM_MIRROR(0xff00) AM_DEVWRITE("ay8910", ay8910_device, address_w)
	AM_RANGE(0xfc, 0xfc) AM_MIRROR(0xff00) AM_READWRITE(cassette_r, cassette_w)
	AM_RANGE(0xfd, 0xfd) AM_MIRROR(0xff00) AM_READWRITE(vsync_r, mapper_w)
	AM_RANGE(0xfe, 0xfe) AM_MIRROR(0xff00) AM_READWRITE(printer_r, printer_w)
	AM_RANGE(0xff, 0xff) AM_MIRROR(0xff00) AM_MASK(0xff00) AM_READWRITE(keyboard_r, scrambler_w)
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

/* the 'reset' key is directly tied to the reset line of the cpu */
INPUT_CHANGED_MEMBER(aquarius_state::aquarius_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( aquarius )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("= +\tNEXT") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90 \\") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR('\\')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(": *\tPEEK") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RTN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("; @\tPOKE") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(". >\tVAL") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("- _\tFOR") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/ ^") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('/') PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 ?") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(16)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L\tPOINT") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(12)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(", <\tSTR$") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 )\tCOPY") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(15)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K\tPRESET") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(11)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("N\tRIGHT$") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(14)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("J\tPSET") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(10)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 (\tRETURN") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(9)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 '\tGOSUB") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(21)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B\tMID$") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 &\tON") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(25)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G\tBELL") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(7)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("V\tLEFT$") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(22)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C\tSTOP") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F\tDATA") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(6)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 %\tGOTO") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("T\tINPUT") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(20)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 $\tTHEN") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R\tRETYP") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(18)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D\tREAD") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X\tDELINE") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(24)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 #\tIF") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E\tDIM") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S\tSTPLST") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z\tCLOAD") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(26)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE\tCHR$") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A\tCSAVE") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 \"\tLIST") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W\tREM") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(23)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 !\tRUN") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(17)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RST") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, aquarius_state, aquarius_reset, 0)

	PORT_START("LEFT")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("RIGHT")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
    CHARACTER LAYOUT
***************************************************************************/

static const gfx_layout aquarius_charlayout =
{
	8, 8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, },
	8 * 8
};

/* Graphics Decode Information */

static GFXDECODE_START( aquarius )
	GFXDECODE_ENTRY( "gfx1", 0x0000, aquarius_charlayout, 0, 256 )
GFXDECODE_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static MACHINE_CONFIG_START( aquarius, aquarius_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_3_579545MHz) // ???
	MCFG_CPU_PROGRAM_MAP(aquarius_mem)
	MCFG_CPU_IO_MAP(aquarius_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", aquarius_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2800))
	MCFG_SCREEN_SIZE(40 * 8, 25 * 8)
	MCFG_SCREEN_VISIBLE_AREA(0, 40 * 8 - 1, 0 * 8, 25 * 8 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(aquarius_state, screen_update_aquarius)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", aquarius )
	MCFG_TEA1002_ADD("encoder", XTAL_8_867238MHz)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INDIRECT_ENTRIES(16)
	MCFG_PALETTE_INIT_OWNER(aquarius_state, aquarius)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_3_579545MHz/2) // ??? AY-3-8914
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("RIGHT"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("LEFT"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "aquarius_cart")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4K")
	MCFG_RAM_EXTRA_OPTIONS("8K,20K,36K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","aquarius")
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( aquarius )
	ROM_REGION(0x10000, "maincpu", 0)

	/* basic rom */
	ROM_DEFAULT_BIOS("rev2")
	ROM_SYSTEM_BIOS(0, "rev1", "Revision 1")
	ROMX_LOAD("aq1.u2", 0x0000, 0x2000, NO_DUMP, ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rev2", "Revision 2")
	ROMX_LOAD("aq2.u2", 0x0000, 0x2000, CRC(a2d15bcf) SHA1(ca6ef55e9ead41453efbf5062d6a60285e9661a6), ROM_BIOS(2))

	/* charrom */
	ROM_REGION(0x800, "gfx1", 0)
	ROM_LOAD("aq2.u5", 0x000, 0x800, CRC(e117f57c) SHA1(3588c0267c67dfbbda615bcf8dc3d3a5c5bd815a))
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME         PARENT    COMPAT  MACHINE      INPUT     INIT      COMPANY   FULLNAME                         FLAGS */
COMP( 1983, aquarius,    0,        0,      aquarius,    aquarius, aquarius_state, aquarius, "Mattel", "Aquarius (NTSC)",               0 )
//COMP( 1984,   aquariu2,   aquarius,   0,      aquarius,   aquarius, driver_device,   0,  "Mattel",   "Aquarius II",  MACHINE_NOT_WORKING )
