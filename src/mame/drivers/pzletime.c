/*****************************************************************************************

    Puzzle Time (Prototype)
    Elettronica Video-Games S.R.L, 199?

    driver by Angelo Salese and Pierpaolo Prazzoli
    dump and info provided by Yoshi

    To initialize the eeprom, keep Service button pressed at boot.

    Notes:
    - Text tilemap flickering could be a bit slower / faster
    - Brightness effect could be a bit darker / lighter

*****************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/eeprom.h"

class pzletime_state : public driver_device
{
public:
	pzletime_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *       m_bg_videoram;
	UINT16 *       m_mid_videoram;
	UINT16 *       m_txt_videoram;
	UINT16 *       m_tilemap_regs;
	UINT16 *       m_video_regs;
	UINT16 *       m_spriteram;
//  UINT16 *       m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t      *m_mid_tilemap;
	tilemap_t      *m_txt_tilemap;

	/* misc */
	int            m_ticket;
	DECLARE_WRITE16_MEMBER(mid_videoram_w);
	DECLARE_WRITE16_MEMBER(txt_videoram_w);
	DECLARE_WRITE16_MEMBER(ticket_w);
	DECLARE_WRITE16_MEMBER(video_regs_w);
};


static TILE_GET_INFO( get_mid_tile_info )
{
	pzletime_state *state = machine.driver_data<pzletime_state>();
	int tileno = state->m_mid_videoram[tile_index] & 0x0fff;
	int colour = state->m_mid_videoram[tile_index] & 0xf000;
	colour = colour >> 12;
	SET_TILE_INFO(2, tileno, colour, 0);
}

static TILE_GET_INFO( get_txt_tile_info )
{
	pzletime_state *state = machine.driver_data<pzletime_state>();
	int tileno = state->m_txt_videoram[tile_index] & 0x0fff;
	int colour = state->m_txt_videoram[tile_index] & 0xf000;
	colour = colour >> 12;

	SET_TILE_INFO(0, tileno, colour, 0);

	tileinfo.category = BIT(colour, 3);
}

static VIDEO_START( pzletime )
{
	pzletime_state *state = machine.driver_data<pzletime_state>();

	state->m_mid_tilemap = tilemap_create(machine, get_mid_tile_info, tilemap_scan_cols, 16, 16, 64, 16);
	state->m_txt_tilemap = tilemap_create(machine, get_txt_tile_info, tilemap_scan_rows,  8, 8, 64, 32);

	state->m_mid_tilemap->set_transparent_pen(0);
	state->m_txt_tilemap->set_transparent_pen(0);
}

static SCREEN_UPDATE_IND16( pzletime )
{
	pzletime_state *state = screen.machine().driver_data<pzletime_state>();
	int count;
	int y, x;

	bitmap.fill(screen.machine().pens[0], cliprect); //bg pen

	state->m_txt_tilemap->set_scrolly(0, state->m_tilemap_regs[0] - 3);
	state->m_txt_tilemap->set_scrollx(0, state->m_tilemap_regs[1]);

	state->m_mid_tilemap->set_scrolly(0, state->m_tilemap_regs[2] - 3);
	state->m_mid_tilemap->set_scrollx(0, state->m_tilemap_regs[3] - 7);

	if (state->m_video_regs[2] & 1)
	{
		count = 0;

		for (y = 255; y >= 0; y--)
		{
			for (x = 0; x < 512; x++)
			{
				if (state->m_bg_videoram[count] & 0x8000)
					bitmap.pix16((y - 18) & 0xff, (x - 32) & 0x1ff) = 0x300 + (state->m_bg_videoram[count] & 0x7fff);

				count++;
			}
		}
	}

	state->m_mid_tilemap->draw(bitmap, cliprect, 0, 0);

	{
		UINT16 *spriteram = state->m_spriteram;
		int offs, spr_offs, colour, sx, sy;

		for(offs = 0; offs < 0x2000 / 2; offs += 4)
		{
			if(spriteram[offs + 0] == 8)
				break;

			spr_offs = spriteram[offs + 3] & 0x0fff;
			sy = 0x200 - (spriteram[offs + 0] & 0x1ff) - 35;
			sx = (spriteram[offs + 1] & 0x1ff) - 30;
			colour = (spriteram[offs + 0] & 0xf000) >> 12;

			// is spriteram[offs + 0] & 0x200 flipy? it's always set

			drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[1], spr_offs, colour, 0, 1, sx, sy, 0);
		}
	}

	state->m_txt_tilemap->draw(bitmap, cliprect, 0, 0);
	if ((screen.frame_number() % 16) != 0)
		state->m_txt_tilemap->draw(bitmap, cliprect, 1, 0);

	return 0;
}

