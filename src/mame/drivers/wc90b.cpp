// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*
World Cup 90 bootleg driver
---------------------------

Ernesto Corvi
(ernesto@imagina.com)

CPU #1 : Handles background & foreground tiles, controllers, dipswitches.
CPU #2 : Handles sprites and palette
CPU #3 : Audio. The audio chip is a YM2203. I need help with this!.

Memory Layout:

CPU #1
0000-8000 ROM
8000-9000 RAM
a000-a800 Color Ram for background #1 tiles
a800-b000 Video Ram for background #1 tiles
c000-c800 Color Ram for background #2 tiles
c800-c000 Video Ram for background #2 tiles
e000-e800 Color Ram for foreground tiles
e800-f000 Video Ram for foreground tiles
f800-fc00 Common Ram with CPU #2
fd00-fd00 Stick 1, Coin 1 & Start 1 input port
fd02-fd02 Stick 2, Coin 2 & Start 2 input port
fd06-fc06 Dip Switch A
fd08-fc08 Dip Switch B

CPU #2
0000-c000 ROM
c000-d000 RAM
d000-d800 RAM Sprite Ram
e000-e800 RAM Palette Ram
f800-fc00 Common Ram with CPU #1

CPU #3
0000-0xc000 ROM
???????????

Notes:
-----
The bootleg video hardware is quite different from the original machine.
I could not figure out the encoding of the scrolling for the new
video hardware. The memory positions, in case anyone wants to try, are
the following ( CPU #1 memory addresses ):
fd06: scroll bg #1 X coordinate
fd04: scroll bg #1 Y coordinate
fd08: scroll bg #2 X coordinate
fd0a: scroll bg #2 Y coordinate
fd0e: ????

What I used instead, was the local copy kept in RAM. These values
are the ones the original machine uses. This will differ when trying
to use some of this code to write a driver for a similar Tecmo bootleg.

Sprites are also very different. There's a code snippet in the ROM
that converts the original sprites to the new format, which only allows
16x16 sprites. That snippet also does some ( nasty ) clipping.

Colors are accurate. The graphics ROMs have been modified severely
and encoded in a different way from the original machine. Even if
sometimes it seems colors are not entirely correct, this is only due
to the crappy artwork of the person that did the bootleg.

Dip switches are not complete and they don't seem to differ from
the original machine.

Last but not least, the set of ROMs i have for Euro League seem to have
the sprites corrupted. The game seems to be exactly the same as the
World Cup 90 bootleg.

Noted added by ClawGrip 28-Mar-2008:
-----------------------------------
-Dumped and added the all the PCB GALs.
-Removed the second YM2203, Ernesto said it wasn't present on his board,
 and also isn't on mine.
-My PCB has a different ROM (a05.bin), but only two bytes are different.
 Dox suggested that it can be just a year or text mod, so I decided not
 to include my set. If anyone wants it, please mail me:
 clawgrip at hotmail dot com. I can't find any graphical difference
 between my set and the one already on MAME.

*/

#include "emu.h"
#include "includes/wc90b.h"

#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "screen.h"
#include "speaker.h"


#define TEST_DIPS false /* enable to test unmapped dip switches */

#define MASTER_CLOCK XTAL(14'318'181)/2
#define SOUND_CLOCK XTAL(20'000'000)/4
#define YM2203_CLOCK XTAL(20'000'000)/16
#define MSM5205_CLOCK XTAL(384'000)


WRITE8_MEMBER(wc90b_state::bankswitch_w)
{
	membank("mainbank")->set_entry(data >> 3);
}

WRITE8_MEMBER(wc90b_state::bankswitch1_w)
{
	membank("subbank")->set_entry(data >> 3);
}

WRITE8_MEMBER(wc90b_state::sound_command_w)
{
	m_soundlatch->write(space, offset, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

WRITE8_MEMBER(wc90b_state::adpcm_control_w)
{
	membank("audiobank")->set_entry(data & 0x01);
	m_msm->reset_w(data & 0x08);
}

WRITE8_MEMBER(wc90b_state::adpcm_data_w)
{
	m_msm5205next = data;
}

READ8_MEMBER(wc90b_state::master_irq_ack_r)
{
	m_maincpu->set_input_line(0,CLEAR_LINE);
	return 0xff;
}

WRITE8_MEMBER(wc90b_state::slave_irq_ack_w)
{
	m_subcpu->set_input_line(0,CLEAR_LINE);
}


void wc90b_state::wc90b_map1(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram(); /* Main RAM */
	map(0xa000, 0xafff).ram().w(FUNC(wc90b_state::fgvideoram_w)).share("fgvideoram");
	map(0xc000, 0xcfff).ram().w(FUNC(wc90b_state::bgvideoram_w)).share("bgvideoram");
	map(0xe000, 0xefff).ram().w(FUNC(wc90b_state::txvideoram_w)).share("txvideoram");
	map(0xf000, 0xf7ff).bankr("mainbank");
	map(0xf800, 0xfbff).ram().share("share1");
	map(0xfc00, 0xfc00).w(FUNC(wc90b_state::bankswitch_w));
	map(0xfd00, 0xfd00).w(FUNC(wc90b_state::sound_command_w));
	map(0xfd04, 0xfd04).writeonly().share("scroll1y");
	map(0xfd06, 0xfd06).writeonly().share("scroll1x");
	map(0xfd08, 0xfd08).writeonly().share("scroll2y");
	map(0xfd0a, 0xfd0a).writeonly().share("scroll2x");
	map(0xfd0e, 0xfd0e).writeonly().share("scroll_x_lo");
	map(0xfd00, 0xfd00).portr("P1");
	map(0xfd02, 0xfd02).portr("P2");
	map(0xfd06, 0xfd06).portr("DSW1");
	map(0xfd08, 0xfd08).portr("DSW2");
	map(0xfd0c, 0xfd0c).r(FUNC(wc90b_state::master_irq_ack_r));
}

void wc90b_state::wc90b_map2(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().share("spriteram");
	map(0xd800, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xe800, 0xefff).rom();
	map(0xf000, 0xf7ff).bankr("subbank");
	map(0xf800, 0xfbff).ram().share("share1");
	map(0xfc00, 0xfc00).w(FUNC(wc90b_state::bankswitch1_w));
	map(0xfd0c, 0xfd0c).w(FUNC(wc90b_state::slave_irq_ack_w));
}

void wc90b_state::sound_cpu(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("audiobank");
	map(0xe000, 0xe000).w(FUNC(wc90b_state::adpcm_control_w));
	map(0xe400, 0xe400).w(FUNC(wc90b_state::adpcm_data_w));
	map(0xe800, 0xe801).rw("ymsnd1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xec00, 0xec01).rw("ymsnd2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}



static INPUT_PORTS_START( wc90b )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Countdown Speed" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )           // 60/60
	PORT_DIPSETTING(    0x00, "Fast" )                      // 56/60
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "1 Player Game Time" )
	PORT_DIPSETTING(    0x01, "1:00" )
	PORT_DIPSETTING(    0x02, "1:30" )
	PORT_DIPSETTING(    0x03, "2:00" )
	PORT_DIPSETTING(    0x00, "2:30" )
	PORT_DIPNAME( 0x1c, 0x1c, "2 Player Game Time" )
	PORT_DIPSETTING(    0x0c, "1:00" )
	PORT_DIPSETTING(    0x14, "1:30" )
	PORT_DIPSETTING(    0x04, "2:00" )
	PORT_DIPSETTING(    0x18, "2:30" )
	PORT_DIPSETTING(    0x1c, "3:00" )
	PORT_DIPSETTING(    0x08, "3:30" )
	PORT_DIPSETTING(    0x10, "4:00" )
	PORT_DIPSETTING(    0x00, "5:00" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	2048,   /* 2048 characters */
	4,  /* 4 bits per pixel */
	{ 0, 0x4000*8, 0x8000*8, 0xc000*8 },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 characters */
	256,    /* 256 characters */
	4,  /* 4 bits per pixel */
	{ 0*0x20000*8, 1*0x20000*8, 2*0x20000*8, 3*0x20000*8 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		(0x1000*8)+0, (0x1000*8)+1, (0x1000*8)+2, (0x1000*8)+3, (0x1000*8)+4, (0x1000*8)+5, (0x1000*8)+6, (0x1000*8)+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		0x800*8, 0x800*8+1*8, 0x800*8+2*8, 0x800*8+3*8, 0x800*8+4*8, 0x800*8+5*8, 0x800*8+6*8, 0x800*8+7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 32*32 characters */
	4096,   /* 1024 characters */
	4,  /* 4 bits per pixel */
	{ 3*0x20000*8, 2*0x20000*8, 1*0x20000*8, 0*0x20000*8 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		(16*8)+0, (16*8)+1, (16*8)+2, (16*8)+3, (16*8)+4, (16*8)+5, (16*8)+6, (16*8)+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 8*8+1*8, 8*8+2*8, 8*8+3*8, 8*8+4*8, 8*8+5*8, 8*8+6*8, 8*8+7*8 },
	32*8    /* every char takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_wc90b )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,       0x100, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, tilelayout,       0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x02000, tilelayout,       0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, tilelayout,       0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x06000, tilelayout,       0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, tilelayout,       0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x0a000, tilelayout,       0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x0c000, tilelayout,       0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x0e000, tilelayout,       0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x10000, tilelayout,       0x300, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x12000, tilelayout,       0x300, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x14000, tilelayout,       0x300, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x16000, tilelayout,       0x300, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x18000, tilelayout,       0x300, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x1a000, tilelayout,       0x300, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x1c000, tilelayout,       0x300, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x1e000, tilelayout,       0x300, 0x10 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, spritelayout,     0x000, 0x10 ) // sprites
GFXDECODE_END



WRITE_LINE_MEMBER(wc90b_state::adpcm_int)
{
	m_toggle ^= 1;
	if(m_toggle)
	{
		m_msm->write_data((m_msm5205next & 0xf0) >> 4);
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
	else
		m_msm->write_data((m_msm5205next & 0x0f) >> 0);
}

void wc90b_state::machine_start()
{
	membank("mainbank")->configure_entries(0, 32, memregion("maincpu")->base() + 0x10000, 0x800);
	membank("subbank")->configure_entries(0, 32, memregion("sub")->base() + 0x10000, 0x800);
	membank("audiobank")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x8000, 0x4000);

	save_item(NAME(m_msm5205next));
	save_item(NAME(m_toggle));
}


void wc90b_state::wc90b(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &wc90b_state::wc90b_map1);
	m_maincpu->set_vblank_int("screen", FUNC(wc90b_state::irq0_line_assert));

	Z80(config, m_subcpu, MASTER_CLOCK);
	m_subcpu->set_addrmap(AS_PROGRAM, &wc90b_state::wc90b_map2);
	m_subcpu->set_vblank_int("screen", FUNC(wc90b_state::irq0_line_assert));

	Z80(config, m_audiocpu, SOUND_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &wc90b_state::sound_cpu);
	/* IRQs are triggered by the main CPU */

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(wc90b_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wc90b);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024).set_endianness(ENDIANNESS_BIG);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2203(config, "ymsnd1", YM2203_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.40);
	YM2203(config, "ymsnd2", YM2203_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.40);

	MSM5205(config, m_msm, MSM5205_CLOCK);
	m_msm->vck_legacy_callback().set(FUNC(wc90b_state::adpcm_int));	/* interrupt function */
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);	/* 4KHz 4-bit */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.20);
}

ROM_START( twcup90b1 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "a02.bin",      0x00000, 0x10000, CRC(192a03dd) SHA1(ab98d370bba5437f956631b0199b173be55f1c27) )  /* c000-ffff is not used */
	ROM_LOAD( "a03.bin",      0x10000, 0x10000, CRC(f54ff17a) SHA1(a19850fc28a5a0da20795a5cc6b56d9c16554bce) )  /* banked at f000-f7ff */

	ROM_REGION( 0x20000, "sub", 0 )  /* Second CPU */
	ROM_LOAD( "a04.bin",      0x00000, 0x10000, CRC(3d535e2f) SHA1(f1e1878b5a8316e770c74a1e1f29a7a81a4e5dfe) )  /* c000-ffff is not used */
	ROM_LOAD( "a05.bin",      0x10000, 0x10000, CRC(9e421c4b) SHA1(e23a1f1d5d1e960696f45df653869712eb889839) )  /* banked at f000-f7ff */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a01.bin",      0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )

	ROM_REGION( 0x010000, "gfx1", 0 )
	ROM_LOAD( "a06.bin",      0x000000, 0x04000, CRC(3b5387b7) SHA1(b839b4eafe8bf6f9e841e19fee1bdb64a66f3448) )
	ROM_LOAD( "a08.bin",      0x004000, 0x04000, CRC(c622a5a3) SHA1(468c8c24af1f6f244228b66df04cb0ea81c1875e) )
	ROM_LOAD( "a10.bin",      0x008000, 0x04000, CRC(0923d9f6) SHA1(4b10ee3fc17bb63cda51b2a978d066b6a140a551) )
	ROM_LOAD( "a20.bin",      0x00c000, 0x04000, CRC(b8dec83e) SHA1(fe617ddccdd0dbd05ca09a1507074aa14b529322) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "a07.bin",      0x000000, 0x20000, CRC(38c31817) SHA1(cb24ed8702d62066366924c033c07ffc78bd1fad) )
	ROM_LOAD( "a09.bin",      0x020000, 0x20000, CRC(32e39e29) SHA1(44f22ed6c983541c7fea5857ba0456aaa87b36d1) )
	ROM_LOAD( "a11.bin",      0x040000, 0x20000, CRC(5ccec796) SHA1(2cc191a4267819eb31962726e2ed4567c825c39e) )
	ROM_LOAD( "a21.bin",      0x060000, 0x20000, CRC(0c54a091) SHA1(3eecb285b5a7bbc310c87492516d7ffb2841aa3b) )

	ROM_REGION( 0x080000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "146_a12.bin",  0x000000, 0x10000, CRC(d5a60096) SHA1(a8e351a4b020b4fc2b2cb7d3f0fdfb43fc44d7d9) )
	ROM_LOAD( "147_a13.bin",  0x010000, 0x10000, CRC(36bbf467) SHA1(627b5847ffb098c92edfd58c25391799f3b209e0) )
	ROM_LOAD( "148_a14.bin",  0x020000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "149_a15.bin",  0x030000, 0x10000, CRC(75aa9b86) SHA1(0c221bd2e8a5472bb0e515f27fb72b0c8e8c0ca4) )
	ROM_LOAD( "150_a16.bin",  0x040000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "151_a17.bin",  0x050000, 0x10000, CRC(228429d8) SHA1(3b2dbea53807929c24d593c469a83172f7747f66) )
	ROM_LOAD( "152_a18.bin",  0x060000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )
	ROM_LOAD( "153_a19.bin",  0x070000, 0x10000, CRC(f36390a9) SHA1(e5ea36e91b3ced068281524ee79d0432f489715c) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "el_ic39_gal16v8_0.bin", 0x0000, 0x0117, NO_DUMP SHA1(894b345b395097acf6cf52ab8bc922099f97a85f) )
	ROM_LOAD( "el_ic44_gal16v8_1.bin", 0x0200, 0x0117, NO_DUMP SHA1(fd41f55d857995fe87217dd9679c42760c241dc4) )
	ROM_LOAD( "el_ic54_gal16v8_2.bin", 0x0400, 0x0117, NO_DUMP SHA1(f6d138fe42549219e11ee8524b05fe3c2b43f5d3) )
	ROM_LOAD( "el_ic100_gal16v8_3.bin", 0x0600, 0x0117, NO_DUMP SHA1(515fcdf378e75ed078f54439fefce8807403bdd5) )
	ROM_LOAD( "el_ic143_gal16v8_4.bin", 0x0800, 0x0117, NO_DUMP SHA1(fbe632437eac2418da7a3c3e947cfd36f6211407) )
ROM_END

ROM_START( twcup90b2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "a02",          0x00000, 0x10000, CRC(1e6e94c9) SHA1(1731e3e3b5d17ba676a7e42638d7206212a0080d) )  /* c000-ffff is not used */
	ROM_LOAD( "a03.bin",      0x10000, 0x10000, CRC(f54ff17a) SHA1(a19850fc28a5a0da20795a5cc6b56d9c16554bce) )  /* banked at f000-f7ff */

	ROM_REGION( 0x20000, "sub", 0 )  /* Second CPU */
	ROM_LOAD( "a04.bin",      0x00000, 0x10000, CRC(3d535e2f) SHA1(f1e1878b5a8316e770c74a1e1f29a7a81a4e5dfe) )  /* c000-ffff is not used */
	ROM_LOAD( "a05.bin",      0x10000, 0x10000, CRC(9e421c4b) SHA1(e23a1f1d5d1e960696f45df653869712eb889839) )  /* banked at f000-f7ff */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a01.bin",      0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )

	ROM_REGION( 0x010000, "gfx1", 0 )
	ROM_LOAD( "a06",       0x000000, 0x04000, CRC(0c054481) SHA1(eebab099a4db5fbf13522ecd67bfa741e16e40d4) )
	ROM_CONTINUE ( 0x000000, 0x04000)
	ROM_LOAD( "a08",       0x004000, 0x04000, CRC(ebb3eb48) SHA1(9cb133e02004bc04a9d7016b8cf5f6865e3ccf26) )
	ROM_CONTINUE ( 0x004000, 0x04000)
	ROM_LOAD( "a10",       0x008000, 0x04000, CRC(c0232af8) SHA1(5bbab00403a47feae153e179c04212021036b8a7) )
	ROM_CONTINUE ( 0x008000, 0x04000)
	ROM_LOAD( "a20",       0x00c000, 0x04000, CRC(a36e17fb) SHA1(45e4df4b4a22658f6dad21853e87fae734698fbd) )
	ROM_CONTINUE ( 0x00c000, 0x04000)

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "a07.bin",      0x000000, 0x20000, CRC(38c31817) SHA1(cb24ed8702d62066366924c033c07ffc78bd1fad) )
	ROM_LOAD( "a09.bin",      0x020000, 0x20000, CRC(32e39e29) SHA1(44f22ed6c983541c7fea5857ba0456aaa87b36d1) )
	ROM_LOAD( "a11.bin",      0x040000, 0x20000, CRC(5ccec796) SHA1(2cc191a4267819eb31962726e2ed4567c825c39e) )
	ROM_LOAD( "a21.bin",      0x060000, 0x20000, CRC(0c54a091) SHA1(3eecb285b5a7bbc310c87492516d7ffb2841aa3b) )

	ROM_REGION( 0x080000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "146_a12.bin",  0x000000, 0x10000, CRC(d5a60096) SHA1(a8e351a4b020b4fc2b2cb7d3f0fdfb43fc44d7d9) )
	ROM_LOAD( "147_a13",      0x010000, 0x10000, CRC(5b16fd48) SHA1(b167d6a7da0c696cde39581822fc61d20756321c) )
	ROM_LOAD( "148_a14.bin",  0x020000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "149_a15",      0x030000, 0x10000, CRC(b2423962) SHA1(098bc06411cf3f9c7cf69933eba360fd059b5d3f) )
	ROM_LOAD( "150_a16.bin",  0x040000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "151_a17",      0x050000, 0x10000, CRC(af98778e) SHA1(5bbce33a4cec5a234ed78e30899a4a166d71447a) )
	ROM_LOAD( "152_a18.bin",  0x060000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )
	ROM_LOAD( "153_a19",      0x070000, 0x10000, CRC(8caa2745) SHA1(41efb92c98e063f5ed5fb0e68fa014f89da00cda) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "el_ic39_gal16v8_0.bin", 0x0000, 0x0117, NO_DUMP SHA1(894b345b395097acf6cf52ab8bc922099f97a85f) )
	ROM_LOAD( "el_ic44_gal16v8_1.bin", 0x0200, 0x0117, NO_DUMP SHA1(fd41f55d857995fe87217dd9679c42760c241dc4) )
	ROM_LOAD( "el_ic54_gal16v8_2.bin", 0x0400, 0x0117, NO_DUMP SHA1(f6d138fe42549219e11ee8524b05fe3c2b43f5d3) )
	ROM_LOAD( "el_ic100_gal16v8_3.bin", 0x0600, 0x0117, NO_DUMP SHA1(515fcdf378e75ed078f54439fefce8807403bdd5) )
	ROM_LOAD( "el_ic143_gal16v8_4.bin", 0x0800, 0x0117, NO_DUMP SHA1(fbe632437eac2418da7a3c3e947cfd36f6211407) )
