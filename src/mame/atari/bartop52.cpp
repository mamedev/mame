// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*
    Atari Bartop 5200 prototype system

    Driver by Mariusz Wojcieszek
    Based on Atari 400/800 MAME Driver by Juergen Buchmueller

    Hardware was based on Atari 5200 game console with additional coin and timer hardware.
    System was intented to run in barrooms and taverns. Software was based on Atari 5200
    Realsports cartridges.

    Barroom Baseball is modified version of Realsports Baseball.

    Coins and timer were handled by Bartop system bios. Since bios dump is not available,
    regular Atari 5200 bios is used and game is marked as not working. Game cart is also
    marked as bad dump as it is done in "console style", i.e. one file for all game data.
*/

#include "emu.h"
#include "atari400.h"
#include "gtia.h"

#include "cpu/m6502/m6502.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "sound/pokey.h"

#include "screen.h"
#include "speaker.h"


namespace {

class bartop52_state : public atari_common_state
{
public:
	bartop52_state(const machine_config &mconfig, device_type type, const char *tag)
		: atari_common_state(mconfig, type, tag)
	{ }

	void a5200(machine_config &config);

protected:
	TIMER_DEVICE_CALLBACK_MEMBER( bartop_interrupt );

	virtual void machine_reset() override ATTR_COLD;
	void a5200_mem(address_map &map) ATTR_COLD;
};


void bartop52_state::a5200_mem(address_map &map)
{
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0xbfff).rom();
	map(0xc000, 0xc0ff).rw(m_gtia, FUNC(gtia_device::read), FUNC(gtia_device::write));
	map(0xd400, 0xd5ff).rw(m_antic, FUNC(antic_device::read), FUNC(antic_device::write));
	map(0xe800, 0xe8ff).rw(m_pokey, FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0xf800, 0xffff).rom();
}

#define JOYSTICK_DELTA          10
#define JOYSTICK_SENSITIVITY    200

static INPUT_PORTS_START(bartop52)
	PORT_START("djoy_b")
	PORT_BIT(0x01, 0x01, IPT_BUTTON1) PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_BUTTON1) PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x04, 0x04, IPT_UNUSED)
	PORT_BIT(0x08, 0x08, IPT_UNUSED)
	PORT_BIT(0x10, 0x10, IPT_BUTTON2) PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x20, 0x20, IPT_BUTTON2) PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_UNUSED)
	PORT_BIT(0x80, 0x80, IPT_UNUSED)

	PORT_START("keypad.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("(Break)") PORT_CODE(KEYCODE_PAUSE)    // is this correct?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Wind-Up") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[0]") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("No Wind-Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keypad.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Fast Ball Low") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Change-Up Low") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Curve Low") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keypad.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Fast Ball Med.") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Change-Up Med.") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Curve Med") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keypad.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Fast Ball High") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Change-Up High") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Curve High") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("analog_0")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(1)

	PORT_START("analog_1")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(1)

	PORT_START("analog_2")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(2)

	PORT_START("analog_3")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(2)

INPUT_PORTS_END


void bartop52_state::machine_reset()
{
	atari_common_state::machine_reset();

	m_pokey->write(15, 0);
}

TIMER_DEVICE_CALLBACK_MEMBER( bartop52_state::bartop_interrupt )
{
	m_antic->generic_interrupt(4);
}

void bartop52_state::a5200(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, pokey_device::FREQ_17_EXACT);
	m_maincpu->set_addrmap(AS_PROGRAM, &bartop52_state::a5200_mem);
	TIMER(config, "scantimer").configure_scanline(FUNC(bartop52_state::bartop_interrupt), "screen", 0, 1);

	ATARI_GTIA(config, m_gtia, 0);
	m_gtia->set_region(GTIA_NTSC);
	m_gtia->trigger_callback().set_ioport("djoy_b");

	ATARI_ANTIC(config, m_antic, 0);
	m_antic->set_gtia_tag(m_gtia);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	config_ntsc_screen(config);

	m_screen->set_screen_update("antic", FUNC(antic_device::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(bartop52_state::atari_palette), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	POKEY(config, m_pokey, pokey_device::FREQ_17_EXACT);
	m_pokey->pot_r<0>().set_ioport("analog_0");
	m_pokey->pot_r<1>().set_ioport("analog_1");
	m_pokey->pot_r<2>().set_ioport("analog_2");
	m_pokey->pot_r<3>().set_ioport("analog_3");
	m_pokey->set_keyboard_callback(FUNC(bartop52_state::a5200_keypads));
	m_pokey->irq_w().set_inputline(m_maincpu, m6502_device::IRQ_LINE);
	m_pokey->add_route(ALL_OUTPUTS, "mono", 1.00);
}

ROM_START(barbball)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "barbball.bin", 0x4000, 0x8000, BAD_DUMP CRC(21d19c8f) SHA1(510ccb20df2ecdbe7f8373de6a9fc11493e8c3f2) )
	ROM_LOAD( "5200.rom",     0xf800, 0x0800, BAD_DUMP CRC(4248d3e3) SHA1(6ad7a1e8c9fad486fbec9498cb48bf5bc3adc530) )
ROM_END

} // anonymous namespace

GAME( 1983, barbball, 0, a5200, bartop52, bartop52_state, empty_init, ROT0, "Atari", "Barroom Baseball (prototype)", MACHINE_NOT_WORKING )
