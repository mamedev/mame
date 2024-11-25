// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Dolphin / Dauphin

2010-04-08 Skeleton driver.
2012-05-20 Fixed keyboard, added notes & speaker [Robbbert]
2013-11-03 Added cassette [Robbbert]

Minimal Setup:
    0000-00FF ROM "MO" (74S471)
    0100-01FF ROM "MONI" (74S471)
    0200-02FF RAM (2x 2112)
    18 pushbuttons for programming (0-F, ADR, NXT).
    4-digit LED display.

Other options:
    0400-07FF Expansion RAM (8x 2112)
    0800-08FF Pulse for operation of an optional EPROM programmer
    0C00-0FFF ROM "MONA" (2708)
    LEDs connected to all Address and Data Lines
    LEDs connected to WAIT and FLAG lines.
    Speaker with a LED wired across it.
    PAUSE switch.
    RUN/STOP switch.
    STEP switch.
    CLOCK switch.

Cassette player connected to SENSE and FLAG lines.

Keyboard encoder: AY-5-2376 (57 keys)

CRT interface: (512 characters on a separate bus)
    2114 video ram (one half holds the lower 4 data bits, other half the upper bits)
    74LS175 holds the upper bits for the 74LS472
    74LS472 Character Generator

NOTE: The cassette rom is missing, when the ADR button (- key) is pressed,
      it causes a freeze in nodebug mode, and a crash in debug mode.
      To see it, start in debug mode. g 6c. In the emulation, press the
      minus key. The debugger will stop and you can see an instruction
      referencing location 0100, which is in the missing rom.
      The machine is marked MNW because it is not possible to save/load programs.

Keys:
    0-9,A-F hexadecimal numbers
    UP - (NXT) to enter data and advance to the next address
    MINUS - (ADR) to change the address to what is shown in the data side
    Special keys:
        Hold UP, hold 0, release UP, release 0 - execute program at the current address (i.e. 2xx)
        Hold UP, hold 1, release UP, release 1 - execute program at address 0C00 (rom MONA)
        Hold UP, hold 2, release UP, release 2 - play a tune with the keys
        Hold UP, hold 3, release UP, release 3 - decrement the address by 2
        Hold MINUS, hold any hex key, release MINUS, release other key - execute program
          at the current address-0x100 (i.e. 1xx).

If you want to scan through other areas of memory (e.g. the roms), alter the
data at address 2F9 (high byte) and 2FA (low byte).

How to Use:
    The red digits are the address, and the orange digits are the data.
    The address range is 200-2FF (the 2 isn't displayed). To select an address,
    either press the UP key until you get there, or type the address and press
    minus. The orange digits show the current data at that address. To alter
    data, just type it in and press UP.

To play the reflexes game, hold UP, press 1, release UP, release 1.
    The display will show A--0 (or some random number in the last position).
    Press any odd-numbered key (B is convenient), and read off the reaction time.
    After a short delay it will show '--' again, this is the signal to react.
    It doesn't seem to reset the counters each time around.

Test Paste:
    Paste this: 11^22^33^44^55^66^77^88^99^00-
    Now press up-arrow to review the data that was entered.

TODO:
    - Find missing roms. Once the cassette rom is found, need to work out
      how to use it, and find out if the cassette interface works.
    - Add optional hardware listed above

Thanks to Amigan site for various documents.


****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/cassette.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "speaker.h"
#include "video/pwm.h"

#include "dolphunk.lh"


namespace {

class dauphin_state : public driver_device
{
public:
	dauphin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_cass(*this, "cassette")
		, m_display(*this, "display")
	{ }

	void dauphin(machine_config &config);

private:
	int cass_r();
	u8 port07_r();
	void port00_w(offs_t offset, u8 data);
	void port06_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void machine_start() override ATTR_COLD;

	u8 m_cass_data = 0U;
	u8 m_last_key = 0U;
	bool m_cassbit = 0;
	bool m_cassold = 0;
	bool m_speaker_state = 0;
	required_device<s2650_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cass;
	required_device<pwm_display_device> m_display;
};

int dauphin_state::cass_r()
{
	return (m_cass->input() > 0.03) ? 1 : 0;
}

void dauphin_state::port00_w(offs_t offset, u8 data)
{
	m_display->matrix(1<<offset, data);
}

void dauphin_state::port06_w(u8 data)
{
	m_speaker_state ^=1;
	m_speaker->level_w(m_speaker_state);
}

u8 dauphin_state::port07_r()
{
	u8 keyin, i, data = 0x40;

	keyin = ioport("X0")->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if (BIT(~keyin, i))
				data = i | 0xc0;

	keyin = ioport("X1")->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if (BIT(~keyin, i))
				data = i | 0xc8;

	if (data == m_last_key)
		data &= 0x7f;
	else
		m_last_key = data;

	data |= ioport("X2")->read();

	return data;
}

TIMER_DEVICE_CALLBACK_MEMBER(dauphin_state::kansas_w)
{
	m_cass_data++;

	if (m_cassbit != m_cassold)
	{
		m_cass_data = 0;
		m_cassold = m_cassbit;
	}

	if (m_cassbit)
		m_cass->output(BIT(m_cass_data, 1) ? -1.0 : +1.0); // 1000Hz
	else
		m_cass->output(BIT(m_cass_data, 0) ? -1.0 : +1.0); // 2000Hz
}

void dauphin_state::machine_start()
{
	save_item(NAME(m_cass_data));
	save_item(NAME(m_last_key));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_speaker_state));
}

