// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina

/* Field Combat (c)1985 Jaleco

    TS 2004.10.22. analog[at]op.pl
    - fixed sprite issues
    - added backgrounds and terrain info (external roms)

    (press buttons 1+2 at the same time, to release 'army' ;)

    todo:
        - fix colours (sprites , bg)
*/

/*

Field Combat (c)1985 Jaleco

From a working board.

CPU: Z80 (running at 3.332 MHz measured at pin 6)
Sound: Z80 (running at 3.332 MHz measured at pin 6), YM2149 (x3)
Other: Unmarked 24 pin near ROMs 2 & 3

RAM: 6116 (x3)

X-TAL: 20 MHz

inputs + notes by stephh

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/fcombat.h"


INPUT_CHANGED_MEMBER(fcombat_state::coin_inserted)
{
	/* coin insertion causes an NMI */
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}



/* is it protection? */
READ8_MEMBER(fcombat_state::fcombat_protection_r)
{
	/* Must match ONE of these values after a "and  $3E" intruction :

	    76F0: 1E 04 2E 26 34 32 3A 16 3E 36

	   Check code at 0x76c8 for more infos.
	*/
	return 0xff;    // seems enough
}


/* same as exerion again */

READ8_MEMBER(fcombat_state::fcombat_port01_r)
{
	/* the cocktail flip bit muxes between ports 0 and 1 */
	return ioport(m_cocktail_flip ? "IN1" : "IN0")->read();
}


//bg scrolls

WRITE8_MEMBER(fcombat_state::e900_w)
{
	m_fcombat_sh = data;
}

WRITE8_MEMBER(fcombat_state::ea00_w)
{
	m_fcombat_sv = (m_fcombat_sv & 0xff00) | data;
}

WRITE8_MEMBER(fcombat_state::eb00_w)
{
	m_fcombat_sv = (m_fcombat_sv & 0xff) | (data << 8);
}


// terrain info (ec00=x, ed00=y, return val in e300

WRITE8_MEMBER(fcombat_state::ec00_w)
{
	m_tx = data;
}

WRITE8_MEMBER(fcombat_state::ed00_w)
{
	m_ty = data;
}

READ8_MEMBER(fcombat_state::e300_r)
{
	int wx = (m_tx + m_fcombat_sh) / 16;
	int wy = (m_ty * 2 + m_fcombat_sv) / 16;

	return memregion("user2")->base()[wx * 32 * 16 + wy];
}

WRITE8_MEMBER(fcombat_state::ee00_w)
{
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, fcombat_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xd8ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe000, 0xe000) AM_READ(fcombat_port01_r)
	AM_RANGE(0xe100, 0xe100) AM_READ_PORT("DSW0")
	AM_RANGE(0xe200, 0xe200) AM_READ_PORT("DSW1")
	AM_RANGE(0xe300, 0xe300) AM_READ(e300_r)
	AM_RANGE(0xe400, 0xe400) AM_READ(fcombat_protection_r) // protection?
	AM_RANGE(0xe800, 0xe800) AM_WRITE(fcombat_videoreg_w)   // at least bit 0 for flip screen and joystick input multiplexor
	AM_RANGE(0xe900, 0xe900) AM_WRITE(e900_w)
	AM_RANGE(0xea00, 0xea00) AM_WRITE(ea00_w)
	AM_RANGE(0xeb00, 0xeb00) AM_WRITE(eb00_w)
	AM_RANGE(0xec00, 0xec00) AM_WRITE(ec00_w)
	AM_RANGE(0xed00, 0xed00) AM_WRITE(ed00_w)
	AM_RANGE(0xee00, 0xee00) AM_WRITE(ee00_w)   // related to protection ? - doesn't seem to have any effect
	AM_RANGE(0xef00, 0xef00) AM_WRITE(soundlatch_byte_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, fcombat_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8001, 0x8001) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x8002, 0x8003) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0xa002, 0xa003) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
	AM_RANGE(0xc001, 0xc001) AM_DEVREAD("ay3", ay8910_device, data_r)
	AM_RANGE(0xc002, 0xc003) AM_DEVWRITE("ay3", ay8910_device, data_address_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( fcombat )
	PORT_START("IN0")      /* player 1 inputs (muxed on 0xe000) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")      /* player 2 inputs (muxed on 0xe000) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW0")      /* dip switches (0xe100) */
	PORT_DIPNAME( 0x07, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x07, "Infinite (Cheat)")
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x18, "40000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")      /* dip switches/VBLANK (0xe200) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )      // related to vblank
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, fcombat_state,coin_inserted, 0)
INPUT_PORTS_END


/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7 },
	16*8
};


/* 16 x 16 sprites -- requires reorganizing characters in init_exerion() */
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{  3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16+3, 16+2, 16+1, 16+0, 24+3, 24+2, 24+1, 24+0 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7,
			32*8, 32*9, 32*10, 32*11, 32*12, 32*13, 32*14, 32*15 },
	64*8
};


