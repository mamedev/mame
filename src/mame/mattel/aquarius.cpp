// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Nigel Barnes
/***************************************************************************

    Mattel Aquarius


    TODO:

    - proper video timings, seems to be some contention involved
    - floppy support (I/O 0xe6-0xe7 = drive 1, 0xea-0xeb = drive 2)
    - modem

Dick Smith catalog numbers, taken from advertisements:

X-6000 : Aquarius Computer (2K RAM)
X-6005 : Mini Expander
X-6010 : Data Recorder
X-6015 : 16K RAM cart
X-6020 : 32K RAM cart
X-6025 : Thermal Printer
X-6026 : Roll of paper for the printer

***************************************************************************/

#include "emu.h"
#include "aquarius.h"

#include "softlist_dev.h"
#include "speaker.h"


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
uint8_t aquarius_state::cassette_r()
{
	return ((m_cassette)->input() < +0.0) ? 0 : 1;
}


/*
    Sound and cassette port use a common pin. Therefore the signal to cassette
    will appear on audio output. Sound port is a simple one bit I/O and therefore
    it must be toggled at a specific rate under software control.
*/
void aquarius_state::cassette_w(uint8_t data)
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
uint8_t aquarius_state::vsync_r()
{
	return (m_screen->vpos() < 16 || m_screen->vpos() > 215) ? 0 : 1;
}


/*
    Bit D0 of this port controls the swapping of the lower 16K block in the memory
    map with the upper 16K. A 1 in this bit indicates swapping. This bit is reset
    after power up initialization.
*/
void aquarius_state::mapper_w(uint8_t data)
{
	m_mapper.select(BIT(data, 0));
}


/*
    Printer handshaking port (read) Port 0xFE when read, presents the clear
    to send status from PRNHASK pin at bit D0. A 1 indicates printer is ready,
    0 means not ready.
*/
uint8_t aquarius_state::printer_r()
{
	return m_printer->cts_r(); /* ready */
}


