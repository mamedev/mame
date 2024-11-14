// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Eggs & Dommy

Very similar to Burger Time hardware (and uses its video driver)

driver by Nicola Salmoria

To Do:
- Sprite Priorities in dommy


Rock Duck
---------

Rock Duck support added based on preliminary findings by Roberto Fresca
It looks like the game may be a ghastly hack of one of the 'eggs' titles
(need to check code but gameplay appears to be the same)

The dump was from a rom 'blister' (no PCB available) and is missing at
least the colour PROM.


Some notes by Roberto Fresca:
----------------------------

1 - The hack was made using "Scrambled Egg" program roms instead of "Eggs".

2 - The code is almost identical to scregg except for a couple of changed
    zero page registers, some data for gfx, strings, and two new subroutines.

Strictly to the code, they re-routed two subroutines to the unused high
program space where there are only zeroes:

$5732: jmp $7f0e (when start to drawing the screen)
This subroutine ($7f0e) is used to draw the game's frame with the holes and
tunnel.

$30ca: jsr $7f44 (called constantly)
This one is amazing!... They made this subroutine ($7f44) only to protect
the string "(R)DATEL SAS" placed at bottom-right of the screen. The code
compare the string stored in rom ($62d8 - $62e1) against the one drawn in
the videoram ($13cf - $13d8"). If something is different, just jump to a
reset ($3003) and the game starts again.

Obviously the string isn't easily visible. You need some arithmetic to show
it as ASCII text.


***************************************************************************/

#include "emu.h"
#include "btime.h"

#include "cpu/m6502/m6502.h"
#include "machine/timer.h"
#include "sound/ay8910.h"

#include "speaker.h"

namespace {

class scregg_state : public btime_state
{
public:
	scregg_state(const machine_config &mconfig, device_type type, const char *tag)
		: btime_state(mconfig, type, tag)
	{ }

	void scregg(machine_config &config);
	void dommy(machine_config &config);

	void init_rockduck();

private:
	void scregg_irqack_w(uint8_t data);
	uint8_t scregg_irqack_r();

	DECLARE_MACHINE_START(scregg);
	DECLARE_MACHINE_RESET(scregg);
	TIMER_DEVICE_CALLBACK_MEMBER(scregg_interrupt);
	void dommy_map(address_map &map) ATTR_COLD;

	void eggs_map(address_map &map) ATTR_COLD;
};



TIMER_DEVICE_CALLBACK_MEMBER(scregg_state::scregg_interrupt)
{
	// assume that the irq generator is similar to burgertime hw
	m_maincpu->set_input_line(0, (param & 8) ? ASSERT_LINE : CLEAR_LINE);
}

void scregg_state::scregg_irqack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

uint8_t scregg_state::scregg_irqack_r()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0;
}


void scregg_state::dommy_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x23ff).ram().share("videoram");
	map(0x2400, 0x27ff).ram().share("colorram");
	map(0x2800, 0x2bff).rw(FUNC(scregg_state::btime_mirrorvideoram_r), FUNC(scregg_state::btime_mirrorvideoram_w));
	map(0x4000, 0x4000).portr("DSW1").w(FUNC(scregg_state::scregg_irqack_w));
	map(0x4001, 0x4001).portr("DSW2").w(FUNC(scregg_state::btime_video_control_w));
	map(0x4002, 0x4002).portr("P1");
	map(0x4003, 0x4003).portr("P2");
	map(0x4004, 0x4005).w("ay1", FUNC(ay8910_device::address_data_w)).r(FUNC(scregg_state::scregg_irqack_r));
	map(0x4006, 0x4007).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xa000, 0xffff).rom();
}


void scregg_state::eggs_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x13ff).ram().share("videoram");
	map(0x1400, 0x17ff).ram().share("colorram");
	map(0x1800, 0x1bff).rw(FUNC(scregg_state::btime_mirrorvideoram_r), FUNC(scregg_state::btime_mirrorvideoram_w));
	map(0x1c00, 0x1fff).rw(FUNC(scregg_state::btime_mirrorcolorram_r), FUNC(scregg_state::btime_mirrorcolorram_w));
	map(0x2000, 0x2000).portr("DSW1").w(FUNC(scregg_state::btime_video_control_w));
	map(0x2001, 0x2001).portr("DSW2").w(FUNC(scregg_state::scregg_irqack_w));
	map(0x2002, 0x2002).portr("P1");
	map(0x2003, 0x2003).portr("P2");
	map(0x2004, 0x2005).w("ay1", FUNC(ay8910_device::address_data_w)).r(FUNC(scregg_state::scregg_irqack_r));
	map(0x2006, 0x2007).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x3000, 0x7fff).rom();
	map(0xf000, 0xffff).rom();    /* reset/interrupt vectors */
}


