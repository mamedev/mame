// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith
/*
    drivers/mbc55x.cpp

    Machine driver for the Sanyo MBC-550 and MBC-555.

    Phill Harvey-Smith
    2011-01-29.


ToDo:
- Fix the sound
- Add serial uart

*/


#include "emu.h"
#include "includes/mbc55x.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
//#include "bus/pc_joy/pc_joy.h"
#include "bus/rs232/rs232.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/clock.h"
#include "machine/i8087.h"
#include "machine/input_merger.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

const unsigned char mbc55x_palette[SCREEN_NO_COLOURS][3] =
{
	/*normal brightness */
	{ 0x00,0x00,0x00 }, /* black */
	{ 0x00,0x00,0x80 }, /* blue */
	{ 0x00,0x80,0x00 }, /* green */
	{ 0x00,0x80,0x80 }, /* cyan */
	{ 0x80,0x00,0x00 }, /* red */
	{ 0x80,0x00,0x80 }, /* magenta */
	{ 0x80,0x80,0x00 }, /* yellow */
	{ 0x80,0x80,0x80 }, /* light grey */
};


void mbc55x_state::mbc55x_mem(address_map &map)
{
	map(0x00000, 0x0FFFF).bankrw(RAM_BANK00_TAG);
	map(0x10000, 0x1FFFF).bankrw(RAM_BANK01_TAG);
	map(0x20000, 0x2FFFF).bankrw(RAM_BANK02_TAG);
	map(0x30000, 0x3FFFF).bankrw(RAM_BANK03_TAG);
	map(0x40000, 0x4FFFF).bankrw(RAM_BANK04_TAG);
	map(0x50000, 0x5FFFF).bankrw(RAM_BANK05_TAG);
	map(0x60000, 0x6FFFF).bankrw(RAM_BANK06_TAG);
	map(0x70000, 0x7FFFF).bankrw(RAM_BANK07_TAG);
	map(0x80000, 0x8FFFF).bankrw(RAM_BANK08_TAG);
	map(0x90000, 0x9FFFF).bankrw(RAM_BANK09_TAG);
	map(0xA0000, 0xAFFFF).bankrw(RAM_BANK0A_TAG);
	map(0xB0000, 0xBFFFF).bankrw(RAM_BANK0B_TAG);
	map(0xC0000, 0xCFFFF).bankrw(RAM_BANK0C_TAG);
	map(0xD0000, 0xDFFFF).bankrw(RAM_BANK0D_TAG);
	map(0xE0000, 0xEFFFF).bankrw(RAM_BANK0E_TAG);
	map(0xF0000, 0xF3FFF).bankrw(RED_PLANE_TAG);
	map(0xF4000, 0xF7FFF).bankrw(BLUE_PLANE_TAG);
	map(0xF8000, 0xFBFFF).noprw();
	map(0xFC000, 0xFDFFF).rom().nopw().region(MAINCPU_TAG, 0x0000).mirror(0x002000);
}

void mbc55x_state::mbc55x_io(address_map &map)
{
	map(0x0000, 0x0000).select(0x003e).rw(FUNC(mbc55x_state::iodecode_r), FUNC(mbc55x_state::iodecode_w));
}

void mbc55x_state::mbc55x_iodecode(address_map &map)
{
	map(0x00, 0x01).mirror(0x02).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x04, 0x07).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x08, 0x08).mirror(0x03).rw(FUNC(mbc55x_state::vram_page_r), FUNC(mbc55x_state::vram_page_w));
	map(0x0c, 0x0f).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x14, 0x15).mirror(0x02).rw("sio", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x18, 0x18).mirror(0x02).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x19, 0x19).mirror(0x02).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x1c, 0x1d).mirror(0x02).r(FUNC(mbc55x_state::mbc55x_kb_usart_r)).w(m_kb_uart, FUNC(i8251_device::write));
}