/*
    This is a single bit I/O at D0, it will perform as a serial output
    port under software control. Since timing is done by software the
    baud rate is variable. In BASIC this is a 1200 baud printer port for
    the 40 column thermal printer.
*/
void aquarius_state::printer_w(uint8_t data)
{
	m_printer->write_txd(BIT(data, 0));
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
uint8_t aquarius_state::keyboard_r(offs_t offset)
{
	uint8_t result = 0xff;

	if (!BIT(offset,  8)) result &= m_y[0]->read();
	if (!BIT(offset,  9)) result &= m_y[1]->read();
	if (!BIT(offset, 10)) result &= m_y[2]->read();
	if (!BIT(offset, 11)) result &= m_y[3]->read();
	if (!BIT(offset, 12)) result &= m_y[4]->read();
	if (!BIT(offset, 13)) result &= m_y[5]->read();
	if (!BIT(offset, 14)) result &= m_y[6]->read();
	if (!BIT(offset, 15)) result &= m_y[7]->read();

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
void aquarius_state::scrambler_w(uint8_t data)
{
	m_scrambler = data;
}


/***************************************************************************
    DRIVER INIT
***************************************************************************/

void aquarius_state::machine_start()
{
	save_item(NAME(m_scrambler));
}

void aquarius_state::machine_reset()
{
	/* reset memory mapper after power up */
	m_mapper.select(0);
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void aquarius_state::aquarius_mem(address_map &map)
{
	map(0x0000, 0xffff).view(m_mapper);
	/* Normal mode */
	m_mapper[0](0x0000, 0x2fff).rom().region("maincpu", 0);
	m_mapper[0](0x3000, 0x33ff).ram().w(FUNC(aquarius_state::videoram_w)).share("videoram");
	m_mapper[0](0x3400, 0x37ff).ram().w(FUNC(aquarius_state::colorram_w)).share("colorram");
	m_mapper[0](0x3800, 0x3fff).ram().share("basicram");
	m_mapper[0](0x4000, 0xbfff).lrw8(NAME([this](offs_t offset) { return m_exp->mreq_r(offset) ^ m_scrambler; }), NAME([this](offs_t offset, u8 data) { m_exp->mreq_w(offset, data ^ m_scrambler); }));
	m_mapper[0](0xc000, 0xffff).lrw8(NAME([this](offs_t offset) { return m_exp->mreq_ce_r(offset) ^ m_scrambler; }), NAME([this](offs_t offset, u8 data) { m_exp->mreq_ce_w(offset, data ^ m_scrambler); }));
	/* CP/M mode */
	m_mapper[1](0x0000, 0x3fff).lrw8(NAME([this](offs_t offset) { return m_exp->mreq_ce_r(offset) ^ m_scrambler; }), NAME([this](offs_t offset, u8 data) { m_exp->mreq_ce_w(offset, data ^ m_scrambler); }));
	m_mapper[1](0x4000, 0xbfff).lrw8(NAME([this](offs_t offset) { return m_exp->mreq_r(offset) ^ m_scrambler; }), NAME([this](offs_t offset, u8 data) { m_exp->mreq_w(offset, data ^ m_scrambler); }));
	m_mapper[1](0xc000, 0xefff).rom().region("maincpu", 0);
	m_mapper[1](0xf000, 0xf3ff).ram().w(FUNC(aquarius_state::videoram_w)).share("videoram");
	m_mapper[1](0xf400, 0xf7ff).ram().w(FUNC(aquarius_state::colorram_w)).share("colorram");
	m_mapper[1](0xf800, 0xffff).ram().share("basicram");
}

void aquarius_state::aquarius_io(address_map &map)
{
	map(0x00, 0xff).mirror(0xff00).rw(m_exp, FUNC(aquarius_cartridge_slot_device::iorq_r), FUNC(aquarius_cartridge_slot_device::iorq_w));
//  map(0x7e, 0x7f).mirror(0xff00).rw(FUNC(aquarius_state::modem_r), FUNC(aquarius_state::modem_w));
	map(0xfc, 0xfc).mirror(0xff00).rw(FUNC(aquarius_state::cassette_r), FUNC(aquarius_state::cassette_w));
	map(0xfd, 0xfd).mirror(0xff00).rw(FUNC(aquarius_state::vsync_r), FUNC(aquarius_state::mapper_w));
	map(0xfe, 0xfe).mirror(0xff00).rw(FUNC(aquarius_state::printer_r), FUNC(aquarius_state::printer_w));
	map(0xff, 0xff).select(0xff00).rw(FUNC(aquarius_state::keyboard_r), FUNC(aquarius_state::scrambler_w));
}


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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("= +\tNEXT")   PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2190 \\") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)   PORT_CHAR('\\') // U+2190 = ←
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(": *\tPEEK")   PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RTN")         PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("; @\tPOKE")   PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(". >\tVAL")    PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("- _\tFOR")    PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/ ^")         PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('/') PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 ?")         PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P")           PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(16)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L\tPOINT")    PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(12)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(", <\tSTR$")   PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 )\tCOPY")   PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("O")           PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(15)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K\tPRESET")   PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(11)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M")           PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("N\tRIGHT$")   PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(14)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("J\tPSET")     PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(10)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 (\tRETURN") PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("I")           PORT_CODE(KEYCODE_I)         PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(9)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 '\tGOSUB")  PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("U")           PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(21)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("H")           PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B\tMID$")     PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 &\tON")     PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Y")           PORT_CODE(KEYCODE_Y)         PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(25)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G\tBELL")     PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(7)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("V\tLEFT$")    PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(22)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C\tSTOP")     PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F\tDATA")     PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(6)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 %\tGOTO")   PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("T\tINPUT")    PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(20)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 $\tTHEN")   PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R\tRETYP")    PORT_CODE(KEYCODE_R)         PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(18)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D\tREAD")     PORT_CODE(KEYCODE_D)         PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X\tDELINE")   PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(24)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 #\tIF")     PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E\tDIM")      PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S\tSTPLST")   PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z\tCLOAD")    PORT_CODE(KEYCODE_Z)         PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(26)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE\tCHR$") PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(32)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A\tCSAVE")    PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 \"\tLIST")  PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W\tREM")      PORT_CODE(KEYCODE_W)         PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(23)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 !\tRUN")    PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Q")           PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(17)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT")       PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTL")         PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RST")         PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, aquarius_state, aquarius_reset, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(aquarius_state::gfx_changed)
{
	m_gfx_bank = newval;
	m_tilemap->mark_all_dirty();
}

