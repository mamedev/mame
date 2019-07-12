// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Sprint 4 driver

***************************************************************************/

#include "emu.h"
#include "includes/sprint4.h"
#include "audio/sprint4.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "speaker.h"

#define MASTER_CLOCK    12096000

#define HTOTAL 384
#define VTOTAL 262

#define PIXEL_CLOCK    (MASTER_CLOCK / 2)


CUSTOM_INPUT_MEMBER(sprint4_state::get_lever)
{
	int n = (uintptr_t) param;

	return 4 * m_gear[n] > m_da_latch;
}


CUSTOM_INPUT_MEMBER(sprint4_state::get_wheel)
{
	int n = (uintptr_t) param;

	return 8 * m_steer_FF1[n] + 8 * m_steer_FF2[n] > m_da_latch;
}


CUSTOM_INPUT_MEMBER(sprint4_state::get_collision)
{
	int n = (uintptr_t) param;

	return m_collision[n];
}


void sprint4_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_NMI:
		nmi_callback(ptr, param);
		break;
	default:
		assert_always(false, "Unknown id in sprint4_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(sprint4_state::nmi_callback)
{
	int scanline = param;

	/* MAME updates controls only once per frame but the game checks them on every NMI */

	uint8_t wheel[4] =
	{
		static_cast<uint8_t>(ioport("WHEEL1")->read()),
		static_cast<uint8_t>(ioport("WHEEL2")->read()),
		static_cast<uint8_t>(ioport("WHEEL3")->read()),
		static_cast<uint8_t>(ioport("WHEEL4")->read())
	};
	uint8_t lever[4] =
	{
		static_cast<uint8_t>(ioport("LEVER1")->read()),
		static_cast<uint8_t>(ioport("LEVER2")->read()),
		static_cast<uint8_t>(ioport("LEVER3")->read()),
		static_cast<uint8_t>(ioport("LEVER4")->read())
	};

	/* emulation of steering wheels isn't very accurate */

	for (int i = 0; i < 4; i++)
	{
		signed char delta = wheel[i] - m_last_wheel[i];

		if (delta < 0)
		{
			m_steer_FF2[i] = 0;
		}
		if (delta > 0)
		{
			m_steer_FF2[i] = 1;
		}

		m_steer_FF1[i] = (wheel[i] >> 4) & 1;

		if (lever[i] & 1) { m_gear[i] = 1; }
		if (lever[i] & 2) { m_gear[i] = 2; }
		if (lever[i] & 4) { m_gear[i] = 3; }
		if (lever[i] & 8) { m_gear[i] = 4; }

		m_last_wheel[i] = wheel[i];
	}

	scanline += 64;

	if (scanline >= VTOTAL)
	{
		scanline = 32;
	}

	/* NMI and watchdog are disabled during service mode */

	m_watchdog->watchdog_enable(ioport("IN0")->read() & 0x40);

	if (ioport("IN0")->read() & 0x40)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	m_nmi_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


void sprint4_state::machine_start()
{
	m_nmi_timer = timer_alloc(TIMER_NMI);

	save_item(NAME(m_da_latch));
	save_item(NAME(m_steer_FF1));
	save_item(NAME(m_steer_FF2));
	save_item(NAME(m_gear));
	save_item(NAME(m_last_wheel));
	save_item(NAME(m_collision));
}


void sprint4_state::machine_reset()
{
	m_nmi_timer->adjust(m_screen->time_until_pos(32), 32);

	memset(m_steer_FF1, 0, sizeof m_steer_FF1);
	memset(m_steer_FF2, 0, sizeof m_steer_FF2);

	m_gear[0] = 1;
	m_gear[1] = 1;
	m_gear[2] = 1;
	m_gear[3] = 1;

	m_da_latch = 0;
}


READ8_MEMBER(sprint4_state::wram_r)
{
	return m_videoram[0x380 + offset];
}


READ8_MEMBER(sprint4_state::analog_r)
{
	return (ioport("ANALOG")->read() << (~offset & 7)) & 0x80;
}
READ8_MEMBER(sprint4_state::coin_r)
{
	return (ioport("COIN")->read() << (~offset & 7)) & 0x80;
}
READ8_MEMBER(sprint4_state::collision_r)
{
	return (ioport("COLLISION")->read() << (~offset & 7)) & 0x80;
}


READ8_MEMBER(sprint4_state::options_r)
{
	return (ioport("DIP")->read() >> (2 * (offset & 3))) & 3;
}


WRITE8_MEMBER(sprint4_state::wram_w)
{
	m_videoram[0x380 + offset] = data;
}


WRITE8_MEMBER(sprint4_state::collision_reset_w)
{
	m_collision[(offset >> 1) & 3] = 0;
}


WRITE8_MEMBER(sprint4_state::da_latch_w)
{
	m_da_latch = data & 15;
}



#ifdef UNUSED_FUNCTION
WRITE8_MEMBER(sprint4_state::lockout_w)
{
	machine().bookkeeping().coin_lockout_global_w(~offset & 1);
}
#endif


WRITE8_MEMBER(sprint4_state::bang_w)
{
	m_discrete->write(SPRINT4_BANG_DATA, data & 0x0f);
}


WRITE8_MEMBER(sprint4_state::attract_w)
{
	m_discrete->write(SPRINT4_ATTRACT_EN, data & 1);
}


void sprint4_state::sprint4_cpu_map(address_map &map)
{

	map.global_mask(0x3fff);

	map(0x0080, 0x00ff).mirror(0x700).rw(FUNC(sprint4_state::wram_r), FUNC(sprint4_state::wram_w));
	map(0x0800, 0x0bff).mirror(0x400).ram().w(FUNC(sprint4_state::video_ram_w)).share("videoram");

	map(0x0000, 0x0007).mirror(0x718).r(FUNC(sprint4_state::analog_r));
	map(0x0020, 0x0027).mirror(0x718).r(FUNC(sprint4_state::coin_r));
	map(0x0040, 0x0047).mirror(0x718).r(FUNC(sprint4_state::collision_r));
	map(0x0060, 0x0063).mirror(0x71c).r(FUNC(sprint4_state::options_r));

	map(0x1000, 0x17ff).portr("IN0");
	map(0x1800, 0x1fff).portr("IN1");

	map(0x0000, 0x0000).mirror(0x71f).w(FUNC(sprint4_state::attract_w));
	map(0x0020, 0x0027).mirror(0x718).w(FUNC(sprint4_state::collision_reset_w));
	map(0x0040, 0x0041).mirror(0x718).w(FUNC(sprint4_state::da_latch_w));
	map(0x0042, 0x0043).mirror(0x718).w(FUNC(sprint4_state::bang_w));
	map(0x0044, 0x0045).mirror(0x718).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x0060, 0x006f).mirror(0x710).w("latch", FUNC(f9334_device::write_a0));

	map(0x2000, 0x27ff).noprw(); /* diagnostic ROM */
	map(0x2800, 0x3fff).rom();

}


static INPUT_PORTS_START( sprint4 )

	PORT_START("IN0")
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Track Select") PORT_CODE(KEYCODE_SPACE)

	PORT_START("COLLISION")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 1 Gas") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_collision, (void *)0 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 2 Gas") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_collision, (void *)1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 3 Gas") PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_collision, (void *)2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 4 Gas") PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_collision, (void *)3 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("DIP")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Language ) ) PORT_DIPLOCATION("DIP:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( German ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x03, DEF_STR( English ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x08, "Allow Late Entry" ) PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xf0, 0xb0, "Play Time" ) PORT_DIPLOCATION("DIP:4,3,2,1")
	PORT_DIPSETTING(    0x70, "60 seconds" )
	PORT_DIPSETTING(    0xb0, "90 seconds" )
	PORT_DIPSETTING(    0xd0, "120 seconds" )
	PORT_DIPSETTING(    0xe0, "150 seconds" )

	PORT_START("ANALOG")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_wheel, (void *)0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_lever, (void *)0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_wheel, (void *)1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_lever, (void *)1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_wheel, (void *)2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_lever, (void *)2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_wheel, (void *)3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sprint4_state, get_lever, (void *)3)

	PORT_START("WHEEL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_PLAYER(1)

	PORT_START("WHEEL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_PLAYER(2)

	PORT_START("WHEEL3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_PLAYER(3)

	PORT_START("WHEEL4")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_PLAYER(4)

	PORT_START("LEVER1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 1 Gear 1") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 1 Gear 2") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 1 Gear 3") PORT_CODE(KEYCODE_C) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 1 Gear 4") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)

	PORT_START("LEVER2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 2 Gear 1") PORT_CODE(KEYCODE_Q) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 2 Gear 2") PORT_CODE(KEYCODE_W) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 2 Gear 3") PORT_CODE(KEYCODE_E) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 2 Gear 4") PORT_CODE(KEYCODE_R) PORT_PLAYER(2)

	PORT_START("LEVER3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 3 Gear 1") PORT_CODE(KEYCODE_Y) PORT_PLAYER(3)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 3 Gear 2") PORT_CODE(KEYCODE_U) PORT_PLAYER(3)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 3 Gear 3") PORT_CODE(KEYCODE_I) PORT_PLAYER(3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 3 Gear 4") PORT_CODE(KEYCODE_O) PORT_PLAYER(3)

	PORT_START("LEVER4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 4 Gear 1") PORT_PLAYER(3)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 4 Gear 2") PORT_PLAYER(3)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 4 Gear 3") PORT_PLAYER(3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 4 Gear 4") PORT_PLAYER(3)

	PORT_START("MOTOR1")
	PORT_ADJUSTER( 35, "Motor 1 RPM" )

	PORT_START("MOTOR2")
	PORT_ADJUSTER( 40, "Motor 2 RPM" )

	PORT_START("MOTOR3")
	PORT_ADJUSTER( 35, "Motor 3 RPM" )

	PORT_START("MOTOR4")
	PORT_ADJUSTER( 40, "Motor 4 RPM" )

INPUT_PORTS_END


static const gfx_layout car_layout =
{
	12, 16,
	RGN_FRAC(1,3),
	1,
	{ 0 },
	{
		7 + RGN_FRAC(0,3), 6 + RGN_FRAC(0,3), 5 + RGN_FRAC(0,3), 4 + RGN_FRAC(0,3),
		7 + RGN_FRAC(1,3), 6 + RGN_FRAC(1,3), 5 + RGN_FRAC(1,3), 4 + RGN_FRAC(1,3),
		7 + RGN_FRAC(2,3), 6 + RGN_FRAC(2,3), 5 + RGN_FRAC(2,3), 4 + RGN_FRAC(2,3)
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80
};


static GFXDECODE_START( gfx_sprint4 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1, 0, 5 )
	GFXDECODE_ENTRY( "gfx2", 0, car_layout, 0, 5 )
GFXDECODE_END


void sprint4_state::sprint4(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, PIXEL_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &sprint4_state::sprint4_cpu_map);

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count(m_screen, 8);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, 0, 256, VTOTAL, 0, 224);
	m_screen->set_screen_update(FUNC(sprint4_state::screen_update));
	m_screen->screen_vblank().set(FUNC(sprint4_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sprint4);
	PALETTE(config, m_palette, FUNC(sprint4_state::sprint4_palette), 10, 6);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	f9334_device &latch(F9334(config, "latch")); // at E11
	latch.q_out_cb<0>().set_output("led0"); // START LAMP 1
	latch.q_out_cb<1>().set_output("led1"); // START LAMP 2
	latch.q_out_cb<2>().set_output("led2"); // START LAMP 3
	latch.q_out_cb<3>().set_output("led3"); // START LAMP 4
	latch.q_out_cb<4>().set("discrete", FUNC(discrete_device::write_line<SPRINT4_SCREECH_EN_1>));
	latch.q_out_cb<5>().set("discrete", FUNC(discrete_device::write_line<SPRINT4_SCREECH_EN_2>));
	latch.q_out_cb<6>().set("discrete", FUNC(discrete_device::write_line<SPRINT4_SCREECH_EN_3>));
	latch.q_out_cb<7>().set("discrete", FUNC(discrete_device::write_line<SPRINT4_SCREECH_EN_4>));

	DISCRETE(config, m_discrete, sprint4_discrete);
	m_discrete->add_route(0, "lspeaker", 1.0);
	m_discrete->add_route(1, "rspeaker", 1.0);
}


ROM_START( sprint4 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD         ( "30031.c1",    0x2800, 0x0800, CRC(017ee7c4) SHA1(9386cacc619669c18af31f66691a45af6dafef64) )
	ROM_LOAD_NIB_LOW ( "30036-02.n1", 0x3000, 0x0800, CRC(883b9d7c) SHA1(af52ffdd9cd8dfed54013c9b0d3c6e48c7419d17) )
	ROM_LOAD_NIB_HIGH( "30037-02.k1", 0x3000, 0x0800, CRC(c297fbd8) SHA1(8cc0f486429e12bee21a5dd1135e799196480044) )
	ROM_LOAD         ( "30033.e1",    0x3800, 0x0800, CRC(b8b717b7) SHA1(2f6b1a0e9803901d9ba79d1f19a025f6a6134756) )

	ROM_REGION( 0x0800, "gfx1", 0 ) /* playfield */
	ROM_LOAD( "30027.h5", 0x0000, 0x0800, CRC(3a752e07) SHA1(d990f94b296409d47e8bada98ddbed5f76567e1b) )

	ROM_REGION( 0x0c00, "gfx2", 0 ) /* motion */
	ROM_LOAD( "30028-01.n6", 0x0000, 0x0400, CRC(3ebcb13f) SHA1(e0b87239081f12f6613d3db6a8cb5b80937df7d7) )
	ROM_LOAD( "30029-01.m6", 0x0400, 0x0400, CRC(963a8424) SHA1(d52a0e73c54154531e825153012687bdb85e479a) )
	ROM_LOAD( "30030-01.l6", 0x0800, 0x0400, CRC(e94dfc2d) SHA1(9c5b1401c4aadda0a3aee76e4f92e73ae1d35cb7) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) /* SYNC */
ROM_END


ROM_START( sprint4a )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD         ( "30031.c1",    0x2800, 0x0800, CRC(017ee7c4) SHA1(9386cacc619669c18af31f66691a45af6dafef64) )
	ROM_LOAD_NIB_LOW ( "30036-02.n1", 0x3000, 0x0800, CRC(883b9d7c) SHA1(af52ffdd9cd8dfed54013c9b0d3c6e48c7419d17) )
	ROM_LOAD_NIB_HIGH( "30037-02.k1", 0x3000, 0x0800, CRC(c297fbd8) SHA1(8cc0f486429e12bee21a5dd1135e799196480044) )
	ROM_LOAD         ( "30033.e1",    0x3800, 0x0800, CRC(b8b717b7) SHA1(2f6b1a0e9803901d9ba79d1f19a025f6a6134756) )

	ROM_REGION( 0x0800, "gfx1", 0 ) /* playfield */
	ROM_LOAD( "30027.h5", 0x0000, 0x0800, CRC(3a752e07) SHA1(d990f94b296409d47e8bada98ddbed5f76567e1b) )

	ROM_REGION( 0x0c00, "gfx2", 0 ) /* motion */
	ROM_LOAD( "30028-03.n6", 0x0000, 0x0400, CRC(d3337030) SHA1(3e73fc45bdcaa52dc1aa01489b46240284562ab7) )
	ROM_LOAD( "30029-03.m6", 0x0400, 0x0400, NO_DUMP )
	ROM_LOAD( "30030-03.l6", 0x0800, 0x0400, CRC(aa1b45ab) SHA1(1ddb64d4ec92a1383866daaefa556499837decd1) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) /* SYNC */
ROM_END


GAME( 1977, sprint4,  0,       sprint4,  sprint4, sprint4_state, empty_init, ROT180, "Atari", "Sprint 4 (set 1)", MACHINE_SUPPORTS_SAVE ) /* large cars */
GAME( 1977, sprint4a, sprint4, sprint4,  sprint4, sprint4_state, empty_init, ROT180, "Atari", "Sprint 4 (set 2)", MACHINE_SUPPORTS_SAVE ) /* small cars */