ROM_END


/* Different bootleg set with only one new ROM, a05 (added as "el_ic98_27c512_05.bin"), probably just a minor text mod from the supported set
(only two bytes differs), although I cannot find the difference:
   Comparing files a05.bin and el_ic98_27c512_05.bin
    00000590: 0F 0B
    00000591: FF FA
*/
ROM_START( twcup90ba )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "a02.bin",      0x00000, 0x10000, CRC(192a03dd) SHA1(ab98d370bba5437f956631b0199b173be55f1c27) )  /* c000-ffff is not used */
	ROM_LOAD( "a03.bin",      0x10000, 0x10000, CRC(f54ff17a) SHA1(a19850fc28a5a0da20795a5cc6b56d9c16554bce) )  /* banked at f000-f7ff */

	ROM_REGION( 0x20000, "sub", 0 )  /* Second CPU */
	ROM_LOAD( "a04.bin",              0x00000, 0x10000, CRC(3d535e2f) SHA1(f1e1878b5a8316e770c74a1e1f29a7a81a4e5dfe) )  /* c000-ffff is not used */
	ROM_LOAD( "el_ic98_27c512_05.bin",0x10000, 0x10000, CRC(c70d8c13) SHA1(365718725ea7d0355c68ba703b7f9624cb1134bc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a01.bin",      0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )

	ROM_REGION( 0x010000, "gfx1", 0 )
	ROM_LOAD( "a06.bin",      0x000000, 0x04000, CRC(3b5387b7) SHA1(b839b4eafe8bf6f9e841e19fee1bdb64a66f3448) )
	ROM_LOAD( "a08.bin",      0x004000, 0x04000, CRC(c622a5a3) SHA1(468c8c24af1f6f244228b66df04cb0ea81c1875e) )
	ROM_LOAD( "a10.bin",      0x008000, 0x04000, CRC(0923d9f6) SHA1(4b10ee3fc17bb63cda51b2a978d066b6a140a551) )
	ROM_LOAD( "a20.bin",      0x00c000, 0x04000, CRC(b8dec83e) SHA1(fe617ddccdd0dbd05ca09a1507074aa14b529322) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "a07.bin",      0x000000, 0x20000, CRC(38c31817) SHA1(cb24ed8702d62066366924c033c07ffc78bd1fad) )
	ROM_LOAD( "a09.bin",      0x020000, 0x20000, CRC(32e39e29) SHA1(44f22ed6c983541c7fea5857ba0456aaa87b36d1) )
	ROM_LOAD( "a11.bin",      0x040000, 0x20000, CRC(5ccec796) SHA1(2cc191a4267819eb31962726e2ed4567c825c39e) )
	ROM_LOAD( "a21.bin",      0x060000, 0x20000, CRC(0c54a091) SHA1(3eecb285b5a7bbc310c87492516d7ffb2841aa3b) )

	ROM_REGION( 0x080000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "146_a12.bin",  0x000000, 0x10000, CRC(d5a60096) SHA1(a8e351a4b020b4fc2b2cb7d3f0fdfb43fc44d7d9) )
	ROM_LOAD( "147_a13.bin",  0x010000, 0x10000, CRC(36bbf467) SHA1(627b5847ffb098c92edfd58c25391799f3b209e0) )
	ROM_LOAD( "148_a14.bin",  0x020000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "149_a15.bin",  0x030000, 0x10000, CRC(75aa9b86) SHA1(0c221bd2e8a5472bb0e515f27fb72b0c8e8c0ca4) )
	ROM_LOAD( "150_a16.bin",  0x040000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "151_a17.bin",  0x050000, 0x10000, CRC(228429d8) SHA1(3b2dbea53807929c24d593c469a83172f7747f66) )
	ROM_LOAD( "152_a18.bin",  0x060000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )
	ROM_LOAD( "153_a19.bin",  0x070000, 0x10000, CRC(f36390a9) SHA1(e5ea36e91b3ced068281524ee79d0432f489715c) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "el_ic39_gal16v8_0.bin", 0x0000, 0x0117, NO_DUMP SHA1(894b345b395097acf6cf52ab8bc922099f97a85f) )
	ROM_LOAD( "el_ic44_gal16v8_1.bin", 0x0200, 0x0117, NO_DUMP SHA1(fd41f55d857995fe87217dd9679c42760c241dc4) )
	ROM_LOAD( "el_ic54_gal16v8_2.bin", 0x0400, 0x0117, NO_DUMP SHA1(f6d138fe42549219e11ee8524b05fe3c2b43f5d3) )
	ROM_LOAD( "el_ic100_gal16v8_3.bin", 0x0600, 0x0117, NO_DUMP SHA1(515fcdf378e75ed078f54439fefce8807403bdd5) )
	ROM_LOAD( "el_ic143_gal16v8_4.bin", 0x0800, 0x0117, NO_DUMP SHA1(fbe632437eac2418da7a3c3e947cfd36f6211407) )
ROM_END


/*
  World Cup '90
  Hack with european teams, like 'Euro League'
  Board found in Argentina.
*/
ROM_START( twcup90bb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "27c512.02",    0x00000, 0x10000, CRC(192a03dd) SHA1(ab98d370bba5437f956631b0199b173be55f1c27) )
	ROM_LOAD( "27c512.03",    0x10000, 0x10000, CRC(f54ff17a) SHA1(a19850fc28a5a0da20795a5cc6b56d9c16554bce) )

	ROM_REGION( 0x20000, "sub", 0 )  /* Second CPU */
	ROM_LOAD( "27c512.04",    0x00000, 0x10000, CRC(3d535e2f) SHA1(f1e1878b5a8316e770c74a1e1f29a7a81a4e5dfe) )
	ROM_LOAD( "27c512.05",    0x10000, 0x10000, CRC(9e421c4b) SHA1(e23a1f1d5d1e960696f45df653869712eb889839) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "27c512.bin",   0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )

	ROM_REGION( 0x010000, "gfx1", 0 )
	ROM_LOAD( "27c256.06",    0x000000, 0x04000, CRC(0c054481) SHA1(eebab099a4db5fbf13522ecd67bfa741e16e40d4) )
	ROM_CONTINUE (            0x000000, 0x04000)
	ROM_LOAD( "27256.08",     0x004000, 0x04000, CRC(ebb3eb48) SHA1(9cb133e02004bc04a9d7016b8cf5f6865e3ccf26) )
	ROM_CONTINUE (            0x004000, 0x04000)
	ROM_LOAD( "27128.10",     0x008000, 0x04000, CRC(0923d9f6) SHA1(4b10ee3fc17bb63cda51b2a978d066b6a140a551) )
	ROM_LOAD( "27128k.20",    0x00c000, 0x04000, CRC(b8dec83e) SHA1(fe617ddccdd0dbd05ca09a1507074aa14b529322) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "ds40986_27c010.07",  0x000000, 0x20000, CRC(38c31817) SHA1(cb24ed8702d62066366924c033c07ffc78bd1fad) )
	ROM_LOAD( "ds40986_27c010.09",  0x020000, 0x20000, CRC(32e39e29) SHA1(44f22ed6c983541c7fea5857ba0456aaa87b36d1) )
	ROM_LOAD( "ds40986_27c010.11",  0x040000, 0x20000, CRC(5ccec796) SHA1(2cc191a4267819eb31962726e2ed4567c825c39e) )
	ROM_LOAD( "ds40986_27c010.21",  0x060000, 0x20000, CRC(0c54a091) SHA1(3eecb285b5a7bbc310c87492516d7ffb2841aa3b) )

	ROM_REGION( 0x080000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "27c512.12",  0x000000, 0x10000, CRC(6a828204) SHA1(0d8e90ee069fe16db3869cbc47991511244e1b34) )
	ROM_LOAD( "27c512.13",  0x010000, 0x10000, CRC(4706bad2) SHA1(f79460f094454b544b2637ff09bc41c9e107c764) )
	ROM_LOAD( "27c512.14",  0x020000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "27c512.15",  0x030000, 0x10000, CRC(77700f2d) SHA1(a39987f8ac1bb26d5aa0ae8cfe67fac823a0d1af) )
	ROM_LOAD( "27c512.16",  0x040000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "27c512.17",  0x050000, 0x10000, CRC(c387c804) SHA1(519a63c337d443f0876fcd44b88ed508b999912f) )
	ROM_LOAD( "27c512.18",  0x060000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )
	ROM_LOAD( "27c512.19",  0x070000, 0x10000, CRC(f9df54f6) SHA1(cee8da5d8e4959e5546b2f7dcc740e98bedda07a) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "el_ic39_gal16v8_0.bin", 0x0000, 0x0117, NO_DUMP SHA1(894b345b395097acf6cf52ab8bc922099f97a85f) )  // from another set
	ROM_LOAD( "el_ic44_gal16v8_1.bin", 0x0200, 0x0117, NO_DUMP SHA1(fd41f55d857995fe87217dd9679c42760c241dc4) )  // from another set
	ROM_LOAD( "el_ic54_gal16v8_2.bin", 0x0400, 0x0117, NO_DUMP SHA1(f6d138fe42549219e11ee8524b05fe3c2b43f5d3) )  // from another set
	ROM_LOAD( "el_ic100_gal16v8_3.bin", 0x0600, 0x0117, NO_DUMP SHA1(515fcdf378e75ed078f54439fefce8807403bdd5) )  // from another set
	ROM_LOAD( "el_ic143_gal16v8_4.bin", 0x0800, 0x0117, NO_DUMP SHA1(fbe632437eac2418da7a3c3e947cfd36f6211407) )  // from another set
ROM_END


GAME( 1989, twcup90b1, twcup90, wc90b, wc90b, wc90b_state, empty_init, ROT0, "bootleg", "Euro League (Italian hack of Tecmo World Cup '90)",               MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90b2, twcup90, wc90b, wc90b, wc90b_state, empty_init, ROT0, "bootleg", "Worldcup '90 (hack)",                                             MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90ba, twcup90, wc90b, wc90b, wc90b_state, empty_init, ROT0, "bootleg", "Euro League (Italian hack of Tecmo World Cup '90 - alt version)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90bb, twcup90, wc90b, wc90b, wc90b_state, empty_init, ROT0, "bootleg", "World Cup '90 (european hack, different title)",                  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
