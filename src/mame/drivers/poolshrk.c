// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Poolshark Driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "includes/poolshrk.h"
#include "sound/discrete.h"




DRIVER_INIT_MEMBER(poolshrk_state,poolshrk)
{
	UINT8* pSprite = memregion("gfx1")->base();
	UINT8* pOffset = memregion("proms")->base();

	/* re-arrange sprite data using the PROM */

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			UINT16 v =
				(pSprite[0] << 0xC) |
				(pSprite[1] << 0x8) |
				(pSprite[2] << 0x4) |
				(pSprite[3] << 0x0);

			v >>= pOffset[j];

			pSprite[0] = (v >> 0xC) & 15;
			pSprite[1] = (v >> 0x8) & 15;
			pSprite[2] = (v >> 0x4) & 15;
			pSprite[3] = (v >> 0x0) & 15;

			pSprite += 4;
		}
	}
	
	save_item(NAME(m_da_latch));
}


WRITE8_MEMBER(poolshrk_state::da_latch_w)
{
	m_da_latch = data & 15;
}


WRITE8_MEMBER(poolshrk_state::led_w)
{
	if (offset & 2)
		set_led_status(machine(), 0, offset & 1);
	if (offset & 4)
		set_led_status(machine(), 1, offset & 1);
}


WRITE8_MEMBER(poolshrk_state::watchdog_w)
{
	if ((offset & 3) == 3)
	{
		watchdog_reset_w(space, 0, 0);
	}
}


READ8_MEMBER(poolshrk_state::input_r)
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3" };
	UINT8 val = ioport(portnames[offset & 3])->read();

	int x = ioport((offset & 1) ? "AN1" : "AN0")->read();
	int y = ioport((offset & 1) ? "AN3" : "AN2")->read();

	if (x >= m_da_latch) val |= 8;
	if (y >= m_da_latch) val |= 4;

	if ((offset & 3) == 3)
	{
		watchdog_reset_r(space, 0);
	}

	return val;
}


READ8_MEMBER(poolshrk_state::irq_reset_r)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}


static ADDRESS_MAP_START( poolshrk_cpu_map, AS_PROGRAM, 8, poolshrk_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x2300) AM_RAM
	AM_RANGE(0x0400, 0x07ff) AM_MIRROR(0x2000) AM_WRITEONLY AM_SHARE("playfield_ram")
	AM_RANGE(0x0800, 0x080f) AM_MIRROR(0x23f0) AM_WRITEONLY AM_SHARE("hpos_ram")
	AM_RANGE(0x0c00, 0x0c0f) AM_MIRROR(0x23f0) AM_WRITEONLY AM_SHARE("vpos_ram")
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0x2000) AM_READWRITE(input_r, watchdog_w)
	AM_RANGE(0x1400, 0x17ff) AM_MIRROR(0x2000) AM_WRITE(scratch_sound_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x2000) AM_WRITE(score_sound_w)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x2000) AM_WRITE(click_sound_w)
	AM_RANGE(0x4000, 0x4000) AM_NOP /* diagnostic ROM location */
	AM_RANGE(0x6000, 0x63ff) AM_WRITE(da_latch_w)
	AM_RANGE(0x6400, 0x67ff) AM_WRITE(bump_sound_w)
	AM_RANGE(0x6800, 0x6bff) AM_READ(irq_reset_r)
	AM_RANGE(0x6c00, 0x6fff) AM_WRITE(led_w)
	AM_RANGE(0x7000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( poolshrk )
	PORT_START("IN0")
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, "Extended Play" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, "Racks Per Game" )
	PORT_DIPSETTING( 0x03, "2" )
	PORT_DIPSETTING( 0x02, "3" )
	PORT_DIPSETTING( 0x01, "4" )
	PORT_DIPSETTING( 0x00, "5" )
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ))
	PORT_DIPSETTING( 0x00, DEF_STR( 2C_1C ))
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_1C ))
	PORT_DIPSETTING( 0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_4C ))
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("AN0")
	PORT_BIT( 15, 8, IPT_AD_STICK_X ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 15, 8, IPT_AD_STICK_X ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(2)

	PORT_START("AN2")
	PORT_BIT( 15, 8, IPT_AD_STICK_Y ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN3")
	PORT_BIT( 15, 8, IPT_AD_STICK_Y ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_REVERSE PORT_PLAYER(2)

INPUT_PORTS_END


static const gfx_layout poolshrk_sprite_layout =
{
	16, 16,   /* width, height */
	16,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F,
		0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1A0, 0x1C0, 0x1E0
	},
	0x200     /* increment */
};


static const gfx_layout poolshrk_tile_layout =
{
	8, 8,     /* width, height */
	64,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		7, 6, 5, 4, 3, 2, 1, 0
	},
	{
		0x000, 0x200, 0x400, 0x600, 0x800, 0xA00, 0xC00, 0xE00
	},
	0x8       /* increment */
};


static GFXDECODE_START( poolshrk )
	GFXDECODE_ENTRY( "gfx1", 0, poolshrk_sprite_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, poolshrk_tile_layout, 0, 1 )
GFXDECODE_END


PALETTE_INIT_MEMBER(poolshrk_state, poolshrk)
{
	palette.set_pen_color(0,rgb_t(0x7F, 0x7F, 0x7F));
	palette.set_pen_color(1,rgb_t(0xFF, 0xFF, 0xFF));
	palette.set_pen_color(2,rgb_t(0x7F, 0x7F, 0x7F));
	palette.set_pen_color(3,rgb_t(0x00, 0x00, 0x00));
}


static MACHINE_CONFIG_START( poolshrk, poolshrk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 11055000 / 8) /* ? */
	MCFG_CPU_PROGRAM_MAP(poolshrk_cpu_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", poolshrk_state,  irq0_line_assert)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(1, 255, 24, 255)
	MCFG_SCREEN_UPDATE_DRIVER(poolshrk_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", poolshrk)
	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(poolshrk_state, poolshrk)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(poolshrk)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( poolshrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7329.k1", 0x7000, 0x800, CRC(88152245) SHA1(c7c5e43ea488a197e92a1dc2231578f8ed86c98d) )
	ROM_LOAD( "7330.l1", 0x7800, 0x800, CRC(fb41d3e9) SHA1(c17994179362da13acfcd36a28f45e328428c031) )

	ROM_REGION( 0x400, "gfx1", 0 )   /* sprites */
	ROM_LOAD( "7325.j5", 0x0000, 0x200, CRC(fae87eed) SHA1(8891d0ea60f72f826d71dc6b064a2ba81b298914) )
	ROM_LOAD( "7326.h5", 0x0200, 0x200, CRC(05ec9762) SHA1(6119c4529334c98a0a42ca13a98a8661fc594d80) )

	ROM_REGION( 0x200, "gfx2", 0 )   /* tiles */
	ROM_LOAD( "7328.n6", 0x0000, 0x200, CRC(64bcbf3a) SHA1(a4e3ce6b4734234359e3ef784a771e40580c2a2a) )

	ROM_REGION( 0x20, "proms", 0 )                   /* line offsets */
	ROM_LOAD( "7327.k6", 0x0000, 0x020, CRC(f74cef5b) SHA1(f470bf5b193dae4b44e89bc4c4476cf8d98e7cfd) )
ROM_END


GAME( 1977, poolshrk, 0, poolshrk, poolshrk, poolshrk_state, poolshrk, 0, "Atari", "Poolshark", GAME_SUPPORTS_SAVE )
