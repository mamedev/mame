// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Stefan Jokisch
/***************************************************************************

Atari Ultra Tank driver

***************************************************************************/

#include "emu.h"
#include "includes/ultratnk.h"

#include "audio/sprint4.h"
#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "speaker.h"

#define MASTER_CLOCK    XTAL(12'096'000)

#define HTOTAL 384
#define VTOTAL 262

#define PIXEL_CLOCK    (MASTER_CLOCK / 2)



template <int N>
READ_LINE_MEMBER(ultratnk_state::collision_flipflop_r)
{
	return m_collision[N];
}


template <int N>
READ_LINE_MEMBER(ultratnk_state::joystick_r)
{
	uint8_t joy = m_joy[N]->read() & 3;

	if (joy == 1)
	{
		return (m_da_latch < 8) ? 1 : 0;
	}
	if (joy == 2)
	{
		return 0;
	}

	return 1;
}


TIMER_CALLBACK_MEMBER(ultratnk_state::nmi_callback)
{
	int scanline = param + 64;

	if (scanline >= VTOTAL)
		scanline = 32;

	/* NMI and watchdog are disabled during service mode */

	m_watchdog->watchdog_enable(ioport("IN0")->read() & 0x40);

	if (ioport("IN0")->read() & 0x40)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	m_nmi_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


void ultratnk_state::machine_start()
{
	m_nmi_timer = timer_alloc(FUNC(ultratnk_state::nmi_callback), this);

	save_item(NAME(m_da_latch));
	save_item(NAME(m_collision));
}


void ultratnk_state::machine_reset()
{
	m_nmi_timer->adjust(m_screen->time_until_pos(32), 32);
}


uint8_t ultratnk_state::wram_r(offs_t offset)
{
	return m_videoram[0x380 + offset];
}


uint8_t ultratnk_state::analog_r(offs_t offset)
{
	return (ioport("ANALOG")->read() << (~offset & 7)) & 0x80;
}
uint8_t ultratnk_state::coin_r(offs_t offset)
{
	return (ioport("COIN")->read() << (~offset & 7)) & 0x80;
}
uint8_t ultratnk_state::collision_r(offs_t offset)
{
	return (ioport("COLLISION")->read() << (~offset & 7)) & 0x80;
}


uint8_t ultratnk_state::options_r(offs_t offset)
{
	return (ioport("DIP")->read() >> (2 * (offset & 3))) & 3;
}


void ultratnk_state::wram_w(offs_t offset, uint8_t data)
{
	m_videoram[0x380 + offset] = data;
}


void ultratnk_state::collision_reset_w(offs_t offset, uint8_t data)
{
	m_collision[(offset >> 1) & 3] = 0;
}


void ultratnk_state::da_latch_w(uint8_t data)
{
	m_da_latch = data & 15;
}


WRITE_LINE_MEMBER(ultratnk_state::lockout_w)
{
	machine().bookkeeping().coin_lockout_global_w(!state);
}


void ultratnk_state::attract_w(uint8_t data)
{
	m_discrete->write(ULTRATNK_ATTRACT_EN, data & 1);
}
void ultratnk_state::explosion_w(uint8_t data)
{
	m_discrete->write(ULTRATNK_EXPLOSION_DATA, data & 15);
}


void ultratnk_state::ultratnk_cpu_map(address_map &map)
{

	map.global_mask(0x3fff);

	map(0x0000, 0x007f).mirror(0x700).ram();
	map(0x0080, 0x00ff).mirror(0x700).rw(FUNC(ultratnk_state::wram_r), FUNC(ultratnk_state::wram_w));
	map(0x0800, 0x0bff).mirror(0x400).ram().w(FUNC(ultratnk_state::video_ram_w)).share("videoram");

	map(0x1000, 0x17ff).portr("IN0");
	map(0x1800, 0x1fff).portr("IN1");

	map(0x2000, 0x2007).mirror(0x718).r(FUNC(ultratnk_state::analog_r));
	map(0x2020, 0x2027).mirror(0x718).r(FUNC(ultratnk_state::coin_r));
	map(0x2040, 0x2047).mirror(0x718).r(FUNC(ultratnk_state::collision_r));
	map(0x2060, 0x2063).mirror(0x71c).r(FUNC(ultratnk_state::options_r));

	map(0x2000, 0x2000).mirror(0x71f).w(FUNC(ultratnk_state::attract_w));
	map(0x2020, 0x2027).mirror(0x718).w(FUNC(ultratnk_state::collision_reset_w));
	map(0x2040, 0x2041).mirror(0x718).w(FUNC(ultratnk_state::da_latch_w));
	map(0x2042, 0x2043).mirror(0x718).w(FUNC(ultratnk_state::explosion_w));
	map(0x2044, 0x2045).mirror(0x718).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x2060, 0x206f).mirror(0x710).w("latch", FUNC(f9334_device::write_a0));

	map(0x2800, 0x2fff).noprw(); /* diagnostic ROM */
	map(0x3000, 0x3fff).rom();

}


