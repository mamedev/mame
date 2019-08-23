// license:BSD-3-Clause
// copyright-holders:R. Belmont, Sergey Svishchev
/***************************************************************************

    Super Games ][

    An arcade board designed to run modified games from Apple ][.
    Most of Apple hardware is missing, keyboard port is reused for joysticks.

    Only one game is known (a Mario Bros. hack/translation patch).

    Info: http://agatcomp.ru/Pravetz/SuperGames.shtml

    To do
    - verify palette, pixel and cpu clocks
    - two player mode

************************************************************************/

#include "emu.h"
#include "video/apple2.h"

#include "cpu/m6502/m6502.h"

#include "machine/74259.h"
#include "machine/apple2common.h"
#include "machine/ram.h"
#include "machine/timer.h"

#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"


#define A2_CPU_TAG "maincpu"
#define A2_VIDEO_TAG "a2video"

class superga2_state : public driver_device
{
public:
	superga2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, A2_CPU_TAG),
		m_screen(*this, "screen"),
		m_ram(*this, RAM_TAG),
		m_video(*this, A2_VIDEO_TAG),
		m_a2common(*this, "a2common"),
		m_speaker(*this, "speaker"),
		m_softlatch(*this, "softlatch")
	{ }

	static constexpr feature_type imperfect_features() { return feature::PALETTE | feature::CONTROLS; }

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ram_device> m_ram;
	required_device<a2_video_device> m_video;
	required_device<apple2_common_device> m_a2common;
	required_device<speaker_sound_device> m_speaker;
	required_device<addressable_latch_device> m_softlatch;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(speaker_toggle_r);
	DECLARE_WRITE8_MEMBER(speaker_toggle_w);
	DECLARE_READ8_MEMBER(switches_r);
	DECLARE_READ8_MEMBER(reset_r);

	void kuzmich(machine_config &config);
	void kuzmich_map(address_map &map);

private:
	int m_speaker_state;

	uint8_t *m_ram_ptr;
	int m_ram_size;

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
	m_video->m_ram_ptr = m_ram_ptr;
	m_video->m_aux_ptr = m_ram_ptr;
	m_video->m_char_ptr = memregion("gfx1")->base();
	m_video->m_char_size = memregion("gfx1")->bytes();
	m_video->m_sysconfig = 0;
}

void superga2_state::machine_reset()
{
	uint8_t *user1 = memregion("maincpu")->base();

	memcpy(&m_ram_ptr[0x1100], user1, 0x8000);
}

/***************************************************************************
    VIDEO
***************************************************************************/

uint32_t superga2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_video->hgr_update(screen, bitmap, cliprect, 0, 191);

	return 0;
}

/***************************************************************************
    I/O
***************************************************************************/

READ8_MEMBER(superga2_state::speaker_toggle_r)
{
	if (!machine().side_effects_disabled())
		speaker_toggle_w(space, offset, 0);
	return read_floatingbus();
}

WRITE8_MEMBER(superga2_state::speaker_toggle_w)
{
	m_speaker_state ^= 1;
	m_speaker->level_w(m_speaker_state);
}

READ8_MEMBER(superga2_state::switches_r)
{
	if (!machine().side_effects_disabled())
		m_softlatch->write_bit((offset & 0x0e) >> 1, offset & 0x01);
	return read_floatingbus();
}

READ8_MEMBER(superga2_state::reset_r)
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

READ8_MEMBER(superga2_state::ram_r)
{
	if (offset < m_ram_size)
	{
		return m_ram_ptr[offset];
	}

	return 0xff;
}

WRITE8_MEMBER(superga2_state::ram_w)
{
	if (offset < m_ram_size)
	{
		m_ram_ptr[offset] = data;
	}
}

void superga2_state::kuzmich_map(address_map &map)
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

static INPUT_PORTS_START( kuzmich )
	PORT_START("P1")
	// verified
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	// unverified
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

void superga2_state::kuzmich(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &superga2_state::kuzmich_map);

	APPLE2_VIDEO(config, m_video, XTAL(14'318'181)).set_screen(m_screen);
	APPLE2_COMMON(config, m_a2common, XTAL(14'318'181));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(1021800*14, (65*7)*2, 0, (40*7)*2, 262, 0, 192);
	m_screen->set_screen_update(FUNC(superga2_state::screen_update));
	m_screen->set_palette(m_video);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* soft switches */
	F9334(config, m_softlatch); // F14 (labeled 74LS259 on some boards and in the Apple ][ Reference Manual)
	m_softlatch->q_out_cb<0>().set(m_video, FUNC(a2_video_device::txt_w));
	m_softlatch->q_out_cb<1>().set(m_video, FUNC(a2_video_device::mix_w));
	m_softlatch->q_out_cb<2>().set(m_video, FUNC(a2_video_device::scr_w));
	m_softlatch->q_out_cb<3>().set(m_video, FUNC(a2_video_device::res_w));

	RAM(config, RAM_TAG).set_default_size("48K").set_default_value(0x00);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(kuzmich)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_FILL(0, 0x800, 0)
	ROM_REGION(0x8000,"maincpu",0)
	ROM_LOAD("ke.bin", 0x0000, 0x8000, CRC(102d246b) SHA1(492dcdf0cc31190a97057a69010e2c9c23b6e59d))
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS           INIT        COMPANY    FULLNAME
COMP( 199?, kuzmich,  0,      0,      kuzmich,  kuzmich, superga2_state, empty_init, "Nippel",  "Kuzmich-Egorych", MACHINE_SUPPORTS_SAVE )
