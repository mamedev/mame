// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

Space Force Memory Map

driver by Zsolt Vasvari


0000-3fff   R   ROM
4000-43ff   R/W RAM
7000-7002   R   input ports 0-2
7000          W sound command
7001          W sound CPU IRQ trigger on bit 3 falling edge
7002          W unknown
7008          W unknown
7009          W unknown
700a          W unknown
700b          W flip screen
700c          W unknown
700d          W unknown
700e          W main CPU interrupt enable (it uses RST7.5)
700f          W unknown
8000-83ff   R/W bit 0-7 of character code
9000-93ff   R/W attributes RAM
                bit 0   - bit 8 of character code
                bit 1-3 - unused
                bit 4-6 - color
                bit 7   - unused
a000-a3ff   R/W X/Y scroll position of each character (can be scrolled up
                to 7 pixels in each direction)

TODO:
- fix cliprect for sprites that goes out of screen if possible

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "includes/spcforce.h"

void spcforce_state::machine_start()
{
	save_item(NAME(m_sn76496_latch));
	save_item(NAME(m_sn76496_select));
	save_item(NAME(m_sn1_ready));
	save_item(NAME(m_sn2_ready));
	save_item(NAME(m_sn3_ready));
	save_item(NAME(m_irq_mask));
}

WRITE8_MEMBER(spcforce_state::SN76496_latch_w)
{
	m_sn76496_latch = data;
}

WRITE_LINE_MEMBER(spcforce_state::write_sn1_ready)
{
	m_sn1_ready = state;
}

WRITE_LINE_MEMBER(spcforce_state::write_sn2_ready)
{
	m_sn2_ready = state;
}

WRITE_LINE_MEMBER(spcforce_state::write_sn3_ready)
{
	m_sn3_ready = state;
}

READ8_MEMBER(spcforce_state::SN76496_select_r)
{
	if (~m_sn76496_select & 0x40) return m_sn1_ready;
	if (~m_sn76496_select & 0x20) return m_sn2_ready;
	if (~m_sn76496_select & 0x10) return m_sn3_ready;

	return 0;
}

WRITE8_MEMBER(spcforce_state::SN76496_select_w)
{
	m_sn76496_select = data;

	if (~data & 0x40) m_sn1->write(space, 0, m_sn76496_latch);
	if (~data & 0x20) m_sn2->write(space, 0, m_sn76496_latch);
	if (~data & 0x10) m_sn3->write(space, 0, m_sn76496_latch);
}

READ8_MEMBER(spcforce_state::t0_r)
{
	/* SN76496 status according to Al - not supported by MAME?? */
	return machine().rand() & 1;
}