WRITE16_MEMBER(pzletime_state::mid_videoram_w)
{
	COMBINE_DATA(&m_mid_videoram[offset]);
	m_mid_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(pzletime_state::txt_videoram_w)
{
	COMBINE_DATA(&m_txt_videoram[offset]);
	m_txt_tilemap->mark_tile_dirty(offset);
}

static WRITE16_DEVICE_HANDLER( eeprom_w )
{
	if (ACCESSING_BITS_0_7)
	{
		eeprom_device *eeprom = downcast<eeprom_device *>(device);
		eeprom->write_bit(data & 0x01);
		eeprom->set_cs_line((data & 0x02) ? CLEAR_LINE : ASSERT_LINE );
		eeprom->set_clock_line((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
	}
}

WRITE16_MEMBER(pzletime_state::ticket_w)
{

	if (ACCESSING_BITS_0_7)
		m_ticket = data & 1;
}

WRITE16_MEMBER(pzletime_state::video_regs_w)
{
	int i;

	COMBINE_DATA(&m_video_regs[offset]);

	if (offset == 0)
	{
		if (m_video_regs[0] > 0)
		{
			for (i = 0; i < 0x300; i++)
			{
				palette_set_pen_contrast(machine(), i, (double)0x8000/(double)m_video_regs[0]);
			}
		}
	}
	else if (offset == 1)
	{
		if (m_video_regs[1] > 0)
		{
			for (i = 0x300; i < 32768 + 0x300; i++)
			{
				palette_set_pen_contrast(machine(), i, (double)0x8000/(double)m_video_regs[1]);
			}
		}
	}
}

static WRITE16_DEVICE_HANDLER( oki_bank_w )
{
	downcast<okim6295_device *>(device)->set_bank_base(0x40000 * (data & 0x3));
}

static CUSTOM_INPUT( ticket_status_r )
{
	pzletime_state *state = field.machine().driver_data<pzletime_state>();
	return (state->m_ticket && !(field.machine().primary_screen->frame_number() % 128));
}

static ADDRESS_MAP_START( pzletime_map, AS_PROGRAM, 16, pzletime_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x700000, 0x700005) AM_RAM_WRITE(video_regs_w) AM_BASE(m_video_regs)
	AM_RANGE(0x800000, 0x800001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x900000, 0x9005ff) AM_RAM_WRITE_LEGACY(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xa00000, 0xa00007) AM_RAM AM_BASE(m_tilemap_regs)
	AM_RANGE(0xb00000, 0xb3ffff) AM_RAM AM_BASE(m_bg_videoram)
	AM_RANGE(0xc00000, 0xc00fff) AM_RAM_WRITE(mid_videoram_w) AM_BASE(m_mid_videoram)
	AM_RANGE(0xc01000, 0xc01fff) AM_RAM_WRITE(txt_videoram_w) AM_BASE(m_txt_videoram)
	AM_RANGE(0xd00000, 0xd01fff) AM_RAM AM_BASE(m_spriteram)
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("INPUT") AM_DEVWRITE_LEGACY("eeprom", eeprom_w)
	AM_RANGE(0xe00002, 0xe00003) AM_READ_PORT("SYSTEM") AM_WRITE(ticket_w)
	AM_RANGE(0xe00004, 0xe00005) AM_DEVWRITE_LEGACY("oki", oki_bank_w)
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( pzletime )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit) /* eeprom */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ticket_status_r, NULL) /* ticket dispenser */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static const gfx_layout layout8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout layout16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24, 16*32+4, 16*32+0, 16*32+12, 16*32+8, 16*32+20, 16*32+16, 16*32+28, 16*32+24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};

static GFXDECODE_START( pzletime )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8,   0x100, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, layout16x16, 0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx3", 0, layout16x16, 0x000, 0x10 )
GFXDECODE_END