static INPUT_PORTS_START( mbc55x )
	PORT_START("KEY0") /* Key row 0 scancodes 00..07 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")      PORT_CODE(KEYCODE_1)            PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")      PORT_CODE(KEYCODE_2)            PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")      PORT_CODE(KEYCODE_3)            PORT_CHAR('3')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")      PORT_CODE(KEYCODE_4)            PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")      PORT_CODE(KEYCODE_5)            PORT_CHAR('5')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")      PORT_CODE(KEYCODE_6)            PORT_CHAR('6')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")      PORT_CODE(KEYCODE_7)            PORT_CHAR('7')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")      PORT_CODE(KEYCODE_8)            PORT_CHAR('8')

	PORT_START("KEY1") /* Key row 1 scancodes 08..0F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")      PORT_CODE(KEYCODE_9)            PORT_CHAR('9')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")      PORT_CODE(KEYCODE_0)            PORT_CHAR('0')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")      PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=")      PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BSLASH") PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('\\')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")      PORT_CODE(KEYCODE_Q)            PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")      PORT_CODE(KEYCODE_W)            PORT_CHAR('W')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")      PORT_CODE(KEYCODE_E)            PORT_CHAR('E')

	PORT_START("KEY2") /* Key row 2 scancodes 10..17 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")      PORT_CODE(KEYCODE_R)            PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")      PORT_CODE(KEYCODE_T)            PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")      PORT_CODE(KEYCODE_Y)            PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")      PORT_CODE(KEYCODE_U)            PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")      PORT_CODE(KEYCODE_I)            PORT_CHAR('I')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")      PORT_CODE(KEYCODE_O)            PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")      PORT_CODE(KEYCODE_P)            PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[")      PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[')


	PORT_START("KEY3") /* Key row 3 scancodes 18..1F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]")      PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")      PORT_CODE(KEYCODE_A)            PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")      PORT_CODE(KEYCODE_S)            PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")      PORT_CODE(KEYCODE_D)            PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")      PORT_CODE(KEYCODE_F)            PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")      PORT_CODE(KEYCODE_G)            PORT_CHAR('G')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")      PORT_CODE(KEYCODE_H)            PORT_CHAR('H')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")      PORT_CODE(KEYCODE_J)            PORT_CHAR('J')

	PORT_START("KEY4") /* Key row 4 scancodes 20..27 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")      PORT_CODE(KEYCODE_K)            PORT_CHAR('K')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")      PORT_CODE(KEYCODE_L)            PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";")      PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TILDE")  PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('`')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("#")      PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER")  PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(0x0D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")      PORT_CODE(KEYCODE_Z)            PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")      PORT_CODE(KEYCODE_X)            PORT_CHAR('X')


	PORT_START("KEY5") /* Key row 5 scancodes 28..2F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")      PORT_CODE(KEYCODE_C)            PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")      PORT_CODE(KEYCODE_V)            PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")      PORT_CODE(KEYCODE_B)            PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")      PORT_CODE(KEYCODE_N)            PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")      PORT_CODE(KEYCODE_M)            PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")      PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")      PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")      PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/')

	PORT_START("KEY6") /* Key row 6 scancodes 30..37 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE")  PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                     PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

#if 0
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BACKSLASH") PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('\\')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)

	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")   PORT_CODE(KEYCODE_LCONTROL)     PORT_CODE(KEYCODE_RCONTROL) // Ether control

	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT)       PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PRSCR")  PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR('*')

	PORT_START("KEY7") /* Key row 7 scancodes 38..3F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALT")    PORT_CODE(KEYCODE_LALT)         PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS")   PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")     PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")     PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")     PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")     PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")     PORT_CODE(KEYCODE_F5)

	PORT_START("KEY8") /* Key row 8 scancodes 40..47 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6")     PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7")     PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F8")     PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F9")     PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F10")    PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NUMLK")  PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SCRLK")  PORT_CODE(KEYCODE_SCRLOCK)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP7")    PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR('7')

	PORT_START("KEY9") /* Key row 9 scancodes 48..4F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP8")    PORT_CODE(KEYCODE_8_PAD)        //PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP9")    PORT_CODE(KEYCODE_9_PAD)        //PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-")    PORT_CODE(KEYCODE_MINUS_PAD)    //PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP4")    PORT_CODE(KEYCODE_4_PAD)        //PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP5")    PORT_CODE(KEYCODE_5_PAD)        //PORT_CHAR('5')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP6")    PORT_CODE(KEYCODE_6_PAD)        //PORT_CHAR('6')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP+")    PORT_CODE(KEYCODE_PLUS_PAD)     //PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP1")    PORT_CODE(KEYCODE_1_PAD)        //PORT_CHAR('1')

	PORT_START("KEY10") /* Key row 10 scancodes 50..57 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP2")    PORT_CODE(KEYCODE_2_PAD)        //PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP3")    PORT_CODE(KEYCODE_3_PAD)        //PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP0")    PORT_CODE(KEYCODE_0_PAD)        //PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP.")    PORT_CODE(KEYCODE_DEL_PAD)      //PORT_CHAR('.')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP5")    PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR('5')
	//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP6")    PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR('6')
	//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP+")    PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR('+')
	//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP1")    PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR('1')
#endif

	PORT_START(KEY_SPECIAL_TAG)
	PORT_BIT(KEY_BIT_LSHIFT,    IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(KEY_BIT_RSHIFT,    IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT)       PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(KEY_BIT_CTRL,      IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CTRL")   PORT_CODE(KEYCODE_LCONTROL)     PORT_CODE(KEYCODE_RCONTROL) // Ether control
	PORT_BIT(KEY_BIT_GRAPH,     IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("GRAPH")  PORT_CODE(KEYCODE_LALT)         PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END


PALETTE_INIT_MEMBER(mbc55x_state, mbc55x)
{
	int colourno;

	logerror("initializing palette\n");

	for ( colourno = 0; colourno < SCREEN_NO_COLOURS; colourno++ )
		palette.set_pen_color(colourno, mbc55x_palette[colourno][RED], mbc55x_palette[colourno][GREEN], mbc55x_palette[colourno][BLUE]);
}


FLOPPY_FORMATS_MEMBER( mbc55x_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END


// MBC-550 : 1 x 5.25" disk-drive (160 KB)
// MBC-555 : 2 x 5.25" disk-drive (160 KB)
// MBC-555-2 : 2 x 5.25" disk-drive (360 KB)
// MBC-555-3 : 2 x 5.25" disk-drive (720 KB)

static void mbc55x_floppies(device_slot_interface &device)
{
	device.option_add("ssdd", FLOPPY_525_SSDD);
	device.option_add("dd", FLOPPY_525_DD);
	device.option_add("qd", FLOPPY_525_QD);
}


MACHINE_CONFIG_START(mbc55x_state::mbc55x)
	/* basic machine hardware */
	I8088(config, m_maincpu, 14.318181_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &mbc55x_state::mbc55x_mem);
	m_maincpu->set_addrmap(AS_IO, &mbc55x_state::mbc55x_io);
	m_maincpu->set_irq_acknowledge_callback(PIC8259_TAG, FUNC(pic8259_device::inta_cb));
	m_maincpu->esc_opcode_handler().set("coproc", FUNC(i8087_device::insn_w));
	m_maincpu->esc_data_handler().set("coproc", FUNC(i8087_device::addr_w));

	i8087_device &i8087(I8087(config, "coproc", 14.318181_MHz_XTAL / 4));
	i8087.set_addrmap(AS_PROGRAM, &mbc55x_state::mbc55x_mem);
	i8087.set_data_width(8);
	i8087.irq().set(m_pic, FUNC(pic8259_device::ir6_w));
	i8087.busy().set_inputline("maincpu", INPUT_LINE_TEST);

	ADDRESS_MAP_BANK(config, m_iodecode);
	m_iodecode->endianness(ENDIANNESS_LITTLE);
	m_iodecode->data_width(8);
	m_iodecode->addr_width(5);
	m_iodecode->set_addrmap(0, &mbc55x_state::mbc55x_iodecode);

	I8049(config, "kbdc", 6_MHz_XTAL).set_disable();

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_raw(14.318181_MHz_XTAL, 896, 0, 640, 262, 0, 200);
	screen.set_screen_update(VID_MC6845_NAME, FUNC(mc6845_device::screen_update));

	MCFG_PALETTE_ADD("palette", SCREEN_NO_COLOURS * 3)
	MCFG_PALETTE_INIT_OWNER(mbc55x_state, mbc55x)

	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("128K,192K,256K,320K,384K,448K,512K,576K,640K");

	/* sound hardware */
	SPEAKER(config, MONO_TAG).front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, MONO_TAG, 0.75);

	/* Devices */
	I8251(config, m_kb_uart, 14.318181_MHz_XTAL / 8);
	m_kb_uart->txd_handler().set("speaker", FUNC(speaker_sound_device::level_w)).invert();
	m_kb_uart->rts_handler().set(m_printer, FUNC(centronics_device::write_init)).invert();
	m_kb_uart->rxrdy_handler().set(m_pic, FUNC(pic8259_device::ir3_w));

	PIT8253(config, m_pit, 0);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_pit->out_handler<0>().append(m_pit, FUNC(pit8253_device::write_clk1));
	m_pit->out_handler<1>().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_pit->set_clk<2>(14.318181_MHz_XTAL / 8);
	m_pit->out_handler<2>().set("sio", FUNC(i8251_device::write_txc));
	m_pit->out_handler<2>().append("sio", FUNC(i8251_device::write_rxc));
	m_pit->out_handler<2>().append("line", FUNC(rs232_port_device::write_etc));

	clock_device &clk_78_6khz(CLOCK(config, "clk_78.6khz", 14.318181_MHz_XTAL / 14 / 13));
	clk_78_6khz.signal_handler().set(m_pit, FUNC(pit8253_device::write_clk0));
	clk_78_6khz.signal_handler().append(m_kb_uart, FUNC(i8251_device::write_txc));
	clk_78_6khz.signal_handler().append(m_kb_uart, FUNC(i8251_device::write_rxc));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(mbc55x_state::game_io_r));
	m_ppi->out_pa_callback().set(FUNC(mbc55x_state::game_io_w));
	m_ppi->out_pb_callback().set(FUNC(mbc55x_state::printer_data_w));
	m_ppi->in_pc_callback().set(FUNC(mbc55x_state::printer_status_r));
	m_ppi->out_pc_callback().set(FUNC(mbc55x_state::disk_select_w));

	HD6845(config, m_crtc, 14.318181_MHz_XTAL / 8); // HD46505SP-1
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(mbc55x_state::crtc_update_row), this);
	m_crtc->out_vsync_callback().set(FUNC(mbc55x_state::vid_vsync_changed));
	m_crtc->out_hsync_callback().set(FUNC(mbc55x_state::vid_hsync_changed));

	/* Backing storage */
	FD1793(config, m_fdc, 14.318181_MHz_XTAL / 14); // M5W1793-02P (clock is nominally 1 MHz)
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir5_w));

	MCFG_FLOPPY_DRIVE_ADD(m_floppy[0], mbc55x_floppies, "qd", mbc55x_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(m_floppy[1], mbc55x_floppies, "qd", mbc55x_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(m_floppy[2], mbc55x_floppies, "", mbc55x_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(m_floppy[3], mbc55x_floppies, "", mbc55x_state::floppy_formats)

	/* Software list */
	MCFG_SOFTWARE_LIST_ADD("disk_list","mbc55x")

	isa8_device &isa(ISA8(config, "isa", 14.318181_MHz_XTAL / 4));
	isa.set_cputag(m_maincpu);
	isa.irq7_callback().set(m_pic, FUNC(pic8259_device::ir7_w)); // all other IRQ and DRQ lines are NC
	//isa.iochck_callback().set_inputline(m_maincpu, INPUT_LINE_NMI));

	ISA8_SLOT(config, "external", 0, "isa", pc_isa8_cards, nullptr, false);

	i8251_device &sio(I8251(config, "sio", 14.318181_MHz_XTAL / 8)); // on separate board, through 20-pin header
	sio.dtr_handler().set("line", FUNC(rs232_port_device::write_dtr));
	sio.txd_handler().set("line", FUNC(rs232_port_device::write_txd));
	sio.rts_handler().set("line", FUNC(rs232_port_device::write_rts));
	sio.rxrdy_handler().set("sioint", FUNC(input_merger_device::in_w<0>));
	sio.txrdy_handler().set("sioint", FUNC(input_merger_device::in_w<1>));

	rs232_port_device &serial(RS232_PORT(config, "line", default_rs232_devices, nullptr));
	serial.rxd_handler().set("sio", FUNC(i8251_device::write_rxd));
	serial.dsr_handler().set("sio", FUNC(i8251_device::write_dsr));
	serial.cts_handler().set("sio", FUNC(i8251_device::write_cts));

	INPUT_MERGER_ANY_HIGH(config, "sioint").output_handler().set(m_pic, FUNC(pic8259_device::ir2_w));

	CENTRONICS(config, m_printer, centronics_devices, nullptr);
	m_printer->busy_handler().set(FUNC(mbc55x_state::printer_busy_w)).invert(); // LS14 Schmitt trigger
	m_printer->busy_handler().append(m_pic, FUNC(pic8259_device::ir4_w)).invert();
	m_printer->perror_handler().set(FUNC(mbc55x_state::printer_paper_end_w));
	m_printer->select_handler().set(FUNC(mbc55x_state::printer_select_w));
MACHINE_CONFIG_END


ROM_START( mbc55x )
	ROM_REGION(0x4000, MAINCPU_TAG, 0)

	ROM_SYSTEM_BIOS(0, "v120", "mbc55x BIOS v1.20 (1983)")
	ROMX_LOAD("mbc55x-v120.rom", 0x0000, 0x2000, CRC(b439b4b8) SHA1(6e8df0f3868e3fd0229a5c2720d6c01e46815cab), ROM_BIOS(0))

	ROM_REGION(0x0800, "kbdc", 0)
	ROM_LOAD("d8049hc.m1", 0x0000, 0x0800, NO_DUMP)
ROM_END


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME   FLAGS
COMP( 1983, mbc55x, 0,      0,      mbc55x,  mbc55x, mbc55x_state, empty_init, "Sanyo", "MBC-55x", 0 )
