// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Dirk Best, tim lindner
/***************************************************************************

    TRS-80 Radio Shack MicroColor Computer

***************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "bus/mc10/mc10_cart.h"
#include "bus/rs232/rs232.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "video/ef9345.h"
#include "video/mc6847.h"

#include "softlist_dev.h"
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
	mc10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_mc10cart(*this, "ext")
		, m_mc6847(*this, "mc6847")
		, m_dac(*this, "dac")
		, m_cassette(*this, "cassette")
		, m_rs232(*this, "rs232")
		, m_pb(*this, "pb%u", 0U)
	{}

	void mc10_base(machine_config &config);
	void mc10_video(machine_config &config);
	void mc10(machine_config &config);
	void alice(machine_config &config);
	void mc10_bfff_w_dac(uint8_t data);
	uint8_t mc10_bfff_r();

protected:
	void mc10_bfff_w(uint8_t data);

	uint8_t mc10_port1_r();
	void mc10_port1_w(uint8_t data);
	uint8_t mc10_port2_r();
	void mc10_port2_w(uint8_t data);

	uint8_t mc6847_videoram_r(offs_t offset);

	// device-level overrides
	virtual void driver_start() override;
	virtual void driver_reset() override;

	required_device<m6803_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	optional_device<mc10cart_slot_device> m_mc10cart;

	uint8_t m_keyboard_strobe = 0;
	uint8_t m_port2 = 0;

	uint8_t read_keyboard_strobe(bool single_line);

	mc10cart_slot_device &mc10cart() { return *m_mc10cart; }

private:
	void mc10_mem(address_map &map) ATTR_COLD;

	optional_device<mc6847_base_device> m_mc6847;
	required_device<dac_bit_interface> m_dac;
	required_device<cassette_image_device> m_cassette;
	required_device<rs232_port_device> m_rs232;
	required_ioport_array<8> m_pb;
};

class alice32_state : public mc10_state
{
public:
	alice32_state(const machine_config &mconfig, device_type type, const char *tag)
		: mc10_state(mconfig, type, tag)
		, m_ef9345(*this, "ef9345")
	{}

	void alice32(machine_config &config);
	void alice90(machine_config &config);
	void alice32_bfff_w(uint8_t data);
	uint8_t alice90_bfff_r();

protected:
	// device-level overrides
	virtual void driver_start() override;

	TIMER_DEVICE_CALLBACK_MEMBER(alice32_scanline);
	required_device<ef9345_device> m_ef9345;

private:
	void alice32_mem(address_map &map) ATTR_COLD;
	void alice90_mem(address_map &map) ATTR_COLD;
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

uint8_t mc10_state::mc10_bfff_r()
{
	return read_keyboard_strobe(false);
}

uint8_t alice32_state::alice90_bfff_r()
{
	return read_keyboard_strobe(true);
}

void mc10_state::mc10_bfff_w(uint8_t data)
{
	// bit 2 to 6, mc6847 mode lines
	m_mc6847->gm2_w(BIT(data, 2));
	m_mc6847->intext_w(BIT(data, 2));
	m_mc6847->gm1_w(BIT(data, 3));
	m_mc6847->gm0_w(BIT(data, 4));
	m_mc6847->ag_w(BIT(data, 5));
	m_mc6847->css_w(BIT(data, 6));

	mc10_bfff_w_dac(data);
}

void mc10_state::mc10_bfff_w_dac(uint8_t data)
{
	// bit 7, dac output
	m_dac->write(BIT(data, 7));
}

void alice32_state::alice32_bfff_w(uint8_t data)
{
	mc10_bfff_w_dac(data);
}

/***************************************************************************
    MC6803 I/O
***************************************************************************/

uint8_t mc10_state::mc10_port1_r()
{
	return m_keyboard_strobe;
}

void mc10_state::mc10_port1_w(uint8_t data)
{
	m_keyboard_strobe = data;
}

