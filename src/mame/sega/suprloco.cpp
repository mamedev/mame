// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/******************************************************************************

Super Locomotive

driver by Zsolt Vasvari

TODO:
- Bit 5 in control_w is pulsed when loco turns "super". This is supposed
  to make red parts of sprites blink to purple, it's not clear how this is
  implemented in hardware, there's a hack to support it.

Sega PCB 834-5137
 Sega 315-5015 (Sega custom encrypted Z80)
 Sega 315-5011
 Sega 315-5012
 Z80
 M5L8255AP
 8 switch Dipswitch x 2

******************************************************************************/

#include "emu.h"
#include "suprloco.h"

#include "machine/segacrpt_device.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/sn76496.h"

#include "screen.h"
#include "speaker.h"


void suprloco_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc1ff).ram().share("spriteram");
	map(0xc200, 0xc7ff).nopw();
	map(0xc800, 0xc800).portr("SYSTEM");
	map(0xd000, 0xd000).portr("P1");
	map(0xd800, 0xd800).portr("P2");
	map(0xe000, 0xe000).portr("DSW1");
	map(0xe001, 0xe001).portr("DSW2");
	map(0xe800, 0xe803).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf000, 0xf6ff).ram().w(FUNC(suprloco_state::videoram_w)).share("videoram");
	map(0xf700, 0xf7df).ram(); /* unused */
	map(0xf7e0, 0xf7ff).ram().w(FUNC(suprloco_state::scrollram_w)).share("scrollram");
	map(0xf800, 0xffff).ram();
}

void suprloco_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share("decrypted_opcodes");
	map(0x8000, 0xbfff).rom().region("maincpu", 0x8000);
}

void suprloco_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa003).w("sn1", FUNC(sn76496_device::write));
	map(0xc000, 0xc003).w("sn2", FUNC(sn76496_device::write));
	map(0xe000, 0xe000).r("ppi", FUNC(i8255_device::acka_r));
}



static INPUT_PORTS_START( suprloco )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPSETTING(    0x03, "50000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )       PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Infinite Lives (Cheat)") PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Initial Entry" )     PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8 by 8 */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1024*8*8, 2*1024*8*8, 3*1024*8*8 },            /* plane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( gfx_suprloco )
	/* sprites use colors 256-511 + 512-767 */
	GFXDECODE_ENTRY( "gfx1", 0x6000, charlayout, 0, 16 )
GFXDECODE_END