static PALETTE_INIT( pzletime )
{
	int i;

	/* first 0x300 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette_set_color_rgb(machine, i + 0x300, pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0));
}

static MACHINE_START( pzletime )
{
	pzletime_state *state = machine.driver_data<pzletime_state>();

	state->save_item(NAME(state->m_ticket));
}

static MACHINE_RESET( pzletime )
{
	pzletime_state *state = machine.driver_data<pzletime_state>();

	state->m_ticket = 0;
}

static MACHINE_CONFIG_START( pzletime, pzletime_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,10000000)
	MCFG_CPU_PROGRAM_MAP(pzletime_map)
	MCFG_CPU_VBLANK_INT("screen",irq4_line_hold)

	MCFG_MACHINE_START(pzletime)
	MCFG_MACHINE_RESET(pzletime)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_STATIC(pzletime)
	MCFG_GFXDECODE(pzletime)
	MCFG_PALETTE_LENGTH(0x300 + 32768)
	MCFG_EEPROM_93C46_ADD("eeprom")

	MCFG_PALETTE_INIT(pzletime)
	MCFG_VIDEO_START(pzletime)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 937500, OKIM6295_PIN7_HIGH) //freq & pin7 taken from stlforce
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pzletime )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "5.bin", 0x000000, 0x80000, CRC(78b027dc) SHA1(6719908a075ecf0666bb817ac8a31056a7f315c6) )
	ROM_LOAD16_BYTE( "1.bin", 0x000001, 0x80000, CRC(0a69cbc7) SHA1(bae8b5746209c6773da27acaec7bd535a69019d2) )
	ROM_LOAD16_BYTE( "6.bin", 0x100000, 0x80000, CRC(526733ef) SHA1(21a921416d1ae7b9d49789d70ae99f240b012489) )
	ROM_LOAD16_BYTE( "2.bin", 0x100001, 0x80000, CRC(2a877266) SHA1(b8e909b3bd21af71782c501fa6eef590045b81e0) )
	ROM_LOAD16_BYTE( "7.bin", 0x200000, 0x80000, CRC(2efdd6d3) SHA1(de35d7a1bcd3ad608b8dfc184e06d6719253a1c7) )
	ROM_LOAD16_BYTE( "3.bin", 0x200001, 0x80000, CRC(1ddacade) SHA1(78f09fdb541e369765abfdf39607ca8f4c771d16) )
	ROM_LOAD16_BYTE( "8.bin", 0x300000, 0x80000, CRC(be7cf043) SHA1(5dadafb6f89f2fc373b77b18746b461117228f08) )
	ROM_LOAD16_BYTE( "4.bin", 0x300001, 0x80000, CRC(374ab900) SHA1(bd7f649bdf2927c1f5cb53492a08cc66c4658a72) )

	ROM_REGION( 0x80000, "user1", 0 ) /* Samples */
	ROM_LOAD( "12.bin",  0x00000, 0x80000,  CRC(203897c1) SHA1(c2495871c796bc7f2dabca1630317313b5aa740a) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_COPY( "user1", 0x000000, 0x000000, 0x020000 )
	ROM_COPY( "user1", 0x000000, 0x020000, 0x020000 )
	ROM_COPY( "user1", 0x000000, 0x040000, 0x020000 )
	ROM_COPY( "user1", 0x020000, 0x060000, 0x020000 )
	ROM_COPY( "user1", 0x000000, 0x080000, 0x020000 )
	ROM_COPY( "user1", 0x040000, 0x0a0000, 0x020000 )
	ROM_COPY( "user1", 0x000000, 0x0c0000, 0x020000 )
	ROM_COPY( "user1", 0x060000, 0x0e0000, 0x020000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "10.bin",  0x00000, 0x80000, CRC(d6ed11a5) SHA1(585aad4e962e7c9ba33e96d4d53e2feddd1a6cd9) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "11.bin",  0x000000, 0x80000, CRC(566e09a3) SHA1(b04d23bd82c609f35e6b651006b5c029f36f54dc) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "9.bin",   0x000000, 0x80000, CRC(a8144a7e) SHA1(9dfdd6c17a91cad6b56c622671042ac2ee2c9ec8) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "pzletime.nv", 0x0000, 0x0080, CRC(e5ed3d40) SHA1(8c163a6e5839e5c82d52f046d3268202fdf9f4d1) )
ROM_END

GAME( 199?, pzletime, 0, pzletime,  pzletime,  0, ROT0, "Elettronica Video-Games S.R.L.", "Puzzle Time (prototype)", GAME_SUPPORTS_SAVE )
