// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli
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
#include "machine/eepromser.h"

class pzletime_state : public driver_device
{
public:
	pzletime_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_regs(*this, "video_regs"),
		m_tilemap_regs(*this, "tilemap_regs"),
		m_bg_videoram(*this, "bg_videoram"),
		m_mid_videoram(*this, "mid_videoram"),
		m_txt_videoram(*this, "txt_videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_video_regs;
	required_shared_ptr<UINT16> m_tilemap_regs;
	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_mid_videoram;
	required_shared_ptr<UINT16> m_txt_videoram;
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	tilemap_t      *m_mid_tilemap;
	tilemap_t      *m_txt_tilemap;

	/* misc */
	int            m_ticket;
	DECLARE_WRITE16_MEMBER(mid_videoram_w);
	DECLARE_WRITE16_MEMBER(txt_videoram_w);
	DECLARE_WRITE16_MEMBER(ticket_w);
	DECLARE_WRITE16_MEMBER(video_regs_w);
	DECLARE_CUSTOM_INPUT_MEMBER(ticket_status_r);
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE16_MEMBER(oki_bank_w);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_txt_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(pzletime);
	UINT32 screen_update_pzletime(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};


TILE_GET_INFO_MEMBER(pzletime_state::get_mid_tile_info)
{
	int tileno = m_mid_videoram[tile_index] & 0x0fff;
	int colour = m_mid_videoram[tile_index] & 0xf000;
	colour = colour >> 12;
	SET_TILE_INFO_MEMBER(2, tileno, colour, 0);
}

TILE_GET_INFO_MEMBER(pzletime_state::get_txt_tile_info)
{
	int tileno = m_txt_videoram[tile_index] & 0x0fff;
	int colour = m_txt_videoram[tile_index] & 0xf000;
	colour = colour >> 12;

	SET_TILE_INFO_MEMBER(0, tileno, colour, 0);

	tileinfo.category = BIT(colour, 3);
}

void pzletime_state::video_start()
{
	m_mid_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pzletime_state::get_mid_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 64, 16);
	m_txt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pzletime_state::get_txt_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);

	m_mid_tilemap->set_transparent_pen(0);
	m_txt_tilemap->set_transparent_pen(0);
}

UINT32 pzletime_state::screen_update_pzletime(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count;
	int y, x;

	bitmap.fill(m_palette->pen(0), cliprect); //bg pen

	m_txt_tilemap->set_scrolly(0, m_tilemap_regs[0] - 3);
	m_txt_tilemap->set_scrollx(0, m_tilemap_regs[1]);

	m_mid_tilemap->set_scrolly(0, m_tilemap_regs[2] - 3);
	m_mid_tilemap->set_scrollx(0, m_tilemap_regs[3] - 7);

	if (m_video_regs[2] & 1)
	{
		count = 0;

		for (y = 255; y >= 0; y--)
		{
			for (x = 0; x < 512; x++)
			{
				if (m_bg_videoram[count] & 0x8000)
					bitmap.pix16((y - 18) & 0xff, (x - 32) & 0x1ff) = 0x300 + (m_bg_videoram[count] & 0x7fff);

				count++;
			}
		}
	}

	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	{
		UINT16 *spriteram = m_spriteram;
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

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, spr_offs, colour, 0, 1, sx, sy, 0);
		}
	}

	m_txt_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if ((screen.frame_number() % 16) != 0)
		m_txt_tilemap->draw(screen, bitmap, cliprect, 1, 0);

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

WRITE16_MEMBER(pzletime_state::eeprom_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->di_write(data & 0x01);
		m_eeprom->cs_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE );
		m_eeprom->clk_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
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
				m_palette->set_pen_contrast(i, (double)0x8000/(double)m_video_regs[0]);
			}
		}
	}
	else if (offset == 1)
	{
		if (m_video_regs[1] > 0)
		{
			for (i = 0x300; i < 32768 + 0x300; i++)
			{
				m_palette->set_pen_contrast(i, (double)0x8000/(double)m_video_regs[1]);
			}
		}
	}
}

WRITE16_MEMBER(pzletime_state::oki_bank_w)
{
	m_oki->set_bank_base(0x40000 * (data & 0x3));
}

CUSTOM_INPUT_MEMBER(pzletime_state::ticket_status_r)
{
	return (m_ticket && !(m_screen->frame_number() % 128));
}

static ADDRESS_MAP_START( pzletime_map, AS_PROGRAM, 16, pzletime_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x700000, 0x700005) AM_RAM_WRITE(video_regs_w) AM_SHARE("video_regs")
	AM_RANGE(0x800000, 0x800001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x900000, 0x9005ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xa00000, 0xa00007) AM_RAM AM_SHARE("tilemap_regs")
	AM_RANGE(0xb00000, 0xb3ffff) AM_RAM AM_SHARE("bg_videoram")
	AM_RANGE(0xc00000, 0xc00fff) AM_RAM_WRITE(mid_videoram_w) AM_SHARE("mid_videoram")
	AM_RANGE(0xc01000, 0xc01fff) AM_RAM_WRITE(txt_videoram_w) AM_SHARE("txt_videoram")
	AM_RANGE(0xd00000, 0xd01fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("INPUT") AM_WRITE(eeprom_w)
	AM_RANGE(0xe00002, 0xe00003) AM_READ_PORT("SYSTEM") AM_WRITE(ticket_w)
	AM_RANGE(0xe00004, 0xe00005) AM_WRITE(oki_bank_w)
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( pzletime )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) /* eeprom */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, pzletime_state,ticket_status_r, NULL) /* ticket dispenser */
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

PALETTE_INIT_MEMBER(pzletime_state, pzletime)
{
	int i;

	/* first 0x300 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette.set_pen_color(i + 0x300, pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0));
}

void pzletime_state::machine_start()
{
	save_item(NAME(m_ticket));
}

void pzletime_state::machine_reset()
{
	m_ticket = 0;
}

static MACHINE_CONFIG_START( pzletime, pzletime_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,10000000)
	MCFG_CPU_PROGRAM_MAP(pzletime_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pzletime_state, irq4_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pzletime_state, screen_update_pzletime)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pzletime)
	MCFG_PALETTE_ADD("palette", 0x300 + 32768)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_PALETTE_INIT_OWNER(pzletime_state, pzletime)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")


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

GAME( 199?, pzletime, 0, pzletime,  pzletime, driver_device,  0, ROT0, "Elettronica Video-Games S.R.L.", "Puzzle Time (prototype)", MACHINE_SUPPORTS_SAVE )