WRITE8_MEMBER(spcforce_state::soundtrigger_w)
{
	m_audiocpu->set_input_line(0, (~data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(spcforce_state::irq_mask_w)
{
	m_irq_mask = data & 1;
}

static ADDRESS_MAP_START( spcforce_map, AS_PROGRAM, 8, spcforce_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x7000, 0x7000) AM_READ_PORT("DSW") AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x7001, 0x7001) AM_READ_PORT("P1") AM_WRITE(soundtrigger_w)
	AM_RANGE(0x7002, 0x7002) AM_READ_PORT("P2")
	AM_RANGE(0x700b, 0x700b) AM_WRITE(flip_screen_w)
	AM_RANGE(0x700e, 0x700e) AM_WRITE(irq_mask_w)
	AM_RANGE(0x700f, 0x700f) AM_WRITENOP
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa3ff) AM_RAM AM_SHARE("scrollram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( meteors_map, AS_PROGRAM, 8, spcforce_state )
	AM_RANGE(0x700b, 0x700b) AM_WRITENOP
	AM_RANGE(0x700d, 0x700d) AM_WRITE(irq_mask_w) // ??
	AM_RANGE(0x700e, 0x700e) AM_WRITE(flip_screen_w) // irq mask isn't here, gets written too early causing the game to not boot, see startup code between sets
	AM_IMPORT_FROM(spcforce_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START( spcforce_sound_map, AS_PROGRAM, 8, spcforce_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( spcforce_sound_io_map, AS_IO, 8, spcforce_state )
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READ(soundlatch_byte_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(SN76496_latch_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(SN76496_select_r, SN76496_select_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(t0_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( spcforce )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )  /* probably unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("P1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_START("P2")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
INPUT_PORTS_END

/* same as spcforce, but no cocktail mode */
static INPUT_PORTS_START( spcforc2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )  /* probably unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )  /* probably unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_START("P2")
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	512,    /* 512 characters */
	3,      /* 3 bits per pixel */
	{ 2*512*8*8, 512*8*8, 0 },  /* The bitplanes are separate */
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8     /* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( spcforce )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 8 )
GFXDECODE_END


/* 1-bit RGB palette */
static const int colortable_source[] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 0, 1, 2, 3, 4, 5, 6,  /* not sure about these, but they are only used */
	0, 7, 0, 1, 2, 3, 4, 5,  /* to change the text color. During the game,   */
	0, 6, 7, 0, 1, 2, 3, 4,  /* only color 0 is used, which is correct.      */
	0, 5, 6, 7, 0, 1, 2, 3,
	0, 4, 5, 6, 7, 0, 1, 2,
	0, 3, 4, 5, 6, 7, 0, 1,
	0, 2, 3, 4, 5, 6, 7, 0
};

PALETTE_INIT_MEMBER(spcforce_state, spcforce)
{
	int i;

	for (i = 0; i < ARRAY_LENGTH(colortable_source); i++)
	{
		int data = colortable_source[i];
		rgb_t color = rgb_t(pal1bit(data >> 0), pal1bit(data >> 1), pal1bit(data >> 2));

		palette.set_pen_color(i, color);
	}
}


INTERRUPT_GEN_MEMBER(spcforce_state::vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(3, HOLD_LINE);
}

static MACHINE_CONFIG_START( spcforce, spcforce_state )

	/* basic machine hardware */
	/* FIXME: The 8085A had a max clock of 6MHz, internally divided by 2! */
	MCFG_CPU_ADD("maincpu", I8085A, 8000000 * 2)        /* 4.00 MHz??? */
	MCFG_CPU_PROGRAM_MAP(spcforce_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spcforce_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", I8035, 6144000)        /* divisor ??? */
	MCFG_CPU_PROGRAM_MAP(spcforce_sound_map)
	MCFG_CPU_IO_MAP(spcforce_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(spcforce_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", spcforce)
	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(colortable_source))
	MCFG_PALETTE_INIT_OWNER(spcforce_state, spcforce)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76496, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_SN76496_READY_HANDLER(WRITELINE(spcforce_state, write_sn1_ready))

	MCFG_SOUND_ADD("sn2", SN76496, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_SN76496_READY_HANDLER(WRITELINE(spcforce_state, write_sn2_ready))

	MCFG_SOUND_ADD("sn3", SN76496, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_SN76496_READY_HANDLER(WRITELINE(spcforce_state, write_sn3_ready))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( meteors, spcforce )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(meteors_map)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/
ROM_START( spcforce )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m1v4f.1a",     0x0000, 0x0800, CRC(7da0d1ed) SHA1(2ee145f590da557be057f181b4861014627872e7) )
	ROM_LOAD( "m2v4f.1c",     0x0800, 0x0800, CRC(25605bff) SHA1(afda2884a00fdbc000191dd548fd8e34df3e2f49) )
	ROM_LOAD( "m3v5f.2a",     0x1000, 0x0800, CRC(6f879366) SHA1(ef624619dbaad1f2adf4fab82e04bac117dbfac6) )
	ROM_LOAD( "m4v5f.2c",     0x1800, 0x0800, CRC(7fbfabfa) SHA1(0d6bbdcc80e251aa0ebd12e66549afaf6d8ccb0e) )
							/*0x2000 empty */
	ROM_LOAD( "m6v4f.3c",     0x2800, 0x0800, CRC(12128e9e) SHA1(b2a113b419e11ca094f56ae93870df11690b119a) )
	ROM_LOAD( "m7v4f.4a",     0x3000, 0x0800, CRC(978ad452) SHA1(fa84dcc6587403dd939da719a747d8c7332ed038) )
	ROM_LOAD( "m8v4f.4c",     0x3800, 0x0800, CRC(f805c3cd) SHA1(78eb13b99aae895742b34ed56bee9313d3643de1) )

	ROM_REGION( 0x1000, "audiocpu", 0 )     /* sound MCU */
	ROM_LOAD( "vm5.k10",      0x0000, 0x0800, CRC(8820913c) SHA1(90002cafdf5f32f916e5457e013ebe53405d5ca8) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rm1v2.6s",     0x0000, 0x0800, CRC(8e3490d7) SHA1(a5e47f953bb833c2bb769b266fff60f7a20c69a6) )
	ROM_LOAD( "rm2v1.7s",     0x0800, 0x0800, CRC(fbbfa05a) SHA1(c737b216f47e14c069cb84b5dbcc5a79fcc13648) )
	ROM_LOAD( "gm1v2.6p",     0x1000, 0x0800, CRC(4f574920) SHA1(05930a8ea5c6e05d01d1b4faabb3305aab44125c) )
	ROM_LOAD( "gm2v1.7p",     0x1800, 0x0800, CRC(0cd89ce2) SHA1(adb101400eb00119930494e99629948248d99d2f) )
	ROM_LOAD( "bm1v2.6m",     0x2000, 0x0800, CRC(130869ce) SHA1(588d6c9403d5fd966266b4f0333ee47b36c8b1d8) )
	ROM_LOAD( "bm2v1.7m",     0x2800, 0x0800, CRC(472f0a9b) SHA1(a8a9e2aa62374cd3bd938b5cb5fb20face3114c3) )
ROM_END

ROM_START( spcforc2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spacefor.1a",  0x0000, 0x0800, CRC(ef6fdccb) SHA1(2fff28437597958b39a821f93ac30f32c24f50aa) )
	ROM_LOAD( "spacefor.1c",  0x0800, 0x0800, CRC(44bd1cdd) SHA1(6dd5ae7a64079c61b63667f06e0d34dec48eac7c) )
	ROM_LOAD( "spacefor.2a",  0x1000, 0x0800, CRC(fcbc7df7) SHA1(b6e89dbfc80d5d9dcf889f618a8278c182773a14) )
	ROM_LOAD( "vm4",          0x1800, 0x0800, CRC(c5b073b9) SHA1(93b77c77488aa954c35880439be6c7629448a3ea) )
							/*0x2000 empty */
	ROM_LOAD( "spacefor.3c",  0x2800, 0x0800, CRC(9fd52301) SHA1(1ea5d5b888dd2f7ac6aab227c78b86c2f2f320da) )
	ROM_LOAD( "spacefor.4a",  0x3000, 0x0800, CRC(89aefc0a) SHA1(0b56efa613bce972af4bbf145853bfc0cda60ef9) )
	ROM_LOAD( "m8v4f.4c",     0x3800, 0x0800, CRC(f805c3cd) SHA1(78eb13b99aae895742b34ed56bee9313d3643de1) )

	ROM_REGION( 0x1000, "audiocpu", 0 )     /* sound MCU */
	ROM_LOAD( "vm5.k10",      0x0000, 0x0800, CRC(8820913c) SHA1(90002cafdf5f32f916e5457e013ebe53405d5ca8) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "spacefor.6s",  0x0000, 0x0800, CRC(848ae522) SHA1(deb28ba09556d04d9f6c906a163372f842b00c63) )
	ROM_LOAD( "rm2v1.7s",     0x0800, 0x0800, CRC(fbbfa05a) SHA1(c737b216f47e14c069cb84b5dbcc5a79fcc13648) )
	ROM_LOAD( "spacefor.6p",  0x1000, 0x0800, CRC(95446911) SHA1(843025d1c557156f73c2e9a1278c02738b69fb5d) )
	ROM_LOAD( "gm2v1.7p",     0x1800, 0x0800, CRC(0cd89ce2) SHA1(adb101400eb00119930494e99629948248d99d2f) )
	ROM_LOAD( "bm1v2.6m",     0x2000, 0x0800, CRC(130869ce) SHA1(588d6c9403d5fd966266b4f0333ee47b36c8b1d8) )
	ROM_LOAD( "bm2v1.7m",     0x2800, 0x0800, CRC(472f0a9b) SHA1(a8a9e2aa62374cd3bd938b5cb5fb20face3114c3) )
ROM_END

ROM_START( meteor )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vm1",          0x0000, 0x0800, CRC(894fe9b1) SHA1(617e05523392e2ba2608ca13aa24d6601289fe87) )
	ROM_LOAD( "vm2",          0x0800, 0x0800, CRC(28685a68) SHA1(f911a3ccb8d63cf82a6dc8f069f3f498e9081656) )
	ROM_LOAD( "vm3",          0x1000, 0x0800, CRC(c88fb12a) SHA1(1eeb26caf7a1421ec2d570f71b8c4675ad7ea172) )
	ROM_LOAD( "vm4",          0x1800, 0x0800, CRC(c5b073b9) SHA1(93b77c77488aa954c35880439be6c7629448a3ea) )
							/*0x2000 empty */
	ROM_LOAD( "vm6",          0x2800, 0x0800, CRC(9969ec43) SHA1(3ce067c34b84e9559f195e7ef9939a78070693b1) )
	ROM_LOAD( "vm7",          0x3000, 0x0800, CRC(39f43ac2) SHA1(b45275759f4003a22a32dc04227a98908bd140a9) )
	ROM_LOAD( "vm8",          0x3800, 0x0800, CRC(a0508de3) SHA1(75666a4e46b6c433f1c1f8e76c30fd087354097b) )

	ROM_REGION( 0x1000, "audiocpu", 0 )     /* sound MCU */
	ROM_LOAD( "vm5",          0x0000, 0x0800, CRC(b14ccd57) SHA1(0349ec5d0ca7f98ffdd96d7bf01cf096fe547f7a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rm1v",         0x0000, 0x0800, CRC(d621fe96) SHA1(29b75333ea8103095a4d452636eea4a1055845e5) )
	ROM_LOAD( "rm2v",         0x0800, 0x0800, CRC(b3981251) SHA1(b6743d121a6b3ad8e8beebe1faff2678b89e7d16) )
	ROM_LOAD( "gm1v",         0x1000, 0x0800, CRC(d44617e8) SHA1(1cec7984cc5e3472c25c23f02179380c4a5b4076) )
	ROM_LOAD( "gm2v",         0x1800, 0x0800, CRC(0997d945) SHA1(16eba77b14c62b2a0ebea47a28d4d5d21d7a2234) )
	ROM_LOAD( "bm1v",         0x2000, 0x0800, CRC(cc97c890) SHA1(e852bfe9d4b2d31801a840c1bacdd4386a93a22f) )
	ROM_LOAD( "bm2v",         0x2800, 0x0800, CRC(2858cf5c) SHA1(1313b4e4adda074499153e4a42bc2c6b41b0ec7e) )
ROM_END

ROM_START( meteors )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1hz1_2.1ab",          0x0000, 0x0800, CRC(86de2a63) SHA1(083a0d31f29bd9d68d240b23234645eeea57556d) )
	ROM_LOAD( "2hz1_2.1cd",          0x0800, 0x0800, CRC(7ef2c421) SHA1(f01327748e5a2144744557cd3cef16c93076466c) )
	ROM_LOAD( "3hz1_2.2ab",          0x1000, 0x0800, CRC(6d631f33) SHA1(4c69e3761d7db5ed6c8c23cc5e255cacfac6137f) )
	ROM_LOAD( "4hz1_2.2cd",          0x1800, 0x0800, CRC(48cb5acc) SHA1(791f9ca0225465d7af8a2a61f617112570f529e6))
							/*0x2000 empty */
	ROM_LOAD( "6hz1_2.3cd",          0x2800, 0x0800, CRC(39541265) SHA1(e55eb6c826fb553123991577be4daa3c2aa236f6) )
	ROM_LOAD( "7hz1_2.4ab",          0x3000, 0x0800, CRC(e718e807) SHA1(d8f3f66aea409c296785d66937b067f4b2c76ed4) )
	ROM_LOAD( "mbv21_2.4cd",         0x3800, 0x0800, CRC(f805c3cd) SHA1(78eb13b99aae895742b34ed56bee9313d3643de1) )

	ROM_REGION( 0x1000, "audiocpu", 0 )     /* sound MCU */
	ROM_LOAD( "vms.10l",          0x0000, 0x0800, CRC(b14ccd57) SHA1(0349ec5d0ca7f98ffdd96d7bf01cf096fe547f7a))

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rm1h1_2.6st",         0x0000, 0x0800, CRC(409fef31) SHA1(7260e06fa654d54f3660712a63f8db8c28b872c9) )
	ROM_LOAD( "rm2h1_2.7st",         0x0800, 0x0800, CRC(b3981251) SHA1(b6743d121a6b3ad8e8beebe1faff2678b89e7d16) )
	ROM_LOAD( "gm1h1_2.6pr",         0x1000, 0x0800, CRC(0b85c282) SHA1(b264c92d4b2533c18ac7831491133170a2fd400b) )
	ROM_LOAD( "gm2h1_2.7pr",         0x1800, 0x0800, CRC(0997d945) SHA1(16eba77b14c62b2a0ebea47a28d4d5d21d7a2234) )
	ROM_LOAD( "bm1h1_2.6nm",         0x2000, 0x0800, CRC(f9501c8e) SHA1(483d3d4c3f9601d7fcbf263bba5bdc5529b13f70) )
	ROM_LOAD( "bm2h1_2.7nm",         0x2800, 0x0800, CRC(2858cf5c) SHA1(1313b4e4adda074499153e4a42bc2c6b41b0ec7e) )
ROM_END


GAME( 1980, spcforce, 0,        spcforce, spcforce, driver_device, 0, ROT270, "Venture Line", "Space Force (set 1)", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 19??, spcforc2, spcforce, spcforce, spcforc2, driver_device, 0, ROT270, "bootleg? (Elcon)", "Space Force (set 2)", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, meteor,   spcforce, spcforce, spcforc2, driver_device, 0, ROT270, "Venture Line", "Meteoroids", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 19??, meteors,  spcforce, meteors,  spcforc2, driver_device, 0, ROT0,   "Amusement World", "Meteors", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
