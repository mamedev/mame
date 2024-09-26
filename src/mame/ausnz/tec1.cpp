// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

TEC-1 driver, written by Robbbert in April, 2009.

The TEC-1 was a single-board "computer" described in Talking Electronics
magazine, issues number 10 and 11. Talking Electronics do not have dates on
their issues, so the date is uncertain, although 1984 seems a reasonable
guess. Talking Electronics operated from Cheltenham, a suburb of Melbourne.

The hardware is quite simple, consisting of a Z80 cpu, 2x 8212 8-bit latch,
74C923 keyboard scanner, 20 push-button keys, 6-digit LED display, a speaker,
a 2k EPROM and sundry parts.

The cpu speed could be adjusted by using a potentiometer, the range being
250 kHz to 2MHz. This is a simple method of adjusting a game's difficulty.

We emulate the original version. Later enhancements included more RAM, speech
synthesis and various attachments, however I have no information on these.

2018-11-08 Obtained fresh dumps from the original designers.

Pasting:
        0-F : as is
        + (inc) : ^
        - (dec) : V
        AD : -
        GO : X

Keys:
0 to 9, A to F are on the key of the same name.
AD (input an address) is the '-' key.
+ and - (increment / decrement address) are the up and down-arrow keys.
GO (execute program at current address) is the X key.
SHIFT - later monitor versions utilised an extra shift button. Hold
        it down and press another key (use Left Shift).

Whenever a program listing mentions RESET, press Left-Alt key.

Each key causes a beep to be heard. You may need to press more than once
to get it to register.

Inbuilt games - press the following sequence of keys:
- Invaders:     RESET AD 0 0 0 8 GO GO (Paste: -0008XX)
- Nim:          RESET AD 0 0 1 0 GO GO (Paste: -0010XX)
- Lunar Lander: RESET AD 0 0 1 8 GO GO (Paste: -0018XX)
Tunes:
- Bealach An Doirin: RESET AD 0 0 2 8 GO GO (Paste: -0028XX)
- Biking up the strand: RESET AD 0 0 3 0 GO GO (Paste: -0030XX)

Thanks to Chris Schwartz who dumped his ROM for me way back in the old days.
It's only taken 25 years to get around to emulating it...


Differences between tec1 and tecjmon:

On the tec1 a keypress is indicated by an NMI from the 74C923; but on
the jmon it sets bit 6 of port 3 low instead. The NMI code is simply
a 'retn' in the jmon rom, so we can use the same code.
The jmon includes a cassette interface, a serial input connection,
and an optional LCD, but the games of the tec1 have been removed.



JMON ToDo:
- Add LCD display (2 rows by 16 characters)


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/mm74c922.h"
#include "machine/rescap.h"
#include "sound/spkrdev.h"
#include "speaker.h"
#include "video/pwm.h"

#include "tec1.lh"


namespace {

class tec1_state : public driver_device
{
public:
	tec1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_cass(*this, "cassette")
		, m_kb(*this, "keyboard")
		, m_io_shift(*this, "SHIFT")
		, m_display(*this, "display")
	{ }