static INPUT_PORTS_START( aquarius_ar )
	PORT_INCLUDE(aquarius)

	PORT_MODIFY("Y0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2190 \ufefb\t\u2190 \\")  PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)           PORT_CHAR('\\', 0xfefb)               // U+FEFB = ﻻ  U+2190 = ←
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0630 *\t: *\tPEEK")       PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(':', 0x0630) PORT_CHAR('*')                        // U+0630 = ذ
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0636 @\t; @\tPOKE")       PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';', 0x0636) PORT_CHAR('@')                        // U+0636 = ض
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0629 >\t. >\tVAL")        PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.', 0x0629) PORT_CHAR('>')                        // U+0629 = ة

	PORT_MODIFY("Y1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0638 ^\t/ ^")             PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('/', 0x0638) PORT_CHAR('^')                        // U+0638 = ظ
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0633  \u0651\tP")         PORT_CODE(KEYCODE_P)         PORT_CHAR('p', 0x0633) PORT_CHAR('P', 0x0651) PORT_CHAR(16)  // U+0633 = س  U+0651 = shadda (combining, keep preceding space)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0648 \u0624\tL\tPOINT")   PORT_CODE(KEYCODE_L)         PORT_CHAR('l', 0x0648) PORT_CHAR('L', 0x0624) PORT_CHAR(12)  // U+0648 = و  U+0624 = ؤ
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0649 <\t, <\tSTR$")       PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',', 0x0649) PORT_CHAR('<')                        // U+0649 = ى

	PORT_MODIFY("Y2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0639 :\tO")               PORT_CODE(KEYCODE_O)         PORT_CHAR('o', 0x0639) PORT_CHAR('O')         PORT_CHAR(15)  // U+0639 = ع
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0627 \u0623\tK\tPRESET")  PORT_CODE(KEYCODE_K)         PORT_CHAR('k', 0x0627) PORT_CHAR('K', 0x0623) PORT_CHAR(11)  // U+0627 = ا  U+0623 = أ
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u062f \u061f\tM")          PORT_CODE(KEYCODE_M)         PORT_CHAR('m', 0x062f) PORT_CHAR('M', 0x061f) PORT_CHAR(13)  // U+062F = د  U+061F = ؟
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u062b  \u064d\tN\tRIGHT$") PORT_CODE(KEYCODE_N)         PORT_CHAR('n', 0x062b) PORT_CHAR('N', 0x064d) PORT_CHAR(14)  // U+062B = ث  U+064D = kasratan (combining, keep preceding space)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0645  \u064e\tJ\tPSET")   PORT_CODE(KEYCODE_J)         PORT_CHAR('j', 0x0645) PORT_CHAR('J', 0x064e) PORT_CHAR(10)  // U+0645 = م  U+064E = fatha (combining, keep preceding space)

	PORT_MODIFY("Y3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0630 /\tI")               PORT_CODE(KEYCODE_I)         PORT_CHAR('i', 0x0630) PORT_CHAR('I')         PORT_CHAR(9)   // U+0630 = ذ
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0641 \u061b\tU")          PORT_CODE(KEYCODE_U)         PORT_CHAR('u', 0x0641) PORT_CHAR('U', 0x061b) PORT_CHAR(21)  // U+0641 = ف  U+061B = ؛
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u062a  \u064b\tH")         PORT_CODE(KEYCODE_H)         PORT_CHAR('h', 0x062a) PORT_CHAR('H', 0x064b) PORT_CHAR(8)   // U+062A = ت  U+064B = fathatan (combining, keep preceding space)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u062e  \u0650\tB\tMID$")   PORT_CODE(KEYCODE_B)         PORT_CHAR('b', 0x062e) PORT_CHAR('B', 0x0650) PORT_CHAR(2)   // U+062e = خ  U+0650 = kasra (combining, keep preceding space)

	PORT_MODIFY("Y4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0628 \u060c\tY")          PORT_CODE(KEYCODE_Y)         PORT_CHAR('y', 0x0628) PORT_CHAR('Y', 0x060c) PORT_CHAR(25)  // U+0628 = ب  U+060C = ،
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0646  \u0652\tG\tBELL")   PORT_CODE(KEYCODE_G)         PORT_CHAR('g', 0x0646) PORT_CHAR('G', 0x0652) PORT_CHAR(7)   // U+0646 = ن  U+0652 = sukun (combining, keep preceding space)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u063a .\tV\tLEFT$")        PORT_CODE(KEYCODE_V)         PORT_CHAR('v', 0x063a) PORT_CHAR('V')         PORT_CHAR(22)  // U+063A = غ
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0637\tC\tSTOP")           PORT_CODE(KEYCODE_C)         PORT_CHAR('c', 0x0637) PORT_CHAR('C')         PORT_CHAR(3)   // U+0637 = ط
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0631  \u064c\tF\tDATA")   PORT_CODE(KEYCODE_F)         PORT_CHAR('f', 0x0631) PORT_CHAR('F', 0x064c) PORT_CHAR(6)   // U+0631 = ر  U+064C = dammatan (combining, keep preceding space)

	PORT_MODIFY("Y5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u062d \\\tT\tINPUT")       PORT_CODE(KEYCODE_T)         PORT_CHAR('t', 0x062d) PORT_CHAR('T')         PORT_CHAR(20)  // U+062D = ح
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0647 \u0640\tR\tRETYP")   PORT_CODE(KEYCODE_R)         PORT_CHAR('r', 0x0647) PORT_CHAR('R', 0x0640) PORT_CHAR(18)  // U+0647 = ه  U+0640 = tatweel
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0644  \u064f\tD\tREAD")   PORT_CODE(KEYCODE_D)         PORT_CHAR('d', 0x0644) PORT_CHAR('D', 0x064f) PORT_CHAR(4)   // U+0644 = ل  U+064F = damma (combining, keep preceding space)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0635\tX\tDELINE")         PORT_CODE(KEYCODE_X)         PORT_CHAR('x', 0x0635) PORT_CHAR('X')         PORT_CHAR(24)  // U+0635 = ص

	PORT_MODIFY("Y6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0621  \u0654\tE\tDIM")    PORT_CODE(KEYCODE_E)         PORT_CHAR('e', 0x0621) PORT_CHAR('E', 0x0654) PORT_CHAR(5)   // U+0621 = ء  U+0654 = hamza above (combining, keep preceding space)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u064a \u0625\tS\tSTPLST")  PORT_CODE(KEYCODE_S)         PORT_CHAR('s', 0x064a) PORT_CHAR('S', 0x0625) PORT_CHAR(19)  // U+064A = ي  U+0625 = إ
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0634\tZ\tCLOAD")          PORT_CODE(KEYCODE_Z)         PORT_CHAR('z', 0x0634) PORT_CHAR('Z')         PORT_CHAR(26)  // U+0634 = ش
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0642 \u0622\tA\tCSAVE")   PORT_CODE(KEYCODE_A)         PORT_CHAR('a', 0x0642) PORT_CHAR('A', 0x0622) PORT_CHAR(1)   // U+0642 = ق  U+0622 = آ

	PORT_MODIFY("Y7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u0643\tW\tREM")            PORT_CODE(KEYCODE_W)         PORT_CHAR('w', 0x0643) PORT_CHAR('W')         PORT_CHAR(23)  // U+0643 = ك
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u062c\tQ")                 PORT_CODE(KEYCODE_Q)         PORT_CHAR('q', 0x062c) PORT_CHAR('Q')         PORT_CHAR(17)  // U+062C = ج

	PORT_START("GFX")
	PORT_CONFNAME(0x01, 0x00, "Character ROM") PORT_CHANGED_MEMBER(DEVICE_SELF, aquarius_state, gfx_changed, 0)
	PORT_CONFSETTING(0x00, "(G)raphics")
	PORT_CONFSETTING(0x01, "(A)rabic")
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