static GFXDECODE_START( fcombat )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,     256, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,     512, 64 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void fcombat_state::machine_start()
{
	save_item(NAME(m_cocktail_flip));
	save_item(NAME(m_char_palette));
	save_item(NAME(m_sprite_palette));
	save_item(NAME(m_char_bank));
	save_item(NAME(m_fcombat_sh));
	save_item(NAME(m_fcombat_sv));
	save_item(NAME(m_tx));
	save_item(NAME(m_ty));
}

void fcombat_state::machine_reset()
{
	m_cocktail_flip = 0;
	m_char_palette = 0;
	m_sprite_palette = 0;
	m_char_bank = 0;
	m_fcombat_sh = 0;
	m_fcombat_sv = 0;
	m_tx = 0;
	m_ty = 0;
}

static MACHINE_CONFIG_START( fcombat, fcombat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 10000000/3)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_ADD("audiocpu", Z80, 10000000/3)
	MCFG_CPU_PROGRAM_MAP(audio_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(FCOMBAT_PIXEL_CLOCK, FCOMBAT_HTOTAL, FCOMBAT_HBEND, FCOMBAT_HBSTART, FCOMBAT_VTOTAL, FCOMBAT_VBEND, FCOMBAT_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(fcombat_state, screen_update_fcombat)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fcombat)
	MCFG_PALETTE_ADD("palette", 256*3)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(fcombat_state, fcombat)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)

	MCFG_SOUND_ADD("ay3", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)
MACHINE_CONFIG_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(fcombat_state,fcombat)
{
	UINT32 oldaddr, newaddr, length;
	UINT8 *src, *dst;

	/* allocate some temporary space */
	dynamic_buffer temp(0x10000);

	/* make a temporary copy of the character data */
	src = &temp[0];
	dst = memregion("gfx1")->base();
	length = memregion("gfx1")->bytes();
	memcpy(src, dst, length);

	/* decode the characters */
	/* the bits in the ROM are ordered: n8-n7 n6 n5 n4-v2 v1 v0 n3-n2 n1 n0 h2 */
	/* we want them ordered like this:  n8-n7 n6 n5 n4-n3 n2 n1 n0-v2 v1 v0 h2 */
	for (oldaddr = 0; oldaddr < length; oldaddr++)
	{
		newaddr = ((oldaddr     ) & 0x1f00) |       /* keep n8-n4 */
					((oldaddr << 3) & 0x00f0) |       /* move n3-n0 */
					((oldaddr >> 4) & 0x000e) |       /* move v2-v0 */
					((oldaddr     ) & 0x0001);        /* keep h2 */
		dst[newaddr] = src[oldaddr];
	}

	/* make a temporary copy of the sprite data */
	src = &temp[0];
	dst = memregion("gfx2")->base();
	length = memregion("gfx2")->bytes();
	memcpy(src, dst, length);

	/* decode the sprites */
	/* the bits in the ROMs are ordered: n9 n8 n3 n7-n6 n5 n4 v3-v2 v1 v0 n2-n1 n0 h3 h2 */
	/* we want them ordered like this:   n9 n8 n7 n6-n5 n4 n3 n2-n1 n0 v3 v2-v1 v0 h3 h2 */

	for (oldaddr = 0; oldaddr < length; oldaddr++)
	{
		newaddr = ((oldaddr << 1) & 0x3c00) |       /* move n7-n4 */
					((oldaddr >> 4) & 0x0200) |       /* move n3 */
					((oldaddr << 4) & 0x01c0) |       /* move n2-n0 */
					((oldaddr >> 3) & 0x003c) |       /* move v3-v0 */
					((oldaddr     ) & 0xc003);        /* keep n9-n8 h3-h2 */

		dst[newaddr] = src[oldaddr];
	}

	/* make a temporary copy of the character data */
	src = &temp[0];
	dst = memregion("gfx3")->base();
	length = memregion("gfx3")->bytes();
	memcpy(src, dst, length);

	/* decode the characters */
	/* the bits in the ROM are ordered: n8-n7 n6 n5 n4-v2 v1 v0 n3-n2 n1 n0 h2 */
	/* we want them ordered like this:  n8-n7 n6 n5 n4-n3 n2 n1 n0-v2 v1 v0 h2 */

	for (oldaddr = 0; oldaddr < length; oldaddr++)
	{
		newaddr = ((oldaddr << 1) & 0x3c00) |       /* move n7-n4 */
					((oldaddr >> 4) & 0x0200) |       /* move n3 */
					((oldaddr << 4) & 0x01c0) |       /* move n2-n0 */
					((oldaddr >> 3) & 0x003c) |       /* move v3-v0 */
					((oldaddr     ) & 0xc003);        /* keep n9-n8 h3-h2 */
		dst[newaddr] = src[oldaddr];
	}

	src = &temp[0];
	dst = memregion("user1")->base();
	length = memregion("user1")->bytes();
	memcpy(src, dst, length);

	for (oldaddr = 0; oldaddr < 32; oldaddr++)
	{
		memcpy(&dst[oldaddr * 32 * 8 * 2], &src[oldaddr * 32 * 8], 32 * 8);
		memcpy(&dst[oldaddr * 32 * 8 * 2 + 32 * 8], &src[oldaddr * 32 * 8 + 0x2000], 32 * 8);
	}


	src = &temp[0];
	dst = memregion("user2")->base();
	length = memregion("user2")->bytes();
	memcpy(src, dst, length);

	for (oldaddr = 0; oldaddr < 32; oldaddr++)
	{
		memcpy(&dst[oldaddr * 32 * 8 * 2], &src[oldaddr * 32 * 8], 32 * 8);
		memcpy(&dst[oldaddr * 32 * 8 * 2 + 32 * 8], &src[oldaddr * 32 * 8 + 0x2000], 32 * 8);
	}
}

