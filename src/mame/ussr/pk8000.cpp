// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

PK-8000

2009-05-12 Skeleton driver.
2009-07-18 Mostly working driver

****************************************************************************/

#include "emu.h"

#include "pk8000_v.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"
#include "machine/ram.h"

#include "screen.h"
#include "speaker.h"

#include "formats/fmsx_cas.h"


namespace {

class pk8000_state : public pk8000_base_state
{
public:
	pk8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: pk8000_base_state(mconfig, type, tag)
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_speaker(*this, "speaker")
		, m_region_maincpu(*this, "maincpu")
		, m_banks(*this, "bank%u", 0U)
		, m_io_joy1(*this, "JOY1")
		, m_io_joy2(*this, "JOY2")
		, m_keyboard(*this, "LINE%u", 0U)
	{ }

	void pk8000(machine_config &config);

private:
	u8 m_keyboard_line = 0U;

	u8 joy_1_r();
	u8 joy_2_r();
	void _80_porta_w(u8 data);
	u8 _80_portb_r();
	void _80_portc_w(u8 data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);
	IRQ_CALLBACK_MEMBER(irq_callback);

	void pk8000_io(address_map &map) ATTR_COLD;
	void pk8000_mem(address_map &map) ATTR_COLD;

	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_region_maincpu;
	required_memory_bank_array<8> m_banks;
	required_ioport m_io_joy1;
	required_ioport m_io_joy2;
	required_ioport_array<10> m_keyboard;

	void set_bank(u8 data);
};



void pk8000_state::set_bank(u8 data)
{
	u8 *rom = m_region_maincpu->base();
	u8 *ram = m_ram->pointer();
	u8 block1 = data & 3;
	u8 block2 = (data >> 2) & 3;
	u8 block3 = (data >> 4) & 3;
	u8 block4 = (data >> 6) & 3;

	switch(block1) {
		case 0:
				m_banks[0]->set_base(rom);
				m_banks[4]->set_base(ram);
				break;
		case 1: break;
		case 2: break;
		case 3:
				m_banks[0]->set_base(ram);
				m_banks[4]->set_base(ram);
				break;
	}

	switch(block2) {
		case 0:
				m_banks[1]->set_base(rom + 0x4000);
				m_banks[5]->set_base(ram + 0x4000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				m_banks[1]->set_base(ram + 0x4000);
				m_banks[5]->set_base(ram + 0x4000);
				break;
	}
	switch(block3) {
		case 0:
				m_banks[2]->set_base(rom + 0x8000);
				m_banks[6]->set_base(ram + 0x8000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				m_banks[2]->set_base(ram + 0x8000);
				m_banks[6]->set_base(ram + 0x8000);
				break;
	}
	switch(block4) {
		case 0:
				m_banks[3]->set_base(rom + 0xc000);
				m_banks[7]->set_base(ram + 0xc000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				m_banks[3]->set_base(ram + 0xc000);
				m_banks[7]->set_base(ram + 0xc000);
				break;
	}
}
void pk8000_state::_80_porta_w(u8 data)
{
	set_bank(data);
}

u8 pk8000_state::_80_portb_r()
{
	if(m_keyboard_line>9) {
		return 0xff;
	}
	return m_keyboard[m_keyboard_line]->read();
}

void pk8000_state::_80_portc_w(u8 data)
{
	m_keyboard_line = data & 0x0f;

	m_speaker->level_w(BIT(data, 7));

	m_cassette->change_state((BIT(data, 4)) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	m_cassette->output((BIT(data, 6)) ? +1.0 : 0.0);
}

u8 pk8000_state::joy_1_r()
{
	u8 retVal = (m_cassette->input() > 0.0038 ? 0x80 : 0);
	retVal |= m_io_joy1->read() & 0x7f;
	return retVal;
}
u8 pk8000_state::joy_2_r()
{
	u8 retVal = (m_cassette->input() > 0.0038 ? 0x80 : 0);
	retVal |= m_io_joy2->read() & 0x7f;
	return retVal;
}

void pk8000_state::pk8000_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).bankr("bank0").bankw("bank4");
	map(0x4000, 0x7fff).bankr("bank1").bankw("bank5");
	map(0x8000, 0xbfff).bankr("bank2").bankw("bank6");
	map(0xc000, 0xffff).bankr("bank3").bankw("bank7");
}

void pk8000_state::pk8000_io(address_map &map)
{
	map.unmap_value_high();
	map(0x80, 0x83).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x84, 0x87).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x88, 0x88).rw(FUNC(pk8000_state::video_color_r), FUNC(pk8000_state::video_color_w));
	map(0x8c, 0x8c).r(FUNC(pk8000_state::joy_1_r));
	map(0x8d, 0x8d).r(FUNC(pk8000_state::joy_2_r));
	map(0x90, 0x90).rw(FUNC(pk8000_state::text_start_r), FUNC(pk8000_state::text_start_w));
	map(0x91, 0x91).rw(FUNC(pk8000_state::chargen_start_r), FUNC(pk8000_state::chargen_start_w));
	map(0x92, 0x92).rw(FUNC(pk8000_state::video_start_r), FUNC(pk8000_state::video_start_w));
	map(0x93, 0x93).rw(FUNC(pk8000_state::color_start_r), FUNC(pk8000_state::color_start_w));
	map(0xa0, 0xbf).rw(FUNC(pk8000_state::color_r), FUNC(pk8000_state::color_w));
	// extras for pk8002
	//map(0x8c, 0x8f).   // cassette in   2x KR1533KP11
	//map(0x94, 0x97).   // video colour
	//map(0x98, 0x9b).w  // audio  KR1533TM9, KR572PA1A, 2x KR555IR8
	//map(0x9c, 0x9f).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));  // part of the audio
}

/*   Input ports */
static INPUT_PORTS_START( pk8000 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_SLASH)
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("|") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_MINUS_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rg") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Upr") PORT_CODE(KEYCODE_LCONTROL)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graf") PORT_CODE(KEYCODE_F10)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alf") PORT_CODE(KEYCODE_F9)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("???") PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_F12)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("VSh") PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sel") PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_START("LINE8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Probel") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cls") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_START("LINE9")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up-Left") PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down-Right") PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Menu") PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Begin") PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End") PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End page") PORT_CODE(KEYCODE_PGDN)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Begin page") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("JOY1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("JOY2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(pk8000_state::interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

IRQ_CALLBACK_MEMBER(pk8000_state::irq_callback)
{
	return 0xff;
}


void pk8000_state::machine_start()
{
	save_item(NAME(m_keyboard_line));
	save_item(NAME(m_text_start));
	save_item(NAME(m_chargen_start));
	save_item(NAME(m_video_start));
	save_item(NAME(m_color_start));
	save_item(NAME(m_video_mode));
	save_item(NAME(m_video_color));
	save_item(NAME(m_color));
	save_item(NAME(m_video_enable));
}

void pk8000_state::machine_reset()
{
	set_bank(0);
}

u32 pk8000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return video_update(screen, bitmap, cliprect, m_ram->pointer());
}