static const gfx_layout aquarius_ar_charlayout =
{
	8, 8,
	512,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, },
	8 * 8
};

/* Graphics Decode Information */

static GFXDECODE_START( gfx_aquarius )
	GFXDECODE_ENTRY( "gfx1", 0x0000, aquarius_charlayout, 0, 256 )
GFXDECODE_END

static GFXDECODE_START( gfx_aquarius_ar )
	GFXDECODE_ENTRY( "gfx1", 0x0000, aquarius_ar_charlayout, 0, 256 )
GFXDECODE_END


/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

static DEVICE_INPUT_DEFAULTS_START(printer)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_1200)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_1200)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_2)
DEVICE_INPUT_DEFAULTS_END

void aquarius_state::cfg_ram16(device_t* device)
{
	device->subdevice<aquarius_cartridge_slot_device>("exp2")->set_default_option("ram16");
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void aquarius_state::aquarius(machine_config &config)
{
	Z80(config, m_maincpu, 7.15909_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &aquarius_state::aquarius_mem);
	m_maincpu->set_addrmap(AS_IO, &aquarius_state::aquarius_io);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(7.15909_MHz_XTAL, 458, 0, 352, 262, 0, 232);
	m_screen->set_screen_update(FUNC(aquarius_state::screen_update_aquarius));
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);
	m_screen->set_palette(m_palette);
	m_screen->scanline().set([this](int scanline) { m_maincpu->adjust_icount(-4); }); // TODO: this tries to compensate for contention, needs a better understanding of video timings

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_aquarius);
	TEA1002(config, m_tea1002, 7.15909_MHz_XTAL);
	PALETTE(config, m_palette, FUNC(aquarius_state::aquarius_palette), 512, 16);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_formats(aquarius_cassette_formats);
	m_cassette->set_interface("aquarius_cass");

	RS232_PORT(config, m_printer, default_rs232_devices, "printer");
	m_printer->set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(printer));

	AQUARIUS_CARTRIDGE_SLOT(config, m_exp, 7.15909_MHz_XTAL / 2, aquarius_cartridge_devices, "mini");
	m_exp->set_option_machine_config("mini", cfg_ram16);
	m_exp->irq_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_exp->nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	SOFTWARE_LIST(config, "cart_list").set_original("aquarius_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("aquarius_cass");
}

