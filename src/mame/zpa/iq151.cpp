// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        IQ-151

        12/05/2009 Skeleton driver.

        07/June/2011 Added screen & keyboard (by looking at the Z80 code)


This computer depends on RAM just happening to be certain values at powerup.
If the conditions are not met, it may crash.

Monitor Commands:
C Call (address)
D Dump memory, any key to dump more, Return to finish
F Fill memory (start, end, withwhat)
G Goto (address)
L Cassette load
M Move (source start, source end, destination)
R Run
S Edit memory
W Cassette save (start, end, goto (0 for null))
X Display/Edit registers


ToDo:
- Add whatever devices may exist.

- Line 32 does not scroll, should it show?
  (could be reserved for a status line in a terminal mode)

- Note that the system checks for 3E at C000, if exist, jump to C000;
  otherwise then checks for non-FF at C800, if so, jumps to C800. Could be
  extra roms or some sort of boot device.

- Key beep sounds better if clock speed changed to 1MHz, but it is still
  highly annoying. Press Ctrl-G to hear the 2-tone bell.

- Cassette support is tested only in the emulator (monitor, BASIC and AMOS),
  needs to be tested with a real cassette dump.

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"

// cartridge slot
#include "bus/iq151/iq151.h"
#include "bus/iq151/rom.h"
#include "bus/iq151/disc2.h"
#include "bus/iq151/minigraf.h"
#include "bus/iq151/ms151a.h"
#include "bus/iq151/staper.h"
#include "bus/iq151/grafik.h"
#include "bus/iq151/video32.h"
#include "bus/iq151/video64.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class iq151_state : public driver_device
{
public:
	iq151_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic8259")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_carts(*this, "slot%u", 1U)
		, m_boot_view(*this, "boot_view")
		, m_keyboard(*this, "X%X", 0U)
	{ }

	void iq151(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(iq151_break);

private:
	uint8_t keyboard_row_r();
	uint8_t keyboard_column_r();
	uint8_t ppi_portc_r();
	void ppi_portc_w(uint8_t data);
	void boot_bank_w(uint8_t data);
	uint8_t cartslot_r(offs_t offset);
	void cartslot_w(offs_t offset, uint8_t data);
	uint8_t cartslot_io_r(offs_t offset);
	void cartslot_io_w(offs_t offset, uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(iq151_vblank_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(cassette_timer);
	void iq151_io(address_map &map) ATTR_COLD;
	void iq151_mem(address_map &map) ATTR_COLD;

	required_device<i8080_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device_array<iq151cart_slot_device, 5> m_carts;
	memory_view m_boot_view;
	required_ioport_array<9> m_keyboard;

	uint8_t m_vblank_irq_state;
	uint8_t m_cassette_clk;
	uint8_t m_cassette_data;
};

uint8_t iq151_state::keyboard_row_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 8; i++)
	{
		data &= m_keyboard[i]->read();
	}

	return data;
}

uint8_t iq151_state::keyboard_column_r()
{
	uint8_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		if (m_keyboard[i]->read() == 0xff)
			data |= (1 << i);
	}

	return data;
}

uint8_t iq151_state::ppi_portc_r()
{
	uint8_t data = 0x00;

	if (m_cassette_data & 0x06)
	{
		// cassette read
		data |= ((m_cassette_clk & 1) << 5);
		data |= (m_cassette->input() > 0.00 ? 0x80 : 0x00);
	}
	else
	{
		// kb read
		data = m_keyboard[8]->read();
	}

	return (data & 0xf0) | (m_cassette_data & 0x0f);
}


void iq151_state::ppi_portc_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 3));
	m_cassette_data = data;
}

void iq151_state::boot_bank_w(uint8_t data)
{
	if (BIT(data, 0))
		m_boot_view.disable();
	else
		m_boot_view.select(0);
}


//**************************************************************************
//  Cartridge slot emulation
//**************************************************************************

uint8_t iq151_state::cartslot_r(offs_t offset)
{
	uint8_t data = 0xff;

	for (auto & elem : m_carts)
		elem->read(offset, data);

	return data;
}

