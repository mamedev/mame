// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

Tank Battalion memory map (verified)

driver by Brad Oliver

$0000-$000f : bullet ram, first entry is player's bullet
$0010-$01ff : zero page & stack
$0200-$07ff : RAM
$0800-$0bff : videoram
$0c00-$0c1f : I/O

Read:
    $0c00-$0c03 : p1 joystick
    $0c04
    $0c07       : stop at grid self-test if bit 7 is low
    $0c0f       : stop at first self-test if bit 7 is low

    $0c18       : Cabinet, 0 = table, 1 = upright
    $0c19-$0c1a : Coinage, 00 = free play, 01 = 2 coin 1 credit, 10 = 1 coin 2 credits, 11 = 1 coin 1 credit
    $0c1b-$0c1c : Bonus, 00 = 10000, 01 = 15000, 10 = 20000, 11 = none
    $0c1d       : Tanks, 0 = 3, 1 = 2
    $0c1e-$0c1f : ??

Write:
    $0c00-$0c01 : p1/p2 start leds
    $0c02       : ?? written to at end of IRQ, either 0 or 1 - coin counter?
    $0c03       : ?? written to during IRQ if grid test is on
    $0c08       : ?? written to during IRQ if grid test is on
    $0c09       : Sound - coin ding
    $0c0a       : Sound enable (active low) ?? game only ??
    $0c0b       : Sound - background noise, 0 - low rumble, 1 - high rumble
    $0c0c       : Sound - player fire
    $0c0d       : Sound - explosion
    $0c0f       : NMI enable (active high) ?? demo only ??

    $0c10       : IRQ ack (written at the end of the irq routine)
    $0c18       : Watchdog

$2000-$3fff : ROM (A14, A15 not decoded)

TODO:
    . Needs proper discrete emulation
    . Resistor values on the color prom need to be corrected

Known issues:
    . The 'moving' tank rumble noise seems to keep playing a second too long
    . Sample support is all a crapshoot. I have no idea how it really works

***************************************************************************/

#include "emu.h"
#include "includes/tankbatt.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/input_merger.h"
#include "machine/watchdog.h"
#include "sound/samples.h"
#include "screen.h"
#include "speaker.h"


void tankbatt_state::machine_start()
{
	save_item(NAME(m_sound_enable));
}

uint8_t tankbatt_state::in0_r(offs_t offset)
{
	int val;

	val = ioport("P1")->read();
	return ((val << (7 - offset)) & 0x80);
}

uint8_t tankbatt_state::in1_r(offs_t offset)
{
	int val;

	val = ioport("P2")->read();
	return ((val << (7 - offset)) & 0x80);
}

uint8_t tankbatt_state::dsw_r(offs_t offset)
{
	int val;

	val = ioport("DSW")->read();
	return ((val << (7 - offset)) & 0x80);
}

WRITE_LINE_MEMBER(tankbatt_state::sound_off_w)
{
	m_sound_enable = !state;

	// turn off the engine noise
	if (state) m_samples->stop(2);
}

WRITE_LINE_MEMBER(tankbatt_state::sh_expl_w)
{
	if (state) // rising edge
	{
		m_samples->start(1, 3);
	}
}

WRITE_LINE_MEMBER(tankbatt_state::sh_engine_w)
{
	if (m_sound_enable)
	{
		if (state)
			m_samples->start(2, 2, true);
		else
			m_samples->start(2, 1, true);
	}
	else m_samples->stop(2);
}

WRITE_LINE_MEMBER(tankbatt_state::sh_fire_w)
{
	if (state) // rising edge
	{
		m_samples->start(0, 0);
	}
}

void tankbatt_state::intack_w(uint8_t data)
{
	m_maincpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
}