void aquarius_state::aquariusp(machine_config &config)
{
	aquarius(config);

	m_screen->set_raw(7.15909_MHz_XTAL, 458, 0, 352, 312, 0, 232);

	m_tea1002->set_unscaled_clock(8.867238_MHz_XTAL);
}

void aquarius_state::aquarius_ar(machine_config &config)
{
	aquariusp(config);

	m_gfxdecode->set_info(gfx_aquarius_ar);
}


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( aquarius )
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASE00)

	/* basic rom */
	ROM_DEFAULT_BIOS("s2")
	ROM_SYSTEM_BIOS(0, "s2", "S2")
	ROMX_LOAD("aq_s2.u2", 0x0000, 0x2000, CRC(5cfa5b42) SHA1(02c8ee11e911d1aa346812492d14284b6870cb3e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "s1", "S1")
	ROMX_LOAD("aq.u2", 0x0000, 0x2000, CRC(28d0fdbd) SHA1(58019da049b611a07adc6456cc9d77d92423d62a), ROM_BIOS(1))

	/* charrom */
	ROM_REGION(0x0800, "gfx1", 0)
	ROM_LOAD("aq2.u5", 0x0000, 0x0800, CRC(e117f57c) SHA1(3588c0267c67dfbbda615bcf8dc3d3a5c5bd815a))
ROM_END

#define rom_aquariusp rom_aquarius

ROM_START( aquarius2 )
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASE00)

	/* extended basic rom */
	ROM_LOAD("aq2_1.rom", 0x0000, 0x2000, CRC(5cfa5b42) SHA1(02c8ee11e911d1aa346812492d14284b6870cb3e))
	ROM_LOAD("aq2_2.rom", 0x2000, 0x1000, CRC(c95117c6) SHA1(6ee8571a93b9b371dfdd26334ae886a69c5b3daf))

	/* charrom */
	ROM_REGION(0x0800, "gfx1", 0)
	ROM_LOAD("aq2.u5", 0x0000, 0x0800, CRC(e117f57c) SHA1(3588c0267c67dfbbda615bcf8dc3d3a5c5bd815a))
ROM_END

ROM_START( aquarius_ar )
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASE00)

	/* basic rom */
	ROM_LOAD("aq_s2.u2", 0x0000, 0x2000, CRC(5cfa5b42) SHA1(02c8ee11e911d1aa346812492d14284b6870cb3e))

	/* charrom */
	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("xz-2120-1_v1.2.u5", 0x0000, 0x1000, CRC(09102213) SHA1(4919f84cb57df000910035b08c35315c732052bb))
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME         PARENT    COMPAT  MACHINE      INPUT        CLASS           INIT        COMPANY                FULLNAME              FLAGS
COMP( 1983, aquarius,    0,        0,      aquarius,    aquarius,    aquarius_state, empty_init, "Mattel Electronics",  "Aquarius (NTSC)",    0 )
COMP( 1983, aquariusp,   aquarius, 0,      aquariusp,   aquarius,    aquarius_state, empty_init, "Mattel Electronics",  "Aquarius (PAL)",     0 )
COMP( 1984, aquarius2,   aquarius, 0,      aquarius,    aquarius,    aquarius_state, empty_init, "Radofin",             "Aquarius II",        0 )
COMP( 198?, aquarius_ar, aquarius, 0,      aquarius_ar, aquarius_ar, aquarius_state, empty_init, "Ecico Electronics",   "Aquarius (Arabic)",  0 )