void pk8000_state::pk8000(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 1780000); // pk8002 = 10'220'000 / 8
	m_maincpu->set_addrmap(AS_PROGRAM, &pk8000_state::pk8000_mem);
	m_maincpu->set_addrmap(AS_IO, &pk8000_state::pk8000_io);
	m_maincpu->set_vblank_int("screen", FUNC(pk8000_state::interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(pk8000_state::irq_callback));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(256+32, 192+32);
	screen.set_visarea(0, 256+32-1, 0, 192+32-1);
	screen.set_screen_update(FUNC(pk8000_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(pk8000_state::pk8000_palette), 16);

	i8255_device &ppi1(I8255(config, "ppi8255_1"));
	ppi1.out_pa_callback().set(FUNC(pk8000_state::_80_porta_w));
	ppi1.in_pb_callback().set(FUNC(pk8000_state::_80_portb_r));
	ppi1.out_pc_callback().set(FUNC(pk8000_state::_80_portc_w));

	i8255_device &ppi2(I8255(config, "ppi8255_2"));
	ppi2.in_pa_callback().set(FUNC(pk8000_state::_84_porta_r));
	ppi2.out_pa_callback().set(FUNC(pk8000_state::_84_porta_w));
	ppi2.out_pc_callback().set(FUNC(pk8000_state::_84_portc_w));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(fmsx_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}

/* ROM definition */
ROM_START( vesta )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vesta.rom", 0x0000, 0x4000, CRC(fbf7e2cc) SHA1(4bc5873066124bd926c3c6aa2fd1a062c87af339))
ROM_END

ROM_START( hobby )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "hobby.rom", 0x0000, 0x4000, CRC(a25b4b2c) SHA1(0d86e6e4be8d1aa389bfa9dd79e3604a356729f7))
ROM_END

ROM_START( pk8002 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// there is actually 1 rom 0000-07ff (U3) mirrored to 0800,1000,1800, then another rom 2000-27ff (U4) mirrored to 2800,3000,3800
	ROM_LOAD( "pk8002.rom", 0x0000, 0x4000, CRC(07b9ae71) SHA1(2137a41cc095c7aba58b7b109fce63f30a4568b2))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY      FULLNAME             FLAGS */
COMP( 1987, vesta,  0,      0,      pk8000,  pk8000, pk8000_state, empty_init, "BP EVM",    "PK8000 Vesta",      MACHINE_SUPPORTS_SAVE )
COMP( 1987, hobby,  vesta,  0,      pk8000,  pk8000, pk8000_state, empty_init, "BP EVM",    "PK8000 Sura/Hobby", MACHINE_SUPPORTS_SAVE )
COMP( 1987, pk8002, vesta,  0,      pk8000,  pk8000, pk8000_state, empty_init, "<unknown>", "PK8002 Elf",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
