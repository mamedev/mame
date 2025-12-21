// license:BSD-3-Clause
// copyright-holders:R. Belmont, Sergey Svishchev
/***************************************************************************

    Nippel Super Games ][

    An arcade board designed to run modified games from Apple ][.
    Most of Apple hardware is missing, keyboard port is reused for joysticks.

    Only one game is known (a Mario Bros. hack/translation patch).

    Info: http://agatcomp.ru/Pravetz/SuperGames.shtml

    To do:
    - verify palette, pixel and cpu clocks
    - proper bootstrap (is there another ROM?)

************************************************************************/

#include "emu.h"

#include "apple2common.h"
#include "apple2video.h"

#include "cpu/m6502/m6502.h"

#include "machine/74259.h"
#include "machine/ram.h"
#include "machine/timer.h"

#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"


namespace {

#define A2_CPU_TAG "maincpu"
#define A2_VIDEO_TAG "a2video"

class superga2_state : public driver_device
{
public:
	superga2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, A2_CPU_TAG)
		, m_screen(*this, "screen")
		, m_ram(*this, RAM_TAG)
		, m_video(*this, A2_VIDEO_TAG)
		, m_a2common(*this, "a2common")
		, m_speaker(*this, "speaker")
		, m_softlatch(*this, "softlatch")
	{ }

	static constexpr feature_type imperfect_features() { return feature::PALETTE; }

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ram_device> m_ram;
	required_device<a2_video_device> m_video;
	required_device<apple2_common_device> m_a2common;
	required_device<speaker_sound_device> m_speaker;
	required_device<addressable_latch_device> m_softlatch;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	uint8_t speaker_toggle_r();
	void speaker_toggle_w(uint8_t data);
	uint8_t switches_r(offs_t offset);
	uint8_t reset_r(offs_t offset);

	void superga2(machine_config &config);
	void superga2_map(address_map &map) ATTR_COLD;

private:
	int m_speaker_state = 0;

	uint8_t *m_ram_ptr = nullptr;
	int m_ram_size = 0;

	uint8_t read_floatingbus();
};

/***************************************************************************
    START/RESET
***************************************************************************/

void superga2_state::machine_start()
{
	m_ram_ptr = m_ram->pointer();
	m_ram_size = m_ram->size();
	m_speaker_state = 0;
	m_speaker->level_w(m_speaker_state);

	// setup save states
	save_item(NAME(m_speaker_state));

	// setup video pointers
	m_video->set_ram_pointers(m_ram_ptr, m_ram_ptr);
	m_video->set_char_pointer(nullptr, 0);  // no text modes on this machine
}

void superga2_state::machine_reset()
{
	uint8_t *user1 = memregion("maincpu")->base();

	memcpy(&m_ram_ptr[0x1100], user1, 0x8000);
}

/***************************************************************************
    I/O
***************************************************************************/

uint8_t superga2_state::speaker_toggle_r()
{
	if (!machine().side_effects_disabled())
		speaker_toggle_w(0);
	return read_floatingbus();
}

void superga2_state::speaker_toggle_w(uint8_t data)
{
	m_speaker_state ^= 1;
	m_speaker->level_w(m_speaker_state);
}

uint8_t superga2_state::switches_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_softlatch->write_bit((offset & 0x0e) >> 1, offset & 0x01);
	return read_floatingbus();
}

uint8_t superga2_state::reset_r(offs_t offset)
{
	switch (offset)
	{
	case 0: case 2: return 0x00; break;
	case 1: case 3: return 0x11; break;
	}
	return 0xff;
}

uint8_t superga2_state::read_floatingbus()
{
	return 0xff;
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

uint8_t superga2_state::ram_r(offs_t offset)
{
	if (offset < m_ram_size)
	{
		return m_ram_ptr[offset];
	}

	return 0xff;
}

void superga2_state::ram_w(offs_t offset, uint8_t data)
{
	if (offset < m_ram_size)
	{
		m_ram_ptr[offset] = data;
	}
}

void superga2_state::superga2_map(address_map &map)
{
	map(0x0000, 0xbfff).rw(FUNC(superga2_state::ram_r), FUNC(superga2_state::ram_w));
	map(0xc000, 0xc000).mirror(0xf).portr("P1").nopw();
	map(0xc030, 0xc030).mirror(0xf).rw(FUNC(superga2_state::speaker_toggle_r), FUNC(superga2_state::speaker_toggle_w));
	map(0xc050, 0xc05f).r(FUNC(superga2_state::switches_r)).w(m_softlatch, FUNC(addressable_latch_device::write_a0));
	map(0xfffc, 0xffff).r(FUNC(superga2_state::reset_r));
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( superga2 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
INPUT_PORTS_END

void superga2_state::superga2(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &superga2_state::superga2_map);

	APPLE2_VIDEO(config, m_video, XTAL(14'318'181)).set_screen(m_screen);
	APPLE2_COMMON(config, m_a2common, XTAL(14'318'181));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(1021800*14, (65*7)*2, 0, (40*7)*2, 262, 0, 192);
	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::II, false, false>)));
	m_screen->set_palette(m_video);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* soft switches */
	F9334(config, m_softlatch);
	m_softlatch->q_out_cb<2>().set(m_video, FUNC(a2_video_device::scr_w));
	// these don't cause mode changes
	// m_softlatch->q_out_cb<0>().set(m_video, FUNC(a2_video_device::txt_w));
	// m_softlatch->q_out_cb<1>().set(m_video, FUNC(a2_video_device::mix_w));
	// m_softlatch->q_out_cb<3>().set(m_video, FUNC(a2_video_device::res_w));

	RAM(config, RAM_TAG).set_default_size("48K").set_default_value(0x00);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(kuzmich)
	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("ke.bin", 0x0000, 0x8000, CRC(102d246b) SHA1(492dcdf0cc31190a97057a69010e2c9c23b6e59d))
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROT,   COMPANY    FULLNAME
GAME( 199?, kuzmich,  0,      superga2, superga2, superga2_state, empty_init, ROT0,  "Nippel",  "Kuzmich-Egorych", MACHINE_SUPPORTS_SAVE )