static INPUT_PORTS_START( ultratnk )
	PORT_START("IN0")
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Option 1") PORT_TOGGLE

	PORT_START("COLLISION")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(ultratnk_state, collision_flipflop_r<0>)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(ultratnk_state, collision_flipflop_r<1>)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* VCC */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(ultratnk_state, collision_flipflop_r<2>)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )   /* SLAM */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(ultratnk_state, collision_flipflop_r<3>)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Option 2") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_NAME("Option 3") PORT_TOGGLE

	PORT_START("DIP")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIP:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIP:6,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x10, "Game Length" ) PORT_DIPLOCATION("DIP:4,3")
	PORT_DIPSETTING(    0x00, "60 Seconds" )
	PORT_DIPSETTING(    0x10, "90 Seconds" )
	PORT_DIPSETTING(    0x20, "120 Seconds" )
	PORT_DIPSETTING(    0x30, "150 Seconds" )
	PORT_DIPNAME( 0xc0, 0x40, "Extended Play" ) PORT_DIPLOCATION("DIP:2,1")
	PORT_DIPSETTING(    0x40, "25 Points" )
	PORT_DIPSETTING(    0x80, "50 Points" )
	PORT_DIPSETTING(    0xc0, "75 Points" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("ANALOG")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(ultratnk_state, joystick_r<0>) // "JOY-W"
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(ultratnk_state, joystick_r<2>) // "JOY-Y"
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(ultratnk_state, joystick_r<1>) // "JOY-X"
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(ultratnk_state, joystick_r<3>) // "JOY-Z"

	PORT_START("JOY-W")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(1)

	PORT_START("JOY-X")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1)

	PORT_START("JOY-Y")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2)

	PORT_START("JOY-Z")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(2)

	PORT_START("MOTOR1")
	PORT_ADJUSTER( 35, "Motor 1 RPM" )

	PORT_START("MOTOR2")
	PORT_ADJUSTER( 40, "Motor 2 RPM" )

INPUT_PORTS_END


static const gfx_layout motion_layout =
{
	15, 16,
	RGN_FRAC(1,4),
	1,
	{ 0 },
	{
		7 + RGN_FRAC(0,4), 6 + RGN_FRAC(0,4), 5 + RGN_FRAC(0,4), 4 + RGN_FRAC(0,4),
		7 + RGN_FRAC(1,4), 6 + RGN_FRAC(1,4), 5 + RGN_FRAC(1,4), 4 + RGN_FRAC(1,4),
		7 + RGN_FRAC(2,4), 6 + RGN_FRAC(2,4), 5 + RGN_FRAC(2,4), 4 + RGN_FRAC(2,4),
		7 + RGN_FRAC(3,4), 6 + RGN_FRAC(3,4), 5 + RGN_FRAC(3,4)
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80
};


static GFXDECODE_START( gfx_ultratnk )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1, 0, 5 )
	GFXDECODE_ENTRY( "gfx2", 0, motion_layout, 0, 5 )