WRITE_LINE_MEMBER(tankbatt_state::coincounter_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(tankbatt_state::coinlockout_w)
{
	machine().bookkeeping().coin_lockout_w(0, state);
	machine().bookkeeping().coin_lockout_w(1, state);
}

void tankbatt_state::main_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x000f).mirror(0x1000).ram().share("bulletsram");
	map(0x0010, 0x07ff).mirror(0x1000).ram();
	map(0x0800, 0x0bff).mirror(0x1000).ram().w(FUNC(tankbatt_state::videoram_w)).share("videoram");
	map(0x0c00, 0x0c07).mirror(0x13e0).r(FUNC(tankbatt_state::in0_r)).w("outlatch0", FUNC(cd4099_device::write_d0));
	map(0x0c08, 0x0c0f).mirror(0x13e0).r(FUNC(tankbatt_state::in1_r)).w("outlatch1", FUNC(cd4099_device::write_d0));
	map(0x0c10, 0x0c10).mirror(0x13e7).w(FUNC(tankbatt_state::intack_w));
	map(0x0c18, 0x0c1f).mirror(0x13e0).r(FUNC(tankbatt_state::dsw_r));
	map(0x0c18, 0x0c18).mirror(0x13e7).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x2000, 0x3fff).rom().region("maincpu", 0);
}

TIMER_DEVICE_CALLBACK_MEMBER(tankbatt_state::scanline_interrupt)
{
	m_maincpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);
}

static INPUT_PORTS_START( tankbatt )
	PORT_START("P1")    /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("P2")    /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )         PORT_DIPLOCATION("DSW:8")

	PORT_START("DSW")   /* DSW */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW:1,2")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW:3,4")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "DSW:6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	1,  /* 1 bit per pixel */
	{ 0 },  /* only one bitplane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout bulletlayout =
{
	/* there is no gfx ROM for this one, it is generated by the hardware */
	3,3,    /* 3*3 box */
	1,  /* just one */
	1,  /* 1 bit per pixel */
	{ 8*8 },
	{ 2, 2, 2 },   /* I "know" that this bit of the */
	{ 2, 2, 2 },   /* graphics ROMs is 1 */
	0   /* no use */
};


static GFXDECODE_START( gfx_tankbatt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 256 )
	GFXDECODE_ENTRY( "gfx1", 0, bulletlayout, 0, 256 )
GFXDECODE_END



static const char *const tankbatt_sample_names[] =
{
	"*tankbatt",
	"fire",
	"engine1",
	"engine2",
	"explode1",
	nullptr   /* end of array */
};