void iq151_state::cartslot_w(offs_t offset, uint8_t data)
{
	for (auto & elem : m_carts)
		elem->write(offset, data);
}

uint8_t iq151_state::cartslot_io_r(offs_t offset)
{
	uint8_t data = 0xff;

	for (auto & elem : m_carts)
		elem->io_read(offset, data);

	return data;
}

void iq151_state::cartslot_io_w(offs_t offset, uint8_t data)
{
	for (auto & elem : m_carts)
		elem->io_write(offset, data);
}

void iq151_state::iq151_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(iq151_state::cartslot_r), FUNC(iq151_state::cartslot_w));

	map(0x0000, 0x7fff).ram();
	map(0x0000, 0x07ff).view(m_boot_view);
	m_boot_view[0](0x0000, 0x07ff).rom().region("maincpu", 0x0800);
	//m_boot_view[0](0x0000, 0x07ff).nopw(); // TODO: write ignored when boot ROM is selected?
	map(0xf000, 0xffff).rom().region("maincpu", 0x0000);
}

void iq151_state::iq151_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(iq151_state::cartslot_io_r), FUNC(iq151_state::cartslot_io_w));

	map(0x80, 0x80).w(FUNC(iq151_state::boot_bank_w));
	map(0x84, 0x87).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x88, 0x89).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
}


INPUT_CHANGED_MEMBER(iq151_state::iq151_break)
{
	m_pic->ir5_w(newval & 1);
}

/* Input ports */
static INPUT_PORTS_START( iq151 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)       PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2)      PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)       PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4)       PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)       PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6)       PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7)       PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8)       PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)     PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)     PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)     PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)     PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)     PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)     PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)     PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)     PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)     PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)     PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)     PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)     PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)     PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)     PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)     PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)     PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)     PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)     PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)     PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)     PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)     PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)     PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)     PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9)       PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)     PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)     PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)     PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)     PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)     PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DL") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("IL") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_END) // its actually some sort of graphic character
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("X8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FA") PORT_CODE(KEYCODE_RSHIFT)       // Function A
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FB") PORT_CODE(KEYCODE_RCONTROL)     // Function B

	PORT_START("BREAK")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC)   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(iq151_state::iq151_break), 0)  PORT_CHAR(UCHAR_MAMEKEY(ESC))
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(iq151_state::iq151_vblank_interrupt)
{
	m_pic->ir6_w(m_vblank_irq_state & 1);
	m_vblank_irq_state ^= 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(iq151_state::cassette_timer)
{
	m_cassette_clk ^= 1;

	m_cassette->output((m_cassette_data & 1) ^ (m_cassette_clk & 1) ? +1 : -1);
}

void iq151_state::machine_start()
{
}

void iq151_state::machine_reset()
{
	m_boot_view.select(0);

	m_vblank_irq_state = 0;
}

// this machine don't have a built-in video controller, but uses external cartridge
uint32_t iq151_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for (auto & elem : m_carts)
		elem->video_update(bitmap, cliprect);

	return 0;
}

static void iq151_cart(device_slot_interface &device)
{
	device.option_add("video32",  IQ151_VIDEO32);       // video32
	device.option_add("video64",  IQ151_VIDEO64);       // video64
	device.option_add("grafik",   IQ151_GRAFIK);        // Grafik
	device.option_add("disc2",    IQ151_DISC2);         // Disc 2
	device.option_add("minigraf", IQ151_MINIGRAF);      // Aritma Minigraf 0507
	device.option_add("ms151a",   IQ151_MS151A);        // MS151A XY Plotter
	device.option_add("staper",   IQ151_STAPER);        // STAPER
	device.option_add("basic6",   IQ151_BASIC6);        // BASIC6
	device.option_add("basicg",   IQ151_BASICG);        // BASICG
	device.option_add("amos1",    IQ151_AMOS1);         // AMOS cart 1
	device.option_add("amos2",    IQ151_AMOS2);         // AMOS cart 2
	device.option_add("amos3",    IQ151_AMOS3);         // AMOS cart 3
}