	void tec1(machine_config &config);
	void tecjmon(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

private:
	virtual void machine_start() override ATTR_COLD;
	u8 kbd_r();
	u8 latch_r();
	void tec1_digit_w(u8 data);
	void tecjmon_digit_w(u8 data);
	void segment_w(u8 data);
	void da_w(int state);
	bool m_key_pressed = 0;
	u8 m_seg = 0U;
	u8 m_digit = 0U;
	void tec1_io(address_map &map) ATTR_COLD;
	void tec1_map(address_map &map) ATTR_COLD;
	void tecjmon_io(address_map &map) ATTR_COLD;
	void tecjmon_map(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	optional_device<cassette_image_device> m_cass;
	required_device<mm74c923_device> m_kb;
	required_ioport m_io_shift;
	required_device<pwm_display_device> m_display;
};




/***************************************************************************

    Display

***************************************************************************/

void tec1_state::segment_w(u8 data)
{
/*  d7 segment d
    d6 segment e
    d5 segment c
    d4 segment dot
    d3 segment b
    d2 segment g
    d1 segment f
    d0 segment a */

	m_seg = bitswap<8>(data, 4, 2, 1, 6, 7, 5, 3, 0);
	m_display->matrix(m_digit, m_seg);
}

void tec1_state::tec1_digit_w(u8 data)
{
/*  d7 speaker
    d6 not used
    d5 data digit 1
    d4 data digit 2
    d3 address digit 1
    d2 address digit 2
    d1 address digit 3
    d0 address digit 4 */

	m_speaker->level_w(BIT(data, 7));

	m_digit = data;
	m_display->matrix(m_digit, m_seg);
}

void tec1_state::tecjmon_digit_w(u8 data)
{
/*  d7 speaker & cassout
    d6 not used
    d5 data digit 1
    d4 data digit 2
    d3 address digit 1
    d2 address digit 2
    d1 address digit 3
    d0 address digit 4 */

	m_speaker->level_w(BIT(data, 7));
	m_cass->output(BIT(data, 7) ? -1.0 : +1.0);
	m_digit = data;
	m_display->matrix(m_digit, m_seg);
}


/***************************************************************************

    Keyboard

***************************************************************************/

u8 tec1_state::latch_r()
{
// bit 7 - cass in ; bit 6 low = key pressed
	u8 data = (m_key_pressed) ? 0 : 0x40;

	if (m_cass->input() > 0.03)
		data |= 0x80;

	return data;
}


u8 tec1_state::kbd_r()
{
	return m_kb->read() | m_io_shift->read();
}

void tec1_state::da_w(int state)
{
	m_key_pressed = state;
	m_maincpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}


/***************************************************************************

    Machine

***************************************************************************/

void tec1_state::machine_start()
{
	save_item(NAME(m_key_pressed));
	save_item(NAME(m_seg));
	save_item(NAME(m_digit));
}

/***************************************************************************

    Address Map

***************************************************************************/

void tec1_state::tec1_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x0fff).ram(); // on main board
	map(0x1000, 0x3fff).ram(); // expansion
}

void tec1_state::tec1_io(address_map &map)
{
	map.global_mask(0x07);
	map(0x00, 0x00).r(FUNC(tec1_state::kbd_r));
	map(0x01, 0x01).w(FUNC(tec1_state::tec1_digit_w));
	map(0x02, 0x02).w(FUNC(tec1_state::segment_w));
}


void tec1_state::tecjmon_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x37ff).ram();
	map(0x3800, 0x3fff).rom().region("maincpu", 0x0800);
}

void tec1_state::tecjmon_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(tec1_state::kbd_r));
	map(0x01, 0x01).w(FUNC(tec1_state::tecjmon_digit_w));
	map(0x02, 0x02).w(FUNC(tec1_state::segment_w));
	map(0x03, 0x03).r(FUNC(tec1_state::latch_r));
	//map(0x04, 0x04).w(FUNC(tec1_state::lcd_en_w));
	//map(0x84, 0x84).w(FUNC(tec1_state::lcd_2nd_w));
}


/**************************************************************************

    Keyboard Layout

***************************************************************************/

static INPUT_PORTS_START( tec1 )
	PORT_START("LINE0") /* KEY ROW 0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)    PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)    PORT_CHAR('4')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)    PORT_CHAR('8')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)    PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_UP)   PORT_CHAR('^')

	PORT_START("LINE1") /* KEY ROW 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)    PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)    PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)    PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)    PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')

	PORT_START("LINE2") /* KEY ROW 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)    PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)    PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)    PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)    PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO") PORT_CODE(KEYCODE_X)   PORT_CHAR('X')

	PORT_START("LINE3") /* KEY ROW 3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)    PORT_CHAR('3')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)    PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)    PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)    PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("AD") PORT_CODE(KEYCODE_MINUS)   PORT_CHAR('-')

	PORT_START("SHIFT")
	PORT_BIT(0x1f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, tec1_state, reset_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(tec1_state::reset_button)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}


/***************************************************************************

    Machine driver

***************************************************************************/