void tankbatt_state::tankbatt(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 18.432_MHz_XTAL / 24); // ϕ0 = 4H
	m_maincpu->set_addrmap(AS_PROGRAM, &tankbatt_state::main_map);

	INPUT_MERGER_ALL_HIGH(config, "nmigate").output_handler().set_inputline(m_maincpu, m6502_device::NMI_LINE);

	TIMER(config, "16v").configure_scanline(FUNC(tankbatt_state::scanline_interrupt), "screen", 16, 32);

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 16); // system reset pulse is ripple carry output of 74LS161 at 7C

	cd4099_device &outlatch0(CD4099(config, "outlatch0")); // 4099 at 5H or 259 at 5J
	outlatch0.q_out_cb<0>().set_output("led0");
	outlatch0.q_out_cb<1>().set_output("led1");
	outlatch0.q_out_cb<2>().set(FUNC(tankbatt_state::coincounter_w));
	outlatch0.q_out_cb<3>().set(FUNC(tankbatt_state::coinlockout_w));
	// Q4 through Q7 are not connected

	cd4099_device &outlatch1(CD4099(config, "outlatch1")); // 4099 at 4H or 259 at 4J
	outlatch1.q_out_cb<0>().set_nop(); // TODO: S1 tone
	outlatch1.q_out_cb<1>().set_nop(); // TODO: S2 tone
	outlatch1.q_out_cb<2>().set(FUNC(tankbatt_state::sound_off_w));
	outlatch1.q_out_cb<3>().set(FUNC(tankbatt_state::sh_engine_w));
	outlatch1.q_out_cb<4>().set(FUNC(tankbatt_state::sh_fire_w));
	outlatch1.q_out_cb<5>().set(FUNC(tankbatt_state::sh_expl_w)); // bit 7 also set by ASL instruction
	// Q6 is not connected
	outlatch1.q_out_cb<7>().set("nmigate", FUNC(input_merger_device::in_w<1>));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL / 3, 384, 0, 256, 264, 16, 240); // timing chain is same as in Galaxian
	screen.set_screen_update(FUNC(tankbatt_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set("nmigate", FUNC(input_merger_device::in_w<0>));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tankbatt);
	PALETTE(config, m_palette, FUNC(tankbatt_state::tankbatt_palette), 256*2, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(3);
	m_samples->set_samples_names(tankbatt_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tankbatt )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "tb1-1.1a",  0x0000, 0x0800, CRC(278a0b8c) SHA1(11ea8fe8401b3cd986616a30a759c0ac1a5ce73b) )
	ROM_LOAD( "tb1-2.1b",  0x0800, 0x0800, CRC(e0923370) SHA1(8d3dbea877bed9f9c267d8002dc180f6eb1e5a8f) )
	ROM_LOAD( "tb1-3.1c",  0x1000, 0x0800, CRC(85005ea4) SHA1(91583081803a5ef600fb90bee34be9edd87f157e) )
	ROM_LOAD( "tb1-4.1d",  0x1800, 0x0800, CRC(3dfb5bcf) SHA1(aa24bf74f4d5dc81baf3843196c837e0b731077b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tb1-5.2k",  0x0000, 0x0800, CRC(aabd4fb1) SHA1(5cff659b531d0f1b6faa503f7c06045c3a209a84) )

	ROM_REGION( 0x0200, "proms", 0 ) /* prom is a Fujitsu MB7052 or equivalent */
	ROM_LOAD( "bct1-1.l3", 0x0000, 0x0100, CRC(d17518bc) SHA1(f3b0deffa586808bc59e9a24ec1699c54ebe84cc) )
ROM_END

ROM_START( tankbattb ) /* board with "NAMCO" removed from gfx1 rom, otherwise identical to original */
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "tb1-1.1a",  0x0000, 0x0800, CRC(278a0b8c) SHA1(11ea8fe8401b3cd986616a30a759c0ac1a5ce73b) ) // a.1a
	ROM_LOAD( "tb1-2.1b",  0x0800, 0x0800, CRC(e0923370) SHA1(8d3dbea877bed9f9c267d8002dc180f6eb1e5a8f) ) // b.1b
	ROM_LOAD( "tb1-3.1c",  0x1000, 0x0800, CRC(85005ea4) SHA1(91583081803a5ef600fb90bee34be9edd87f157e) ) // c.1c
	ROM_LOAD( "tb1-4.1d",  0x1800, 0x0800, CRC(3dfb5bcf) SHA1(aa24bf74f4d5dc81baf3843196c837e0b731077b) ) // d.1d

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "e.2k",  0x0000, 0x0800, CRC(249f4e1b) SHA1(8654e8f9aa042ba49f20a58ff21879a593da57a3) )

	ROM_REGION( 0x0200, "proms", 0 ) /* prom is a Fujitsu MB7052 or equivalent */
	ROM_LOAD( "bct1-1.l3", 0x0000, 0x0100, CRC(d17518bc) SHA1(f3b0deffa586808bc59e9a24ec1699c54ebe84cc) ) // dm74s287n.3l
ROM_END

GAME( 1980, tankbatt,  0,        tankbatt, tankbatt, tankbatt_state, empty_init, ROT90, "Namco",   "Tank Battalion", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS |MACHINE_SUPPORTS_SAVE )
GAME( 1980, tankbattb, tankbatt, tankbatt, tankbatt, tankbatt_state, empty_init, ROT90, "bootleg", "Tank Battalion (bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