ROM_START( fcombat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fcombat2.t9",  0x0000, 0x4000, CRC(30cb0c14) SHA1(8b5b6a4efaca2f138709184725e9e0e0b9cfc4c7) )
	ROM_LOAD( "fcombat3.10t", 0x4000, 0x4000, CRC(e8511da0) SHA1(bab5c9244c970b97c025381c37ad372aa3b5cddf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "fcombat1.t5",  0x0000, 0x4000, CRC(a0cc1216) SHA1(3a8963ffde2ff4a3f428369133f94bb37717cae5) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "fcombat7.l11", 0x00000, 0x2000, CRC(401061b5) SHA1(09dd23e86a56db8021e14432aced0eaf013fefe2) ) /* fg chars */

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "fcombat8.d10", 0x00000, 0x4000, CRC(e810941e) SHA1(19ae85af0bf245caf3afe10d65e618cfb47d33c2) ) /* sprites */
	ROM_LOAD( "fcombat9.d11", 0x04000, 0x4000, CRC(f95988e6) SHA1(25876652decca7ec1e9b37a16536c15ca2d1cb12) )
	ROM_LOAD( "fcomba10.d12", 0x08000, 0x4000, CRC(908f154c) SHA1(b3761ee60d4a5ea36376759875105d23c57b4bf2) )

	ROM_REGION( 0x04000, "gfx3", 0 )
	ROM_LOAD( "fcombat6.f3",  0x00000, 0x4000, CRC(97282729) SHA1(72db0593551c2d15631341bf621b96013b46ce72) )

	ROM_REGION( 0x04000, "user1", 0 )
	ROM_LOAD( "fcombat5.l3",  0x00000, 0x4000, CRC(96194ca7) SHA1(087d6ac8f93f087cb5e378dbe9a8cfcffa2cdddc) ) /* bg data */

	ROM_REGION( 0x04000, "user2", 0 )
	ROM_LOAD( "fcombat4.p3",  0x00000, 0x4000, CRC(efe098ab) SHA1(fe64a5e9170835d242368109b1b221b0f8090e7e) ) /* terrain info */

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "fcprom_a.c2",  0x0000, 0x0020, CRC(7ac480f0) SHA1(f491fe4da19d8c037e3733a5836de35cc438907e) ) /* palette */
	ROM_LOAD( "fcprom_d.k12", 0x0020, 0x0100, CRC(9a348250) SHA1(faf8db4c42adee07795d06bea20704f8c51090ff) ) /* fg char lookup table */
	ROM_LOAD( "fcprom_b.c4",  0x0120, 0x0100, CRC(ac9049f6) SHA1(57aa5b5df3e181bad76149745a422c3dd1edad49) ) /* sprite lookup table */
	ROM_LOAD( "fcprom_c.a9",  0x0220, 0x0100, CRC(768ac120) SHA1(ceede1d6cbeae08da96ef52bdca2718a839d88ab) ) /* bg char mixer */
ROM_END

GAME( 1985, fcombat,  0,       fcombat, fcombat, fcombat_state, fcombat,  ROT90, "Jaleco", "Field Combat", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