void iq151_state::iq151(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(2'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &iq151_state::iq151_mem);
	m_maincpu->set_addrmap(AS_IO, &iq151_state::iq151_io);
	m_maincpu->set_vblank_int("screen", FUNC(iq151_state::iq151_vblank_interrupt));
	m_maincpu->in_inta_func().set("pic8259", FUNC(pic8259_device::acknowledge));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(iq151_state::screen_update));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 32*8-1, 0, 32*8-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	i8255_device &ppi(I8255(config, "ppi8255"));
	ppi.in_pa_callback().set(FUNC(iq151_state::keyboard_row_r));
	ppi.in_pb_callback().set(FUNC(iq151_state::keyboard_column_r));
	ppi.in_pc_callback().set(FUNC(iq151_state::ppi_portc_r));
	ppi.out_pc_callback().set(FUNC(iq151_state::ppi_portc_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("iq151_cass");

	TIMER(config, "cassette_timer").configure_periodic(FUNC(iq151_state::cassette_timer), attotime::from_hz(2000));

	/* cartridge */
	IQ151CART_SLOT(config, m_carts[0], iq151_cart, nullptr);
	m_carts[0]->set_screen_tag("screen");
	m_carts[0]->out_irq0_callback().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_carts[0]->out_irq1_callback().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_carts[0]->out_irq2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_carts[0]->out_irq3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_carts[0]->out_irq4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	IQ151CART_SLOT(config, m_carts[1], iq151_cart, nullptr);
	m_carts[1]->set_screen_tag("screen");
	m_carts[1]->out_irq0_callback().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_carts[1]->out_irq1_callback().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_carts[1]->out_irq2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_carts[1]->out_irq3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_carts[1]->out_irq4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	IQ151CART_SLOT(config, m_carts[2], iq151_cart, nullptr);
	m_carts[2]->set_screen_tag("screen");
	m_carts[2]->out_irq0_callback().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_carts[2]->out_irq1_callback().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_carts[2]->out_irq2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_carts[2]->out_irq3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_carts[2]->out_irq4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	IQ151CART_SLOT(config, m_carts[3], iq151_cart, nullptr);
	m_carts[3]->set_screen_tag("screen");
	m_carts[3]->out_irq0_callback().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_carts[3]->out_irq1_callback().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_carts[3]->out_irq2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_carts[3]->out_irq3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_carts[3]->out_irq4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	IQ151CART_SLOT(config, m_carts[4], iq151_cart, "video32");
	m_carts[4]->set_screen_tag("screen");
	m_carts[4]->out_irq0_callback().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_carts[4]->out_irq1_callback().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_carts[4]->out_irq2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_carts[4]->out_irq3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_carts[4]->out_irq4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("iq151_cart");
	SOFTWARE_LIST(config, "flop_list").set_original("iq151_flop");
}

/* ROM definition */
ROM_START( iq151 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE )
	/* A number of bios versions here. The load address is shown for each */
	ROM_SYSTEM_BIOS( 0, "orig", "Original" )
	ROMX_LOAD( "iq151_monitor_orig.rom", 0x0000, 0x1000, CRC(acd10268) SHA1(4d75c73f155ed4dc2ac51a9c22232f869cca95e2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "disasm", "Disassembler" )
	ROMX_LOAD( "iq151_monitor_disasm.rom", 0x0000, 0x1000, CRC(45c2174e) SHA1(703e3271a124c3ef9330ae399308afd903316ab9), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "cpm", "CPM" )
	ROMX_LOAD( "iq151_monitor_cpm.rom", 0x0000, 0x1000, CRC(26f57013) SHA1(4df396edc375dd2dd3c82c4d2affb4f5451066f1), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "cpmold", "CPM (old)" )
	ROMX_LOAD( "iq151_monitor_cpm_old.rom", 0x0000, 0x1000, CRC(6743e1b7) SHA1(ae4f3b1ba2511a1f91c4e8afdfc0e5aeb0fb3a42), ROM_BIOS(3))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  STATE        INIT        COMPANY         FULLNAME  FLAGS
COMP( 198?, iq151, 0,      0,      iq151,   iq151, iq151_state, empty_init, "ZPA Novy Bor", "IQ-151", 0 )