static INPUT_PORTS_START( scregg )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "50000" )
	PORT_DIPSETTING(    0x00, "70000"  )
	PORT_DIPSETTING(    0x06, "Never"  )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( rockduck )
	PORT_INCLUDE( scregg )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )

INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	3,      /* 3 bits per pixel */
	{ 2*1024*8*8, 1024*8*8, 0 },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	3,      /* 3 bits per pixel */
	{ 2*256*16*16, 256*16*16, 0 },  /* the bitplanes are separated */
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( gfx_scregg )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,          0, 1 )     /* char set #1 */
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,        0, 1 )     /* sprites */
GFXDECODE_END



MACHINE_START_MEMBER(scregg_state,scregg)
{
	save_item(NAME(m_btime_palette));
	save_item(NAME(m_bnj_scroll1));
	save_item(NAME(m_bnj_scroll2));
	save_item(NAME(m_btime_tilemap));
}

MACHINE_RESET_MEMBER(scregg_state,scregg)
{
	m_btime_palette = 0;
	m_bnj_scroll1 = 0;
	m_bnj_scroll2 = 0;
	m_btime_tilemap[0] = 0;
	m_btime_tilemap[1] = 0;
	m_btime_tilemap[2] = 0;
	m_btime_tilemap[3] = 0;
}

void scregg_state::dommy(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(12'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &scregg_state::dommy_map);
	TIMER(config, "irq").configure_scanline(FUNC(scregg_state::scregg_interrupt), "screen", 0, 8);

	MCFG_MACHINE_START_OVERRIDE(scregg_state,scregg)
	MCFG_MACHINE_RESET_OVERRIDE(scregg_state,scregg)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(12'000'000)/2, 384, 8, 248, 272, 8, 248);
	m_screen->set_screen_update(FUNC(scregg_state::screen_update_eggs));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_scregg);
	PALETTE(config, m_palette, FUNC(scregg_state::btime_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.23);

	AY8910(config, "ay2", XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.23);
}


void scregg_state::scregg(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(12'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &scregg_state::eggs_map);
	TIMER(config, "irq").configure_scanline(FUNC(scregg_state::scregg_interrupt), "screen", 0, 8);

	MCFG_MACHINE_START_OVERRIDE(scregg_state,scregg)
	MCFG_MACHINE_RESET_OVERRIDE(scregg_state,scregg)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(12'000'000)/2, 384, 8, 248, 272, 8, 248);
	m_screen->set_screen_update(FUNC(scregg_state::screen_update_eggs));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_scregg);
	PALETTE(config, m_palette, FUNC(scregg_state::btime_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.23);

	AY8910(config, "ay2", XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.23);
}