GFXDECODE_END


void ultratnk_state::ultratnk(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, PIXEL_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &ultratnk_state::ultratnk_cpu_map);

	f9334_device &latch(F9334(config, "latch")); // E11
	latch.q_out_cb<3>().set(FUNC(ultratnk_state::lockout_w));
	latch.q_out_cb<4>().set_output("led0"); // LED1 (left player start)
	latch.q_out_cb<5>().set_output("led1"); // LED2 (right player start)
	latch.q_out_cb<6>().set(m_discrete, FUNC(discrete_device::write_line<ULTRATNK_FIRE_EN_2>));
	latch.q_out_cb<7>().set(m_discrete, FUNC(discrete_device::write_line<ULTRATNK_FIRE_EN_1>));

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count(m_screen, 8);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, 0, 256, VTOTAL, 0, 224);
	m_screen->set_screen_update(FUNC(ultratnk_state::screen_update));
	m_screen->screen_vblank().set(FUNC(ultratnk_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ultratnk);
	PALETTE(config, m_palette, FUNC(ultratnk_state::ultratnk_palette), 10, 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, ultratnk_discrete).add_route(0, "mono", 1.0);
}


ROM_START( ultratnk )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "030180.n1",  0x3000, 0x0800, CRC(b6aa6056) SHA1(6de094017b5d87a238053fac88129d20260f8222) ) /* ROM 3 */
	ROM_LOAD_NIB_HIGH( "030181.k1",  0x3000, 0x0800, CRC(17145c97) SHA1(afe0c9c562c27cd1fba57ea83377b0a4c12496db) ) /* ROM 3 */
	ROM_LOAD_NIB_LOW ( "030182.m1",  0x3800, 0x0800, CRC(034366a2) SHA1(dc289ce4c79e9937977ca8804ce07b4c8e40e969) ) /* ROM 4 */
	ROM_LOAD_NIB_HIGH( "030183.l1",  0x3800, 0x0800, CRC(be141602) SHA1(17aad9bab9bf6bd22dc3c2214b049bbd68c87380) ) /* ROM 4 */

	ROM_REGION( 0x0200, "gfx1", 0 ) /* playfield */
	ROM_LOAD_NIB_HIGH( "30172-01.j6", 0x0000, 0x0200, CRC(1d364b23) SHA1(44c5792ed3f33f40cd8632718b0e82152559ecdf) )
	ROM_LOAD_NIB_LOW ( "30173-01.h6", 0x0000, 0x0200, CRC(5c32f331) SHA1(c1d675891490fbc533eaa0da57545398d7325df8) )

	ROM_REGION( 0x1000, "gfx2", 0 ) /* motion */
	ROM_LOAD( "30174-01.n6", 0x0000, 0x0400, CRC(d0e20e73) SHA1(0df1ed4a73255032bb809fb4d0a4bf3f151c749d) )
	ROM_LOAD( "30175-01.m6", 0x0400, 0x0400, CRC(a47459c9) SHA1(4ca92edc172fbac923ba71731a25546c04ffc7b0) )
	ROM_LOAD( "30176-01.l6", 0x0800, 0x0400, CRC(1cc7c2dd) SHA1(7f8aebe8375751183afeae35ea2d241d22ee7a4f) )
	ROM_LOAD( "30177-01.k6", 0x0c00, 0x0400, CRC(3a91b09f) SHA1(1e713cb612eb7d78fc4a003e4e60308f62e0b169) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "30218-01.j10", 0x0000, 0x0020, CRC(d7a2c7b4) SHA1(7453921ecb6268b604dee3743f6e217db19c9871) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) /* SYNC */
ROM_END


GAME( 1978, ultratnk, 0, ultratnk, ultratnk, ultratnk_state, empty_init, ROT0, "Atari (Kee Games)", "Ultra Tank", MACHINE_SUPPORTS_SAVE )
