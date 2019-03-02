// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Dirk Best
/***************************************************************************

    TRS-80 Radio Shack MicroColor Computer

***************************************************************************/


#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/ef9345.h"
#include "video/mc6847.h"

#include "softlist.h"
#include "speaker.h"

#include "formats/coco_cas.h"


namespace
{

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class mc10_state : public driver_device
{
public:
	mc10_state(const machine_config &mconfig, device_type type, const char *tag);

	void alice90(machine_config &config);
	void alice32(machine_config &config);
	void mc10(machine_config &config);

private:
	DECLARE_READ8_MEMBER(mc10_bfff_r);
	DECLARE_WRITE8_MEMBER(mc10_bfff_w);
	DECLARE_READ8_MEMBER(mc10_port1_r);
	DECLARE_WRITE8_MEMBER(mc10_port1_w);
	DECLARE_READ8_MEMBER(mc10_port2_r);
	DECLARE_WRITE8_MEMBER(mc10_port2_w);
	DECLARE_READ8_MEMBER(alice90_bfff_r);
	DECLARE_WRITE8_MEMBER(alice32_bfff_w);

	DECLARE_READ8_MEMBER(mc6847_videoram_r);
	TIMER_DEVICE_CALLBACK_MEMBER(alice32_scanline);

	void alice32_mem(address_map &map);
	void alice90_mem(address_map &map);
	void mc10_mem(address_map &map);

	// device-level overrides
	virtual void driver_start() override;

	//printer state
	enum class printer_state : uint8_t
	{
		WAIT,
		REC,
		DONE
	};

	required_device<m6803_cpu_device> m_maincpu;
	optional_device<mc6847_base_device> m_mc6847;
	optional_device<ef9345_device> m_ef9345;
	required_device<dac_bit_interface> m_dac;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cassette;
	required_device<printer_image_device> m_printer;
	required_ioport_array<8> m_pb;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank2;

	uint8_t *m_ram_base;
	uint32_t m_ram_size;
	uint8_t m_keyboard_strobe;
	uint8_t m_port2;

	// printer
	uint8_t m_pr_buffer;
	uint8_t m_pr_counter;
	printer_state m_pr_state;

	uint8_t read_keyboard_strobe(bool single_line);
};


/***************************************************************************
    MEMORY MAPPED I/O
***************************************************************************/

uint8_t mc10_state::read_keyboard_strobe(bool single_line)
{
	bool read = false;
	uint8_t result = 0xff;

	for (int i = m_pb.size() - 1; (i >= 0) && (!read || !single_line); i--)
	{
		if (!BIT(m_keyboard_strobe, i))
		{
			result &= m_pb[i]->read();
			read = true;
		}
	}
	return result;
}


READ8_MEMBER( mc10_state::mc10_bfff_r )
{
	return read_keyboard_strobe(false);
}

READ8_MEMBER( mc10_state::alice90_bfff_r )
{
	return read_keyboard_strobe(true);
}

WRITE8_MEMBER( mc10_state::mc10_bfff_w )
{
	// bit 2 to 6, mc6847 mode lines
	m_mc6847->gm2_w(BIT(data, 2));
	m_mc6847->intext_w(BIT(data, 2));
	m_mc6847->gm1_w(BIT(data, 3));
	m_mc6847->gm0_w(BIT(data, 4));
	m_mc6847->ag_w(BIT(data, 5));
	m_mc6847->css_w(BIT(data, 6));

	// bit 7, dac output
	m_dac->write(BIT(data, 7));
}

WRITE8_MEMBER( mc10_state::alice32_bfff_w )
{
	// bit 7, dac output
	m_dac->write(BIT(data, 7));
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
	uint8_t result = 0xeb;

	// bit 1, keyboard line pa6
	if (!BIT(m_keyboard_strobe, 0)) result &= m_pb[0]->read() >> 5;
	if (!BIT(m_keyboard_strobe, 2)) result &= m_pb[2]->read() >> 5;
	if (!BIT(m_keyboard_strobe, 7)) result &= m_pb[7]->read() >> 5;

	// bit 2, printer ots input
	result |= (m_printer->is_ready() ? 1 : 0) << 2;

	// bit 3, rs232 input

	// bit 4, cassette input
	result |= (m_cassette->input() >= 0 ? 1 : 0) << 4;

	return result;
}

WRITE8_MEMBER( mc10_state::mc10_port2_w )
{
	// bit 0, cassette & printer output
	m_cassette->output( BIT(data, 0) ? +1.0 : -1.0);

	switch (m_pr_state)
	{
		case printer_state::WAIT:
			if (BIT(m_port2, 0) && !BIT(data, 0))
			{
				m_pr_state = printer_state::REC;
				m_pr_counter = 8;
				m_pr_buffer = 0;
			}
			break;
		case printer_state::REC:
			if (m_pr_counter--)
				m_pr_buffer |= (BIT(data,0)<<(7-m_pr_counter));
			else
				m_pr_state = printer_state::DONE;
			break;
		case printer_state::DONE:
			if (BIT(data,0))
				m_printer->output(m_pr_buffer);
			m_pr_state = printer_state::WAIT;
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
	m_ef9345->update_scanline((uint16_t)param);
}

/***************************************************************************
    DRIVER INIT
***************************************************************************/

mc10_state::mc10_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, "maincpu")
	, m_mc6847(*this, "mc6847")
	, m_ef9345(*this, "ef9345")
	, m_dac(*this, "dac")
	, m_ram(*this, RAM_TAG)
	, m_cassette(*this, "cassette")
	, m_printer(*this, "printer")
	, m_pb(*this, "pb%u", 0)
	, m_bank1(*this, "bank1")
	, m_bank2(*this, "bank2")
{
}


void mc10_state::driver_start()
{
	// call base device_start
	driver_device::driver_start();

	address_space &prg = m_maincpu->space(AS_PROGRAM);

	/* initialize keyboard strobe */
	m_keyboard_strobe = 0x00;

	/* initialize memory */
	m_ram_base = m_ram->pointer();
	m_ram_size = m_ram->size();
	m_pr_state = printer_state::WAIT;

	m_bank1->set_base(m_ram_base);

	/* initialize memory expansion */
	if (m_bank2)
	{
		if (m_ram_size == 20 * 1024)
			m_bank2->set_base(m_ram_base + 0x1000);
		else if (m_ram_size == 24 * 1024)
			m_bank2->set_base(m_ram_base + 0x2000);
		else if (m_ram_size != 32 * 1024)        //ensure that is not alice90
			prg.nop_readwrite(0x5000, 0x8fff);
	}

	/* register for state saving */
	save_item(NAME(m_keyboard_strobe));
	save_item(NAME(m_pr_state));
	save_item(NAME(m_pr_counter));

	//for alice32 force port4 DDR to 0xff at startup
	if (!strcmp(machine().system().name, "alice32") || !strcmp(machine().system().name, "alice90"))
		m_maincpu->m6801_io_w(prg, 0x05, 0xff);
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mc10_state::mc10_mem(address_map &map)
{
	map(0x0100, 0x3fff).noprw(); /* unused */
	map(0x4000, 0x4fff).bankrw("bank1"); /* 4k internal ram */
	map(0x5000, 0x8fff).bankrw("bank2"); /* 16k memory expansion */
	map(0x9000, 0xbffe).noprw(); /* unused */
	map(0xbfff, 0xbfff).rw(FUNC(mc10_state::mc10_bfff_r), FUNC(mc10_state::mc10_bfff_w));
	map(0xe000, 0xffff).rom().region("maincpu", 0x0000); /* ROM */
}

void mc10_state::alice32_mem(address_map &map)
{
	map(0x0100, 0x2fff).noprw(); /* unused */
	map(0x3000, 0x4fff).bankrw("bank1"); /* 8k internal ram */
	map(0x5000, 0x8fff).bankrw("bank2"); /* 16k memory expansion */
	map(0x9000, 0xafff).noprw(); /* unused */
	map(0xbf20, 0xbf29).rw(m_ef9345, FUNC(ef9345_device::data_r), FUNC(ef9345_device::data_w));
	map(0xbfff, 0xbfff).rw(FUNC(mc10_state::mc10_bfff_r), FUNC(mc10_state::alice32_bfff_w));
	map(0xc000, 0xffff).rom().region("maincpu", 0x0000); /* ROM */
}

void mc10_state::alice90_mem(address_map &map)
{
	map(0x0100, 0x2fff).noprw(); /* unused */
	map(0x3000, 0xafff).bankrw("bank1");    /* 32k internal ram */
	map(0xbf20, 0xbf29).rw(m_ef9345, FUNC(ef9345_device::data_r), FUNC(ef9345_device::data_w));
	map(0xbfff, 0xbfff).rw(FUNC(mc10_state::alice90_bfff_r), FUNC(mc10_state::alice32_bfff_w));
	map(0xc000, 0xffff).rom().region("maincpu", 0x0000); /* ROM */
}

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
INPUT_PORTS_START( mc10 )
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
INPUT_PORTS_START( alice )
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

void mc10_state::mc10(machine_config &config)
{
	/* basic machine hardware */
	M6803(config, m_maincpu, XTAL(3'579'545));  /* 0,894886 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mc10_state::mc10_mem);
	m_maincpu->in_p1_cb().set(FUNC(mc10_state::mc10_port1_r));
	m_maincpu->out_p1_cb().set(FUNC(mc10_state::mc10_port1_w));
	m_maincpu->in_p2_cb().set(FUNC(mc10_state::mc10_port2_r));
	m_maincpu->out_p2_cb().set(FUNC(mc10_state::mc10_port2_w));

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	mc6847_ntsc_device &vdg(MC6847_NTSC(config, "mc6847", XTAL(3'579'545)));
	vdg.set_screen("screen");
	vdg.input_callback().set(FUNC(mc10_state::mc6847_videoram_r));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.0625);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(coco_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->set_interface("mc10_cass");

	/* printer */
	PRINTER(config, m_printer, 0);

	/* internal ram */
	RAM(config, m_ram).set_default_size("20K").set_extra_options("4K");

	/* Software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("mc10");
}

void mc10_state::alice32(machine_config &config)
{
	M6803(config, m_maincpu, XTAL(3'579'545));
	m_maincpu->set_addrmap(AS_PROGRAM, &mc10_state::alice32_mem);
	m_maincpu->in_p1_cb().set(FUNC(mc10_state::mc10_port1_r));
	m_maincpu->out_p1_cb().set(FUNC(mc10_state::mc10_port1_w));
	m_maincpu->in_p2_cb().set(FUNC(mc10_state::mc10_port2_r));
	m_maincpu->out_p2_cb().set(FUNC(mc10_state::mc10_port2_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update("ef9345", FUNC(ef9345_device::screen_update));
	screen.set_size(336, 270);
	screen.set_visarea(00, 336-1, 00, 270-1);
	PALETTE(config, "palette").set_entries(8);

	EF9345(config, m_ef9345, 0);
	m_ef9345->set_palette_tag("palette");
	TIMER(config, "alice32_sl").configure_scanline(FUNC(mc10_state::alice32_scanline), "screen", 0, 10);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.0625);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(alice32_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->set_interface("mc10_cass");

	/* printer */
	PRINTER(config, m_printer, 0);

	/* internal ram */
	RAM(config, m_ram).set_default_size("24K").set_extra_options("8K");

	/* Software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("alice32");
	SOFTWARE_LIST(config, "mc10_cass").set_compatible("mc10");
}

void mc10_state::alice90(machine_config &config)
{
	alice32(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mc10_state::alice90_mem);

	m_ram->set_default_size("32K");

	/* Software lists */
	subdevice<software_list_device>("cass_list")->set_original("alice90");
	SOFTWARE_LIST(config, "alice32_cass").set_compatible("alice32");
	config.device_remove("mc10_cass");
}

}


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

ALLOW_SAVE_TYPE(mc10_state::printer_state);

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY              FULLNAME     FLAGS
COMP( 1983, mc10,    0,       0,      mc10,    mc10,  mc10_state, empty_init, "Tandy Radio Shack", "MC-10",     MACHINE_SUPPORTS_SAVE )
COMP( 1983, alice,   mc10,    0,      mc10,    alice, mc10_state, empty_init, "Matra & Hachette",  "Alice",     MACHINE_SUPPORTS_SAVE )
COMP( 1984, alice32, 0,       0,      alice32, alice, mc10_state, empty_init, "Matra & Hachette",  "Alice 32",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1985, alice90, alice32, 0,      alice90, alice, mc10_state, empty_init, "Matra & Hachette",  "Alice 90",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