uint8_t mc10_state::mc10_port2_r()
{
	uint8_t result = 0xeb;

	// bit 1, keyboard line pa6
	if (!BIT(m_keyboard_strobe, 0)) result &= m_pb[0]->read() >> 5;
	if (!BIT(m_keyboard_strobe, 2)) result &= m_pb[2]->read() >> 5;
	if (!BIT(m_keyboard_strobe, 7)) result &= m_pb[7]->read() >> 5;

	// bit 2, rs232 input
	result |= (m_rs232->rxd_r() ? 1 : 0) << 2;

	// bit 3, printer ots input
	result |= (m_rs232->cts_r() ? 1 : 0) << 3;

	// bit 4, cassette input
	result |= (m_cassette->input() >= 0 ? 1 : 0) << 4;

	return result;
}

void mc10_state::mc10_port2_w(uint8_t data)
{
	// bit 0, cassette & printer output
	m_cassette->output( BIT(data, 0) ? +1.0 : -1.0);
	m_rs232->write_txd(BIT(data, 0) ? 1 : 0);

	m_port2 = data;
}


/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

uint8_t mc10_state::mc6847_videoram_r(offs_t offset)
{
	if (offset == ~0) return 0xff;

	uint8_t result = m_ram->read(offset);
	m_mc6847->inv_w(BIT(result, 6));
	m_mc6847->as_w(BIT(result, 7));
	return result;
}

TIMER_DEVICE_CALLBACK_MEMBER(alice32_state::alice32_scanline)
{
	m_ef9345->update_scanline((uint16_t)param);
}

/***************************************************************************
    DRIVER INIT
***************************************************************************/

void mc10_state::driver_reset()
{
	m_keyboard_strobe = 0x00;
}

void mc10_state::driver_start()
{
	save_item(NAME(m_keyboard_strobe));

	address_space &space = m_maincpu->space(AS_PROGRAM);
	u32 ram_size = m_ram->size();

	// don't step on hardware page at 0xbf00
	if (ram_size == 0x8000)
		ram_size -= 0x100;

	space.install_ram(0x4000, 0x4000+ram_size-1, m_ram->pointer());
}

void alice32_state::driver_start()
{
	save_item(NAME(m_keyboard_strobe));

	address_space &space = m_maincpu->space(AS_PROGRAM);
	u32 ram_size = m_ram->size();

	space.install_ram(0x3000, 0x3000+ram_size-1, m_ram->pointer());
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mc10_state::mc10_mem(address_map &map)
{
	// mc10 / alice: RAM start at 0x4000, installed in driver_start
	map(0xbfff, 0xbfff).rw(FUNC(mc10_state::mc10_bfff_r), FUNC(mc10_state::mc10_bfff_w));
	map(0xe000, 0xffff).rom().region("maincpu", 0x0000);
}

void alice32_state::alice32_mem(address_map &map)
{
	// alice32: RAM start at 0x3000, installed in driver_start
	map(0xbf20, 0xbf29).rw(m_ef9345, FUNC(ef9345_device::data_r), FUNC(ef9345_device::data_w));
	map(0xbfff, 0xbfff).rw(FUNC(mc10_state::mc10_bfff_r), FUNC(alice32_state::alice32_bfff_w));
	map(0xc000, 0xffff).rom().region("maincpu", 0x0000);
}

void alice32_state::alice90_mem(address_map &map)
{
	// alice90: RAM start at 0x3000, installed in driver_start
	map(0xbf20, 0xbf29).rw(m_ef9345, FUNC(ef9345_device::data_r), FUNC(ef9345_device::data_w));
	map(0xbfff, 0xbfff).rw(FUNC(alice32_state::alice90_bfff_r), FUNC(alice32_state::alice32_bfff_w));
	map(0xc000, 0xffff).rom().region("maincpu", 0x0000);
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

static DEVICE_INPUT_DEFAULTS_START( printer )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void mc10_state::mc10_base(machine_config &config)
{
	/* basic machine hardware */
	M6803(config, m_maincpu, XTAL(3'579'545));  /* 0,894886 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mc10_state::mc10_mem);
	m_maincpu->in_p1_cb().set(FUNC(mc10_state::mc10_port1_r));
	m_maincpu->out_p1_cb().set(FUNC(mc10_state::mc10_port1_w));
	m_maincpu->in_p2_cb().set(FUNC(mc10_state::mc10_port2_r));
	m_maincpu->out_p2_cb().set(FUNC(mc10_state::mc10_port2_w));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.0625);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(alice32_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "speaker", 0.05);
	m_cassette->set_interface("mc10_cass");

	/* printer */
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "rs_printer"));
	rs232.set_option_device_input_defaults("rs_printer", DEVICE_INPUT_DEFAULTS_NAME(printer));
}