ROM_START( dommy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dommy.e01",  0xa000, 0x2000, CRC(9ae064ed) SHA1(73082e5254d54d8386f580cc82a74242a6debd84) )
	ROM_LOAD( "dommy.e11",  0xc000, 0x2000, CRC(7c4fad5c) SHA1(fb733ac979092a6fc278836b82d8ed3fae7a20d9) )
	ROM_LOAD( "dommy.e21",  0xe000, 0x2000, CRC(cd1a4d55) SHA1(f7f4f5ef2e89519652e8401e75dc4e2b8edf4bae) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "dommy.e50",  0x0000, 0x2000, CRC(5e9db0a4) SHA1(82e60d6b65a4d901102d7e195e66b278f18586f7) )
	ROM_LOAD( "dommy.e40",  0x2000, 0x2000, CRC(4d1c36fb) SHA1(5904421e8e2f727dd6292045871429a1373085e9) )
	ROM_LOAD( "dommy.e30",  0x4000, 0x2000, CRC(4e68bb12) SHA1(de26d278e43882deffad4d5b19d785f8824cf05a) )

	ROM_REGION( 0x0040, "proms", 0 ) /* palette decoding is probably wrong */
	ROM_LOAD( "dommy.e70",  0x0018, 0x0008, CRC(50c1d86e) SHA1(990a87a7f7e6a2af67dc6890e2326c7403e46520) )  /* palette */
	ROM_CONTINUE(             0x0000, 0x0018 )
	ROM_LOAD( "dommy.e60",  0x0020, 0x0020, CRC(24da2b63) SHA1(4db7e1ff1b9fd5ae4098cd7ca66cf1fa2574501a) )  /* unknown */
ROM_END

ROM_START( scregg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d00.e14",   0x3000, 0x1000, CRC(29226d77) SHA1(e1a329a4452eeb90801d001140ce865bf1ea7716) )
	ROM_LOAD( "d10.d14",   0x4000, 0x1000, CRC(eb143880) SHA1(73b3ca6e0d72cd0db951ae9ed1552cf8b7d91e68) )
	ROM_LOAD( "d20.c14",   0x5000, 0x1000, CRC(4455f262) SHA1(fc7b2d9094fa5e25c1bf4b68386f640f4502e0c0) )
	ROM_LOAD( "d30.b14",   0x6000, 0x1000, CRC(044ac5d2) SHA1(f2d2fe2236de1b3b2614cc95f61a90571638cd69) )
	ROM_LOAD( "d40.a14",   0x7000, 0x1000, CRC(b5a0814a) SHA1(192cdc506fb0bbfed8ae687f2699397ace3bef30) )
	ROM_RELOAD(            0xf000, 0x1000 )        /* for reset/interrupt vectors */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "d50.j12",   0x0000, 0x1000, CRC(a485c10c) SHA1(88edd35479ceb58244f644a7e0520d225df3bf65) )
	ROM_LOAD( "d60.j10",   0x1000, 0x1000, CRC(1fd4e539) SHA1(3067bbd9493614e80d8d3982fe80ef25688d256c) )
	ROM_LOAD( "d70.h12",   0x2000, 0x1000, CRC(8454f4b2) SHA1(6a8d257a3fec901453c7216ad894badf96188ebf) )
	ROM_LOAD( "d80.h10",   0x3000, 0x1000, CRC(72bd89ee) SHA1(2e38c27b546eeef0fe42340777c8687f4c65ee97) )
	ROM_LOAD( "d90.g12",   0x4000, 0x1000, CRC(ff3c2894) SHA1(0da866db6a79f658de3efc609b9ca8520b4d22d0) )
	ROM_LOAD( "da0.g10",   0x5000, 0x1000, CRC(9c20214a) SHA1(e01b72501a01ffc0370cf19c9a379a54800cccc6) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "dc0.c6",    0x0000, 0x0020, CRC(ff23bdd6) SHA1(d09738915da456449bb4e8d9eefb8e6378f0edea) )   /* palette */
	ROM_LOAD( "db1.b4",    0x0020, 0x0020, CRC(7cc4824b) SHA1(2a283fc17fac32e63385948bfe180d05f1fb8727) )   /* unknown */
ROM_END

ROM_START( eggs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d0a.e14",   0x3000, 0x1000, CRC(4e216f9d) SHA1(7b9d984481c8227e417dae4a1adbb5dec5f959b7) )
	ROM_LOAD( "d1b.d14",   0x4000, 0x1000, CRC(4edb267f) SHA1(f5d1a79b13d6fbb92561b4e4cfb78465114497d1) )
	ROM_LOAD( "d2b.c14",   0x5000, 0x1000, CRC(15a5c48c) SHA1(70141c739a8c019554a6c5257ad12606a1542b1f) )
	ROM_LOAD( "d3a.b14",   0x6000, 0x1000, CRC(5c11c00e) SHA1(4a9295086bf935a1c9b1b01f83d1ff6242d74907) )
	ROM_LOAD( "d4b.a14",   0x7000, 0x1000, CRC(953faf07) SHA1(ee3181e9ee664053d6e6899fe38e136a9ea6abe1) )
	ROM_RELOAD(            0xf000, 0x1000 )   /* for reset/interrupt vectors */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "d5a.j12",   0x0000, 0x1000, CRC(ce4a2e46) SHA1(6b31c481ca038834ae295d015054f852baa6330f) )
	ROM_LOAD( "d6a.j10",   0x1000, 0x1000, CRC(a1bcaffc) SHA1(74f6df3136826822bbc22b027700fb3ddfceaa97) )
	ROM_LOAD( "d7a.h12",   0x2000, 0x1000, CRC(9562836d) SHA1(c5d5d6ceede6105975c87ff8e1f7e5312b992b92) )
	ROM_LOAD( "d8a.h10",   0x3000, 0x1000, CRC(3cfb3a8e) SHA1(e60c9da1a7841c3bb5351a109d99c8df34747212) )
	ROM_LOAD( "d9a.g12",   0x4000, 0x1000, CRC(679f8af7) SHA1(f69302ff0125d96fbfdd914d7ecefd7130a24616) )
	ROM_LOAD( "daa.g10",   0x5000, 0x1000, CRC(5b58d3b5) SHA1(f138b7294dd20d050bb8a2191e87b0c3815f6148) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "dca.c6",    0x0000, 0x0020, CRC(e8408c81) SHA1(549b9948a4a73e7a704731b942565183cef05d52) )   /* palette - TI TBP18S030N BPROM */
	ROM_LOAD( "db1.b4",    0x0020, 0x0020, CRC(7cc4824b) SHA1(2a283fc17fac32e63385948bfe180d05f1fb8727) )   /* unknown - TI TBP18S030N BPROM */
ROM_END

// rockduck - check gfx roms (planes) order

ROM_START( rockduck )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rde.bin",    0x4000, 0x2000, CRC(56e2a030) SHA1(f03cca53ac30f1c4ec45afbe58c231673739e425) )
	ROM_COPY( "maincpu",    0x5000, 0x3000, 0x1000 ) // rgn,srcoffset,offset,length.
	ROM_LOAD( "rdc.bin",    0x6000, 0x2000, CRC(482d9a0c) SHA1(2838cbcd35edaf19848fcf1588ec3a35adf5b179) )
	ROM_COPY( "maincpu",    0x7000, 0x5000, 0x1000 ) // rgn,srcoffset,offset,length.
	ROM_LOAD( "rdb.bin",    0x8000, 0x2000, CRC(974626f2) SHA1(cfd767947df9aa99b22afbc0a83afd3f92e7d903) )
	ROM_RELOAD(             0xe000, 0x2000 )    // for vectors/pointers
	ROM_COPY( "maincpu",    0x9000, 0x7000, 0x1000 ) // rgn,srcoffset,offset,length.

//  ROM_LOAD( "b.bin",  0x8000, 0x2000, CRC(637fbb50) SHA1(b31799f9cc6aefd9f4b39cc1afb1ca00d9200efb) ) // alternate rom, bad dump
//  this rom is a bad dump of rdb.bin with only 1 bit different.
//  (bit 5 is on at offset $1629).

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "rd3.rdg",    0x0000, 0x2000, CRC(8a3f1e53) SHA1(398bbbab314e4ea87cc5f5978c7e806818398d02) ) // not scrambled
	ROM_LOAD( "rd2.rdh",    0x2000, 0x2000, CRC(e94e673e) SHA1(0adf01d35879b9dd355d0c53a51b5f416f22d7b2) )
	ROM_LOAD( "rd1.rdj",    0x4000, 0x2000, CRC(654afff2) SHA1(f1e21447f0a2ac23cd64cf1f6f315937787b6377) )

	ROM_REGION( 0x0040, "proms", 0 )
	/* no proms were in the rock duck set and no PCB available, use eggs palette for now, although its probably wrong */
	ROM_LOAD( "eggs.c6",      0x0000, 0x0020, BAD_DUMP CRC(e8408c81) SHA1(549b9948a4a73e7a704731b942565183cef05d52) )   /* palette */