void suprloco_state::suprloco(machine_config &config)
{
	/* basic machine hardware */
	sega_315_5015_device &maincpu(SEGA_315_5015(config, m_maincpu, 4000000));   /* 4 MHz (?) */
	maincpu.set_addrmap(AS_PROGRAM, &suprloco_state::main_map);
	maincpu.set_addrmap(AS_OPCODES, &suprloco_state::decrypted_opcodes_map);
	maincpu.set_vblank_int("screen", FUNC(suprloco_state::irq0_line_hold));
	maincpu.set_decrypted_tag(":decrypted_opcodes");

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &suprloco_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(suprloco_state::irq0_line_hold), attotime::from_hz(4*60));          /* NMIs are caused by the main CPU */

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pb_callback().set(FUNC(suprloco_state::control_w));
	ppi.tri_pb_callback().set_constant(0);
	ppi.out_pc_callback().set_output("lamp0").bit(0).invert(); // set by 8255 bit mode when no credits inserted
	ppi.out_pc_callback().append_inputline(m_audiocpu, INPUT_LINE_NMI).bit(7).invert();

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(5000));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(1*8, 31*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(suprloco_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_suprloco);
	PALETTE(config, "palette", FUNC(suprloco_state::suprloco_palette), 512+256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SN76496(config, "sn1", 4000000).add_route(ALL_OUTPUTS, "mono", 1.0);

	SN76496(config, "sn2", 2000000).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( suprloco )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5226a.37",    0x0000, 0x4000, CRC(33b02368) SHA1(c6e3116ad4b52bcc3174de5770f7a7ce024790d5) ) /* encrypted */
	ROM_LOAD( "epr-5227a.15",    0x4000, 0x4000, CRC(a5e67f50) SHA1(1dd52e4cf00ce414fe1db8259c9976cdc23513b4) ) /* encrypted */
	ROM_LOAD( "epr-5228.28",     0x8000, 0x4000, CRC(a597828a) SHA1(61004d112591fd2d752c39df71c1304d9308daae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-5222.64",     0x0000, 0x2000, CRC(0aa57207) SHA1(b29b533505cb5b47c90534f2f610baeb7265d030) )

	ROM_REGION( 0xe000, "gfx1", 0 )
	ROM_LOAD( "epr-5225.63",     0x0000, 0x2000, CRC(e571fe81) SHA1(ac2b5914a445b89b7456b2c4290e4630b525f05d) )
	ROM_LOAD( "epr-5224.62",     0x2000, 0x2000, CRC(6130f93c) SHA1(ae0657f46c10e75eec994e75359a89b5d61baf68) )
	ROM_LOAD( "epr-5223.61",     0x4000, 0x2000, CRC(3b03004e) SHA1(805b51cb14d3ace97f2e0f306db28921b2f5e322) )
							/* 0x6000- 0xe000 will be created by init_suprloco */

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data used at runtime */
	ROM_LOAD( "epr-5229.55",     0x0000, 0x4000, CRC(ee2d3ed3) SHA1(593f3cd5c4e7f20b5e31e6bac8864774442e4b75) )
	ROM_LOAD( "epr-5230.56",     0x4000, 0x2000, CRC(f04a4b50) SHA1(80363f0c508fb2a755bf684f9a6862c1e7285495) )
							/* 0x6000 empty */

	ROM_REGION( 0x0620, "proms", 0 )
	ROM_LOAD( "pr-5220.100",     0x0100, 0x0080, CRC(7b0c8ce5) SHA1(4e1ea5ce38198a3965dfeb609ba0c7e8211531c3) ) /* color PROM */
	ROM_CONTINUE(                0x0000, 0x0080 )
	ROM_CONTINUE(                0x0180, 0x0080 )
	ROM_CONTINUE(                0x0080, 0x0080 )
	ROM_LOAD( "pr-5219.89",      0x0200, 0x0400, CRC(1d4b02cb) SHA1(00d822f1bc4f57f2f5d5a0615241f8136246a842) ) /* 3bpp to 4bpp table */
	ROM_LOAD( "pr-5221.7",       0x0600, 0x0020, CRC(89ba674f) SHA1(17c87840c8011968675a5a6f55966467df02364b) ) /* unknown */
ROM_END

ROM_START( suprlocoo )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5226.37",     0x0000, 0x4000, CRC(57f514dd) SHA1(707800b90a22547a56b01d1e11775e9ee5555d23) ) /* encrypted */
	ROM_LOAD( "epr-5227.15",     0x4000, 0x4000, CRC(5a1d2fb0) SHA1(fdb9416e5530718245fd597073a63feddb233c3c) ) /* encrypted */
	ROM_LOAD( "epr-5228.28",     0x8000, 0x4000, CRC(a597828a) SHA1(61004d112591fd2d752c39df71c1304d9308daae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-5222.64",     0x0000, 0x2000, CRC(0aa57207) SHA1(b29b533505cb5b47c90534f2f610baeb7265d030) )

	ROM_REGION( 0xe000, "gfx1", 0 )
	ROM_LOAD( "epr-5225.63",     0x0000, 0x2000, CRC(e571fe81) SHA1(ac2b5914a445b89b7456b2c4290e4630b525f05d) )
	ROM_LOAD( "epr-5224.62",     0x2000, 0x2000, CRC(6130f93c) SHA1(ae0657f46c10e75eec994e75359a89b5d61baf68) )
	ROM_LOAD( "epr-5223.61",     0x4000, 0x2000, CRC(3b03004e) SHA1(805b51cb14d3ace97f2e0f306db28921b2f5e322) )
							/* 0x6000- 0xe000 will be created by init_suprloco */

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data used at runtime */
	ROM_LOAD( "epr-5229.55",     0x0000, 0x4000, CRC(ee2d3ed3) SHA1(593f3cd5c4e7f20b5e31e6bac8864774442e4b75) )
	ROM_LOAD( "epr-5230.56",     0x4000, 0x2000, CRC(f04a4b50) SHA1(80363f0c508fb2a755bf684f9a6862c1e7285495) )
							/* 0x6000 empty */

	ROM_REGION( 0x0620, "proms", 0 )
	ROM_LOAD( "pr-5220.100",     0x0100, 0x0080, CRC(7b0c8ce5) SHA1(4e1ea5ce38198a3965dfeb609ba0c7e8211531c3) ) /* color PROM */
	ROM_CONTINUE(                0x0000, 0x0080 )
	ROM_CONTINUE(                0x0180, 0x0080 )
	ROM_CONTINUE(                0x0080, 0x0080 )
	ROM_LOAD( "pr-5219.89",      0x0200, 0x0400, CRC(1d4b02cb) SHA1(00d822f1bc4f57f2f5d5a0615241f8136246a842) ) /* 3bpp to 4bpp table */
	ROM_LOAD( "pr-5221.7",       0x0600, 0x0020, CRC(89ba674f) SHA1(17c87840c8011968675a5a6f55966467df02364b) ) /* unknown */
ROM_END

void suprloco_state::init_suprloco()
{
	/* convert graphics to 4bpp from 3bpp */
	uint8_t *source = memregion("gfx1")->base();
	uint8_t *dest   = source + 0x6000;
	uint8_t *lookup = memregion("proms")->base() + 0x0200;

	for (int i = 0; i < 0x80; i++, lookup += 8)
	{
		for (int j = 0; j < 0x40; j++, source++, dest++)
		{
			dest[0] = dest[0x2000] = dest[0x4000] = dest[0x6000] = 0;

			for (int k = 0; k < 8; k++)
			{
				const int color_source = (((source[0x0000] >> k) & 0x01) << 2) |
										 (((source[0x2000] >> k) & 0x01) << 1) |
										 (((source[0x4000] >> k) & 0x01) << 0);

				const int color_dest = lookup[color_source];

				dest[0x0000] |= (((color_dest >> 3) & 0x01) << k);
				dest[0x2000] |= (((color_dest >> 2) & 0x01) << k);
				dest[0x4000] |= (((color_dest >> 1) & 0x01) << k);
				dest[0x6000] |= (((color_dest >> 0) & 0x01) << k);
			}
		}
	}

}

GAME( 1982, suprloco,         0, suprloco, suprloco, suprloco_state, init_suprloco, ROT0, "Sega", "Super Locomotive (Rev.A)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, suprlocoo, suprloco, suprloco, suprloco, suprloco_state, init_suprloco, ROT0, "Sega", "Super Locomotive",         MACHINE_SUPPORTS_SAVE )