void mc10_state::mc10_video(machine_config &config)
{
	/* internal ram */
	RAM(config, m_ram).set_default_size("4K").set_extra_options("8K,20K,32K");

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	mc6847_ntsc_device &vdg(MC6847_NTSC(config, "mc6847", XTAL(3'579'545)));
	vdg.set_screen("screen");
	vdg.input_callback().set(FUNC(mc10_state::mc6847_videoram_r));
}

void mc10_state::mc10(machine_config &config)
{
	mc10_base(config);
	mc10_video(config);

	/* expansion port hardware */
	mc10cart_slot_device &cartslot(MC10CART_SLOT(config, "ext", DERIVED_CLOCK(1, 1), mc10_cart_add_basic_devices, nullptr));
	cartslot.set_memspace(m_maincpu, AS_PROGRAM);
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* Software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("mc10");
}

void mc10_state::alice(machine_config &config)
{
	mc10_base(config);
	mc10_video(config);

	/* expansion port hardware */
	mc10cart_slot_device &cartslot(MC10CART_SLOT(config, "ext", DERIVED_CLOCK(1, 1), alice_cart_add_basic_devices, nullptr));
	cartslot.set_memspace(m_maincpu, AS_PROGRAM);
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* Software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("mc10");
}

void alice32_state::alice32(machine_config &config)
{
	mc10_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &alice32_state::alice32_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update("ef9345", FUNC(ef9345_device::screen_update));
	screen.set_size(336, 270);
	screen.set_visarea(00, 336-1, 00, 270-1);
	PALETTE(config, "palette").set_entries(8);

	EF9345(config, m_ef9345, 0);
	m_ef9345->set_screen("screen");
	m_ef9345->set_palette_tag("palette");
	TIMER(config, "alice32_sl").configure_scanline(FUNC(alice32_state::alice32_scanline), "screen", 0, 10);

	/* expansion port hardware */
	mc10cart_slot_device &cartslot(MC10CART_SLOT(config, "ext", DERIVED_CLOCK(1, 1), alice32_cart_add_basic_devices, nullptr));
	cartslot.set_memspace(m_maincpu, AS_PROGRAM);
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	/* internal ram */
	RAM(config, m_ram).set_default_size("8K").set_extra_options("24K");

	/* Software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("alice32");
	SOFTWARE_LIST(config, "mc10_cass").set_compatible("mc10");
}

void alice32_state::alice90(machine_config &config)
{
	alice32(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &alice32_state::alice90_mem);

	/* internal ram */
	m_ram->set_default_size("32K");

	/* Software lists */
	subdevice<software_list_device>("cass_list")->set_original("alice90");
	SOFTWARE_LIST(config, "alice32_cass").set_compatible("alice32");
	config.device_remove("mc10_cass");
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

#define rom_alice90 rom_alice32

}

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT  CLASS          INIT        COMPANY              FULLNAME     FLAGS
COMP( 1983, mc10,    0,       0,      mc10,    mc10,  mc10_state,    empty_init, "Tandy Radio Shack", "MC-10",     MACHINE_SUPPORTS_SAVE )
COMP( 1983, alice,   mc10,    0,      alice,   alice, mc10_state,    empty_init, "Matra & Hachette",  "Alice",     MACHINE_SUPPORTS_SAVE )
COMP( 1984, alice32, 0,       0,      alice32, alice, alice32_state, empty_init, "Matra & Hachette",  "Alice 32",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1985, alice90, alice32, 0,      alice90, alice, alice32_state, empty_init, "Matra & Hachette",  "Alice 90",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