void dauphin_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x01ff).rom();
	map(0x0200, 0x02ff).ram();
	map(0x0c00, 0x0fff).rom();
}

void dauphin_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x03).w(FUNC(dauphin_state::port00_w)); // 4-led display
	map(0x06, 0x06).w(FUNC(dauphin_state::port06_w));  // speaker (NOT a keyclick)
	map(0x07, 0x07).r(FUNC(dauphin_state::port07_r)); // pushbuttons
}

/* Input ports */
static INPUT_PORTS_START( dauphin )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NXT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ADR") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
INPUT_PORTS_END


void dauphin_state::dauphin(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, XTAL(1'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &dauphin_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &dauphin_state::io_map);
	m_maincpu->sense_handler().set(FUNC(dauphin_state::cass_r));
	m_maincpu->flag_handler().set([this] (bool state) { m_cassbit = state; });

	/* video hardware */
	config.set_default_layout(layout_dolphunk);
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0x0f, 0xff);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_w").configure_periodic(FUNC(dauphin_state::kansas_w), attotime::from_hz(4000));
}

/* ROM definition */
ROM_START( dauphin )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dolphin_mo.rom", 0x0000, 0x0100, CRC(a8811f48) SHA1(233c629dc20fac286c8c1559e461bb0b742a675e) )
	// This one is used in winarcadia but it is a bad dump, we use the corrected one above
	//ROM_LOAD( "dolphin_mo.rom", 0x0000, 0x0100, BAD_DUMP CRC(1ac4ac18) SHA1(62a63de6fcd6cd5fcee930d31c73fe603647f06c) )

	// Cassette rom
	ROM_LOAD( "dolphin_moni.rom", 0x0100, 0x0100, NO_DUMP )

	//ROM_LOAD_OPTIONAL( "dolphin_mona.rom", 0x0c00, 0x0400, NO_DUMP )
	// This rom is a bugfixed and relocated version of the game found on the Amigan site
	ROM_LOAD_OPTIONAL( "reflexes.bin", 0x0c00, 0x0400, CRC(14a1557d) SHA1(789d10551f1bb3472057901fa3cee0c6bfe220ac) )
	// This the original
	//ROM_LOAD( "reflexes.bin", 0x0c00, 0x0072, CRC(c4bed94b) SHA1(cf525755a1dbce6a4681353be692ddf0346f323b) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY              FULLNAME   FLAGS
COMP( 1979, dauphin, 0,      0,      dauphin, dauphin, dauphin_state, empty_init, "LCD EPFL Stoppani", "Dauphin", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