void tec1_state::tec1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 1000000);   /* speed can be varied between 250kHz and 2MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &tec1_state::tec1_map);
	m_maincpu->set_addrmap(AS_IO, &tec1_state::tec1_io);

	/* video hardware */
	config.set_default_layout(layout_tec1);
	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0x3f, 0xff);

	MM74C923(config, m_kb, 0);
	m_kb->set_cap_osc(CAP_N(100));
	m_kb->set_cap_debounce(CAP_U(1));
	m_kb->da_wr_callback().set(FUNC(tec1_state::da_w));
	m_kb->x1_rd_callback().set_ioport("LINE0");
	m_kb->x2_rd_callback().set_ioport("LINE1");
	m_kb->x3_rd_callback().set_ioport("LINE2");
	m_kb->x4_rd_callback().set_ioport("LINE3");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void tec1_state::tecjmon(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(3'579'545) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &tec1_state::tecjmon_map);
	m_maincpu->set_addrmap(AS_IO, &tec1_state::tecjmon_io);

	/* video hardware */
	config.set_default_layout(layout_tec1);
	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0x3f, 0xff);

	MM74C923(config, m_kb, 0);
	m_kb->set_cap_osc(CAP_N(100));
	m_kb->set_cap_debounce(CAP_U(1));
	m_kb->da_wr_callback().set(FUNC(tec1_state::da_w));
	m_kb->x1_rd_callback().set_ioport("LINE0");
	m_kb->x2_rd_callback().set_ioport("LINE1");
	m_kb->x3_rd_callback().set_ioport("LINE2");
	m_kb->x4_rd_callback().set_ioport("LINE3");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
}


/***************************************************************************

    Game driver

***************************************************************************/

ROM_START(tec1)
	ROM_REGION(0x0800, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "mon1", "MON1")
	ROMX_LOAD( "mon1.rom",   0x0000, 0x0800, CRC(5d379e6c) SHA1(5c810885a3f0d03c54aea74aaaa8fae8a2fd9ad4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "mon1a", "MON1A")
	ROMX_LOAD("mon1a.rom",   0x0000, 0x0800, CRC(b3390c36) SHA1(18aabc68d473206b7fc4e365c6b57a4e218482c3), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "mon1b", "MON1B")
	//ROMX_LOAD("mon1b.rom",   0x0000, 0x0800, CRC(60daea3c) SHA1(383b7e7f02e91fb18c87eb03c5949e31156771d4), ROM_BIOS(2))
	ROMX_LOAD("mon1b.rom",   0x0000, 0x0800, CRC(6088811d) SHA1(2cec14a24fae769f22f6598b5a63fc79d90db394), ROM_BIOS(2))    // redump
	ROM_SYSTEM_BIOS(3, "mon2", "MON2")
	ROMX_LOAD("mon2.rom",    0x0000, 0x0800, CRC(082fd7e7) SHA1(7659add30ca22b15a03d1cbac0892a5c25e47ecd), ROM_BIOS(3))
ROM_END

ROM_START(tecjmon)
	ROM_REGION(0x1000, "maincpu", 0 )
	ROM_LOAD("jmon.rom",    0x0000, 0x0800, CRC(202c47a2) SHA1(701588ec5640d633d90d94b2ccd6f65422e19a70) )
	ROM_LOAD("util.rom",    0x0800, 0x0800, CRC(7c19700d) SHA1(dc5b3ade66bb11c54430056966ed99cdd299d82b) )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                         FULLNAME            FLAGS
COMP( 1984, tec1,     0,      0,      tec1,    tec1,  tec1_state, empty_init, "Talking Electronics magazine", "TEC-1",            MACHINE_SUPPORTS_SAVE )
COMP( 1984, tecjmon,  tec1,   0,      tecjmon, tec1,  tec1_state, empty_init, "Talking Electronics magazine", "TEC-1A with JMON", MACHINE_SUPPORTS_SAVE )
