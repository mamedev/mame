// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Dirk Best
/***************************************************************************

    TRS-80 Radio Shack MicroColor Computer

***************************************************************************/


#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/dac.h"
#include "video/mc6847.h"
#include "video/ef9345.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "formats/coco_cas.h"
#include "machine/ram.h"
#include "softlist.h"

//printer state
enum
{
	PRINTER_WAIT,
	PRINTER_REC,
	PRINTER_DONE
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class mc10_state : public driver_device
{
public:
	mc10_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_mc6847(*this, "mc6847"),
	m_ef9345(*this, "ef9345"),
	m_dac(*this, "dac"),
	m_ram(*this, RAM_TAG),
	m_cassette(*this, "cassette"),
	m_printer(*this, "printer")
	{ }

	required_device<m6803_cpu_device> m_maincpu;
	optional_device<mc6847_base_device> m_mc6847;
	optional_device<ef9345_device> m_ef9345;
	required_device<dac_device> m_dac;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cassette;
	required_device<printer_image_device> m_printer;

	UINT8 *m_ram_base;
	UINT32 m_ram_size;
	UINT8 m_keyboard_strobe;
	UINT8 m_port2;

	// printer
	UINT8 m_pr_buffer;
	UINT8 m_pr_counter;
	UINT8 m_pr_state;

	DECLARE_READ8_MEMBER( mc10_bfff_r );
	DECLARE_WRITE8_MEMBER( mc10_bfff_w );
	DECLARE_READ8_MEMBER( alice90_bfff_r );
	DECLARE_WRITE8_MEMBER( alice32_bfff_w );
	DECLARE_READ8_MEMBER( mc10_port1_r );
	DECLARE_WRITE8_MEMBER( mc10_port1_w );
	DECLARE_READ8_MEMBER( mc10_port2_r );
	DECLARE_WRITE8_MEMBER( mc10_port2_w );
	DECLARE_READ8_MEMBER( mc6847_videoram_r );
	DECLARE_DRIVER_INIT(mc10);
	TIMER_DEVICE_CALLBACK_MEMBER(alice32_scanline);
};


/***************************************************************************
    MEMORY MAPPED I/O
***************************************************************************/

READ8_MEMBER( mc10_state::mc10_bfff_r )
{
	UINT8 result = 0xff;

	if (!BIT(m_keyboard_strobe, 0)) result &= ioport("pb0")->read();
	if (!BIT(m_keyboard_strobe, 1)) result &= ioport("pb1")->read();
	if (!BIT(m_keyboard_strobe, 2)) result &= ioport("pb2")->read();
	if (!BIT(m_keyboard_strobe, 3)) result &= ioport("pb3")->read();
	if (!BIT(m_keyboard_strobe, 4)) result &= ioport("pb4")->read();
	if (!BIT(m_keyboard_strobe, 5)) result &= ioport("pb5")->read();
	if (!BIT(m_keyboard_strobe, 6)) result &= ioport("pb6")->read();
	if (!BIT(m_keyboard_strobe, 7)) result &= ioport("pb7")->read();

	return result;
}

READ8_MEMBER( mc10_state::alice90_bfff_r )
{
	UINT8 result = 0xff;

	if (!BIT(m_keyboard_strobe, 7)) result &= ioport("pb7")->read();
	else
	if (!BIT(m_keyboard_strobe, 6)) result &= ioport("pb6")->read();
	else
	if (!BIT(m_keyboard_strobe, 5)) result &= ioport("pb5")->read();
	else
	if (!BIT(m_keyboard_strobe, 4)) result &= ioport("pb4")->read();
	else
	if (!BIT(m_keyboard_strobe, 3)) result &= ioport("pb3")->read();
	else
	if (!BIT(m_keyboard_strobe, 2)) result &= ioport("pb2")->read();
	else
	if (!BIT(m_keyboard_strobe, 1)) result &= ioport("pb1")->read();
	else
	if (!BIT(m_keyboard_strobe, 0)) result &= ioport("pb0")->read();

	return result;
}

WRITE8_MEMBER( mc10_state::mc10_bfff_w )
{
	/* bit 2 to 6, mc6847 mode lines */
	m_mc6847->gm2_w(BIT(data, 2));
	m_mc6847->intext_w(BIT(data, 2));
	m_mc6847->gm1_w(BIT(data, 3));
	m_mc6847->gm0_w(BIT(data, 4));
	m_mc6847->ag_w(BIT(data, 5));
	m_mc6847->css_w(BIT(data, 6));

	/* bit 7, dac output */
	m_dac->write_unsigned8(BIT(data, 7));
}

WRITE8_MEMBER( mc10_state::alice32_bfff_w )
{
	/* bit 7, dac output */
	m_dac->write_unsigned8(BIT(data, 7));
}


/***************************************************************************
    MC6803 I/O
***************************************************************************/

/* keyboard strobe */
READ8_MEMBER( mc10_state::mc10_port1_r )
{
	return m_keyboard_strobe;
}

/* keyboard strobe */
WRITE8_MEMBER( mc10_state::mc10_port1_w )
{
	m_keyboard_strobe = data;
}

READ8_MEMBER( mc10_state::mc10_port2_r )
{
	UINT8 result = 0xeb;

	/* bit 1, keyboard line pa6 */
	if (!BIT(m_keyboard_strobe, 0)) result &= ioport("pb0")->read() >> 5;
	if (!BIT(m_keyboard_strobe, 2)) result &= ioport("pb2")->read() >> 5;
	if (!BIT(m_keyboard_strobe, 7)) result &= ioport("pb7")->read() >> 5;

	/* bit 2, printer ots input */
	result |= (m_printer->is_ready() ? 0 : 4);

	/* bit 3, rs232 input */

	/* bit 4, cassette input */
	result |= ((m_cassette)->input() >= 0 ? 1 : 0) << 4;

	return result;
}

WRITE8_MEMBER( mc10_state::mc10_port2_w )
{
	/* bit 0, cassette & printer output */
	m_cassette->output( BIT(data, 0) ? +1.0 : -1.0);

	switch (m_pr_state)
	{
		case PRINTER_WAIT:
			if (BIT(m_port2, 0) && !BIT(data, 0))
			{
				m_pr_state = PRINTER_REC;
				m_pr_counter = 8;
				m_pr_buffer = 0;
			}
			break;
		case PRINTER_REC:
			if (m_pr_counter--)
				m_pr_buffer |= (BIT(data,0)<<(7-m_pr_counter));
			else
				m_pr_state = PRINTER_DONE;
			break;
		case PRINTER_DONE:
			if (BIT(data,0))
				m_printer->output(m_pr_buffer);
			m_pr_state = PRINTER_WAIT;
			break;
	}

	m_port2 = data;
}


/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

READ8_MEMBER( mc10_state::mc6847_videoram_r )
{
	if (offset == ~0) return 0xff;
	m_mc6847->inv_w(BIT(m_ram_base[offset], 6));
	m_mc6847->as_w(BIT(m_ram_base[offset], 7));

	return m_ram_base[offset];
}

TIMER_DEVICE_CALLBACK_MEMBER(mc10_state::alice32_scanline)
{
	m_ef9345->update_scanline((UINT16)param);
}

/***************************************************************************
    DRIVER INIT
***************************************************************************/

DRIVER_INIT_MEMBER(mc10_state,mc10)
{
	address_space &prg = m_maincpu->space(AS_PROGRAM);

	/* initialize keyboard strobe */
	m_keyboard_strobe = 0x00;

	/* initialize memory */
	m_ram_base = m_ram->pointer();
	m_ram_size = m_ram->size();
	m_pr_state = PRINTER_WAIT;

	membank("bank1")->set_base(m_ram_base);

	/* initialize memory expansion */
	if (m_ram_size == 20*1024)
		membank("bank2")->set_base(m_ram_base + 0x1000);
	else if (m_ram_size == 24*1024)
		membank("bank2")->set_base(m_ram_base + 0x2000);
	else  if (m_ram_size != 32*1024)        //ensure that is not alice90
		prg.nop_readwrite(0x5000, 0x8fff);

	/* register for state saving */
	save_item(NAME(m_keyboard_strobe));

	//for alice32 force port4 DDR to 0xff at startup
	if (!strcmp(machine().system().name, "alice32") || !strcmp(machine().system().name, "alice90"))
		m_maincpu->m6801_io_w(prg, 0x05, 0xff);
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( mc10_mem, AS_PROGRAM, 8 , mc10_state)
	AM_RANGE(0x0100, 0x3fff) AM_NOP /* unused */
	AM_RANGE(0x4000, 0x4fff) AM_RAMBANK("bank1") /* 4k internal ram */
	AM_RANGE(0x5000, 0x8fff) AM_RAMBANK("bank2") /* 16k memory expansion */
	AM_RANGE(0x9000, 0xbffe) AM_NOP /* unused */
	AM_RANGE(0xbfff, 0xbfff) AM_READWRITE(mc10_bfff_r, mc10_bfff_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0x0000) /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mc10_io, AS_IO, 8 , mc10_state)
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_READWRITE(mc10_port1_r, mc10_port1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(mc10_port2_r, mc10_port2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( alice32_mem, AS_PROGRAM, 8 , mc10_state)
	AM_RANGE(0x0100, 0x2fff) AM_NOP /* unused */
	AM_RANGE(0x3000, 0x4fff) AM_RAMBANK("bank1") /* 8k internal ram */
	AM_RANGE(0x5000, 0x8fff) AM_RAMBANK("bank2") /* 16k memory expansion */
	AM_RANGE(0x9000, 0xafff) AM_NOP /* unused */
	AM_RANGE(0xbf20, 0xbf29) AM_DEVREADWRITE("ef9345", ef9345_device, data_r, data_w)
	AM_RANGE(0xbfff, 0xbfff) AM_READWRITE(mc10_bfff_r, alice32_bfff_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION("maincpu", 0x0000) /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( alice90_mem, AS_PROGRAM, 8 , mc10_state)
	AM_RANGE(0x0100, 0x2fff) AM_NOP /* unused */
	AM_RANGE(0x3000, 0xafff) AM_RAMBANK("bank1")    /* 32k internal ram */
	AM_RANGE(0xbf20, 0xbf29) AM_DEVREADWRITE("ef9345", ef9345_device, data_r, data_w)
	AM_RANGE(0xbfff, 0xbfff) AM_READWRITE(alice90_bfff_r, alice32_bfff_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION("maincpu", 0x0000) /* ROM */
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/* MC-10 keyboard

       PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7
  PA6: Ctl N/c Brk N/c N/c N/c N/c Shift
  PA5: 8   9   :   ;   ,   -   .   /
  PA4: 0   1   2   3   4   5   6   7
  PA3: X   Y   Z   N/c N/c N/c Ent Space
  PA2: P   Q   R   S   T   U   V   W
  PA1: H   I   J   K   L   M   N   O
  PA0: @   A   B   C   D   E   F   G
 */

/*  Port                                        Key description                 Emulated key                  Natural key     Shift 1         Shift 2 (Ctrl) */
static INPUT_PORTS_START( mc10 )
	PORT_START("pb0") /* KEY ROW 0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@     INPUT")        PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H     THEN")         PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P     INKEY$")       PORT_CODE(KEYCODE_P)          PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X     SQN")          PORT_CODE(KEYCODE_X)          PORT_CHAR('X')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")                  PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (  CLS")          PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CONTROL")            PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb1") /* KEY ROW 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A     \xE2\x86\x90") PORT_CODE(KEYCODE_A)          PORT_CHAR('A')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I     NEXT")         PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q     L.DEL")        PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y     RESTORE")      PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !  RUN")          PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )  PRINT")        PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb2") /* KEY ROW 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B     ABS")          PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J     GOTO")         PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R     RESET")        PORT_CODE(KEYCODE_R)          PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z     \xE2\x86\x93") PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"  CONT")        PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *  END")          PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(':')  PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK")              PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb3") /* KEY ROW 3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C     INT")          PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K     SOUND")        PORT_CODE(KEYCODE_K)          PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S     \xE2\x86\x92") PORT_CODE(KEYCODE_S)          PORT_CHAR('S')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #  CSAVE")        PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +  POKE")         PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR('+')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb4") /* KEY ROW 4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D     GOSUB")        PORT_CODE(KEYCODE_D)          PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L     PEEK")         PORT_CODE(KEYCODE_L)          PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T     READ")         PORT_CODE(KEYCODE_T)          PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $  CLOAD")        PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <  TAN")          PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb5") /* KEY ROW 5 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E     SET")          PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M     COS")          PORT_CODE(KEYCODE_M)          PORT_CHAR('M')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U     FOR")          PORT_CODE(KEYCODE_U)          PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %  NEW")          PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =  STOP")         PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-')  PORT_CHAR('=')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb6") /* KEY ROW 6 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F     RETURN")       PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N     SIN")          PORT_CODE(KEYCODE_N)          PORT_CHAR('N')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V     RND")          PORT_CODE(KEYCODE_V)          PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER")              PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &  LIST")         PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >  LOG")          PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb7") /* KEY ROW 7 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G     IF")           PORT_CODE(KEYCODE_G)          PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O     STEP")         PORT_CODE(KEYCODE_O)          PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W     \xE2\x86\x91") PORT_CODE(KEYCODE_W)          PORT_CHAR('W')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE")              PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '  CLEAR")        PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('\'')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?  SQR")          PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")              PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/* Alice uses an AZERTY keyboard */
/*  Port                                        Key description                 Emulated key                  Natural key     Shift 1         Shift 2 (Ctrl) */
static INPUT_PORTS_START( alice )
	PORT_START("pb0") /* KEY ROW 0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@     INPUT")        PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H     THEN")         PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P     INKEY$")       PORT_CODE(KEYCODE_P)          PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X     SQN")          PORT_CODE(KEYCODE_X)          PORT_CHAR('X')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")                  PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (  CLS")          PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CONTROL")            PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb1") /* KEY ROW 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q     L.DEL")        PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I     NEXT")         PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A     \xE2\x86\x90") PORT_CODE(KEYCODE_A)          PORT_CHAR('A')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y     RESTORE")      PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !  RUN")          PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )  PRINT")        PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb2") /* KEY ROW 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B     ABS")          PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J     GOTO")         PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R     RESET")        PORT_CODE(KEYCODE_R)          PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W     \xE2\x86\x91") PORT_CODE(KEYCODE_W)          PORT_CHAR('W')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"  CONT")        PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *  END")          PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(':')  PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK")              PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb3") /* KEY ROW 3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C     INT")          PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K     SOUND")        PORT_CODE(KEYCODE_K)          PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S     \xE2\x86\x92") PORT_CODE(KEYCODE_S)          PORT_CHAR('S')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #  CSAVE")        PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M     COS")          PORT_CODE(KEYCODE_M)          PORT_CHAR('M')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb4") /* KEY ROW 4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D     GOSUB")        PORT_CODE(KEYCODE_D)          PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L     PEEK")         PORT_CODE(KEYCODE_L)          PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T     READ")         PORT_CODE(KEYCODE_T)          PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $  CLOAD")        PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <  TAN")          PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb5") /* KEY ROW 5 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E     SET")          PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?  SQR")          PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U     FOR")          PORT_CODE(KEYCODE_U)          PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %  NEW")          PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =  STOP")         PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-')  PORT_CHAR('=')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb6") /* KEY ROW 6 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F     RETURN")       PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N     SIN")          PORT_CODE(KEYCODE_N)          PORT_CHAR('N')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V     RND")          PORT_CODE(KEYCODE_V)          PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER")              PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &  LIST")         PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >  LOG")          PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("pb7") /* KEY ROW 7 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G     IF")           PORT_CODE(KEYCODE_G)          PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O     STEP")         PORT_CODE(KEYCODE_O)          PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z     \xE2\x86\x93") PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')  PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE")              PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '  CLEAR")        PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('\'')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +  POKE")         PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")              PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static MACHINE_CONFIG_START( mc10, mc10_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6803, XTAL_3_579545MHz)  /* 0,894886 MHz */
	MCFG_CPU_PROGRAM_MAP(mc10_mem)
	MCFG_CPU_IO_MAP(mc10_io)

	/* video hardware */
	MCFG_SCREEN_MC6847_NTSC_ADD("screen", "mc6847")

	MCFG_DEVICE_ADD("mc6847", MC6847_NTSC, XTAL_3_579545MHz)
	MCFG_MC6847_INPUT_CALLBACK(READ8(mc10_state, mc6847_videoram_r))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(coco_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)
	MCFG_CASSETTE_INTERFACE("mc10_cass")

	/* printer */
	MCFG_DEVICE_ADD("printer", PRINTER, 0)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("20K")
	MCFG_RAM_EXTRA_OPTIONS("4K")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cass_list", "mc10")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( alice32, mc10_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6803, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(alice32_mem)
	MCFG_CPU_IO_MAP(mc10_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE("ef9345", ef9345_device, screen_update)
	MCFG_SCREEN_SIZE(336, 270)
	MCFG_SCREEN_VISIBLE_AREA(00, 336-1, 00, 270-1)
	MCFG_PALETTE_ADD("palette", 8)

	MCFG_DEVICE_ADD("ef9345", EF9345, 0)
	MCFG_EF9345_PALETTE("palette")
	MCFG_TIMER_DRIVER_ADD_SCANLINE("alice32_sl", mc10_state, alice32_scanline, "screen", 0, 10)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(alice32_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)
	MCFG_CASSETTE_INTERFACE("mc10_cass")

	/* printer */
	MCFG_DEVICE_ADD("printer", PRINTER, 0)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("24K")
	MCFG_RAM_EXTRA_OPTIONS("8K")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cass_list", "alice32")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("mc10_cass", "mc10")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( alice90, alice32 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(alice90_mem)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")

	/* Software lists */
	MCFG_SOFTWARE_LIST_MODIFY("cass_list", "alice90")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("alice32_cass", "alice32")
	MCFG_DEVICE_REMOVE("mc10_cass")
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( mc10 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("mc10.rom", 0x0000, 0x2000, CRC(11fda97e) SHA1(4afff2b4c120334481aab7b02c3552bf76f1bc43))
ROM_END

ROM_START( alice )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("alice.rom", 0x0000, 0x2000, CRC(f876abe9) SHA1(c2166b91e6396a311f486832012aa43e0d2b19f8))
ROM_END

ROM_START( alice32 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("alice32.rom", 0x0000, 0x4000, CRC(c3854ddf) SHA1(f34e61c3cf711fb59ff4f1d4c0d2863dab0ab5d1))

	ROM_REGION( 0x2000, "ef9345", 0 )
	ROM_LOAD( "charset.rom", 0x0000, 0x2000, BAD_DUMP CRC(b2f49eb3) SHA1(d0ef530be33bfc296314e7152302d95fdf9520fc) )            // from dcvg5k
ROM_END

ROM_START( alice90 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("alice90.rom", 0x0000, 0x4000, CRC(d0a874bb) SHA1(a65c7be2d516bed2584c51c1ef78b045b91faef6))

	ROM_REGION( 0x2000, "ef9345", 0 )
	ROM_LOAD( "charset.rom", 0x0000, 0x2000, BAD_DUMP CRC(b2f49eb3) SHA1(d0ef530be33bfc296314e7152302d95fdf9520fc) )            // from dcvg5k
ROM_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  INIT  COMPANY              FULLNAME  FLAGS */
COMP( 1983, mc10,  0,      0,      mc10,    mc10, mc10_state,  mc10, "Tandy Radio Shack", "MC-10",  MACHINE_SUPPORTS_SAVE )
COMP( 1983, alice, mc10,   0,      mc10,    alice, mc10_state, mc10, "Matra & Hachette",  "Alice",  MACHINE_SUPPORTS_SAVE )
COMP( 1984, alice32,       0, 0,   alice32, alice, mc10_state, mc10, "Matra & Hachette",  "Alice 32",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1985, alice90, alice32, 0,   alice90, alice, mc10_state, mc10, "Matra & Hachette",  "Alice 90",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