//  ROM_LOAD( "screggco.b4",  0x0020, 0x0020, BAD_DUMP CRC(7cc4824b) SHA1(2a283fc17fac32e63385948bfe180d05f1fb8727) )   /* unknown */
ROM_END


void scregg_state::init_rockduck()
{
	// rd2.rdh and rd1.rdj are bitswapped, but not rd3.rdg .. are they really from the same board?
	uint8_t *src = memregion("gfx1")->base();

	for (int x = 0x2000; x < 0x6000; x++)
	{
		src[x] = bitswap<8>(src[x],2,0,3,6,1,4,7,5);

	}
}

} // anonymous namespace


GAME( 1983, dommy,    0,        dommy,  scregg,   scregg_state, empty_init,    ROT270, "Technos Japan", "Dommy", MACHINE_SUPPORTS_SAVE )
GAME( 1983, scregg,   0,        scregg, scregg,   scregg_state, empty_init,    ROT270, "Technos Japan", "Scrambled Egg", MACHINE_SUPPORTS_SAVE )
GAME( 1983, eggs,     scregg,   scregg, scregg,   scregg_state, empty_init,    ROT270, "Technos Japan (Universal USA license)", "Eggs (USA)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, rockduck, 0,        scregg, rockduck, scregg_state, init_rockduck, ROT270, "Datel SAS", "Rock Duck (prototype?)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
