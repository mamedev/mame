// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************************************************************

   Super Duck (c) 1992 Comad

   hardware appears to be roughly based off Bionic Commando, close to the
   Tiger Road / F1-Dream based Pushman / Bouncing Balls.


PCB Clocks as measured:

Crystal 1: 8mhz
Crystal 2: 24mhz

All clock timing comes from crystal 1
 68k - 8mhz
 Z80 - 2mhz
 OKI M6295 - 1mhz

*********************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "video/bufsprite.h"
#include "video/tigeroad_spr.h"

class supduck_state : public driver_device
{
public:
	supduck_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_spriteram(*this, "spriteram") ,
			m_text_videoram(*this, "textvideoram"),
			m_fore_videoram(*this, "forevideoram"),
			m_back_videoram(*this, "backvideoram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),
			m_spritegen(*this, "spritegen")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;

	// shared pointers
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_text_videoram;
	required_shared_ptr<UINT16> m_fore_videoram;
	required_shared_ptr<UINT16> m_back_videoram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tigeroad_spr_device> m_spritegen;

	tilemap_t     *m_text_tilemap;
	tilemap_t     *m_fore_tilemap;
	tilemap_t     *m_back_tilemap;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE16_MEMBER(text_videoram_w);
	DECLARE_WRITE16_MEMBER(fore_videoram_w);
	DECLARE_WRITE16_MEMBER(back_videoram_w);
	DECLARE_WRITE16_MEMBER(supduck_scroll_w);

	DECLARE_WRITE16_MEMBER(supduck_4000_w);
	DECLARE_WRITE16_MEMBER(supduck_4002_w);

	TILEMAP_MAPPER_MEMBER(supduk_tilemap_scan);

	DECLARE_WRITE8_MEMBER(okibank_w);

protected:

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_back_tile_info);

};



TILEMAP_MAPPER_MEMBER(supduck_state::supduk_tilemap_scan)
{
	// where does each page start?
	int pagesize = 0x8 * 0x8;

	int offset = ((col & ~0x7) / 0x8) * (pagesize);
	offset += ((row^0x3f) & 0x7)*0x8;
	offset += col & 0x7;

	offset &= 0x3ff;

	offset += (((row^0x3f) & ~0x7) / 0x8) * 0x400;


	return offset;
}

void supduck_state::video_start()
{
	m_text_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supduck_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fore_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supduck_state::get_fore_tile_info),this), tilemap_mapper_delegate(FUNC(supduck_state::supduk_tilemap_scan),this), 32, 32, 128,64);
	m_back_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supduck_state::get_back_tile_info),this), tilemap_mapper_delegate(FUNC(supduck_state::supduk_tilemap_scan),this), 32, 32, 128,64);

	m_text_tilemap->set_transparent_pen(0x3);
	m_fore_tilemap->set_transparent_pen(0xf);

}

UINT32 supduck_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_back_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fore_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_spritegen->draw_sprites(bitmap, cliprect, m_gfxdecode, 3, m_spriteram->buffer(), m_spriteram->bytes(), flip_screen(), 1 );

	m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


WRITE16_MEMBER(supduck_state::text_videoram_w)
{
	COMBINE_DATA(&m_text_videoram[offset]);
	m_text_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(supduck_state::fore_videoram_w)
{
	COMBINE_DATA(&m_fore_videoram[offset]);
	m_fore_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(supduck_state::back_videoram_w)
{
	COMBINE_DATA(&m_back_videoram[offset]);
	m_back_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(supduck_state::get_text_tile_info) // same as tigeroad.c
{
	UINT16 *videoram = m_text_videoram;
	int data = videoram[tile_index];
	int attr = data >> 8;
	int code = (data & 0xff) + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5);
	int color = attr & 0x0f;
	int flags = (attr & 0x10) ? TILE_FLIPY : 0;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

TILE_GET_INFO_MEMBER(supduck_state::get_fore_tile_info)
{
	UINT16 *videoram = m_fore_videoram;
	int data = videoram[tile_index];
	int code = data & 0xff;
	if (data & 0x4000) code |= 0x100;
	if (data & 0x8000) code |= 0x200;

	int color = (data & 0x0f00)>>8;
	int flags = (data & 0x2000) ? TILE_FLIPX : 0;
		flags |=(data & 0x1000) ? TILE_FLIPY : 0;


	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(supduck_state::get_back_tile_info)
{
	UINT16 *videoram = m_back_videoram;
	int data = videoram[tile_index];

	int code = data & 0xff;
	if (data & 0x4000) code |= 0x100;
	if (data & 0x8000) code |= 0x200;

	int color = (data & 0x0f00)>>8;
	int flags = (data & 0x2000) ? TILE_FLIPX : 0;
		flags |=(data & 0x1000) ? TILE_FLIPY : 0;

	SET_TILE_INFO_MEMBER(2, code, color, flags);
}



WRITE16_MEMBER(supduck_state::supduck_4000_w)
{
}


WRITE16_MEMBER(supduck_state::supduck_4002_w)
{
	data &= mem_mask;

	soundlatch_byte_w(space, 0, (data>>8));
	m_audiocpu->set_input_line(0, HOLD_LINE);

}

WRITE16_MEMBER(supduck_state::supduck_scroll_w)
{
	data &= mem_mask;

	switch (offset)
	{
	case 0:
		m_back_tilemap->set_scrollx(0, data);
		break;
	case 1:
		m_back_tilemap->set_scrolly(0, -data - 32 * 8);
		break;
	case 2:
		m_fore_tilemap->set_scrollx(0, data);
		break;
	case 3:
		m_fore_tilemap->set_scrolly(0, -data - 32 * 8);
		break;
	}
}



static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, supduck_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_WRITENOP
	AM_RANGE(0xfe0000, 0xfe1fff) AM_RAM AM_SHARE("spriteram")

	AM_RANGE(0xfe4000, 0xfe4001) AM_READ_PORT("P1_P2") AM_WRITE( supduck_4000_w )
	AM_RANGE(0xfe4002, 0xfe4003) AM_READ_PORT("SYSTEM") AM_WRITE( supduck_4002_w )
	AM_RANGE(0xfe4004, 0xfe4005) AM_READ_PORT("DSW")

	AM_RANGE(0xfe8000, 0xfe8007) AM_WRITE(supduck_scroll_w)
	AM_RANGE(0xfe800e, 0xfe800f) AM_WRITENOP // watchdog or irqack

	AM_RANGE(0xfec000, 0xfecfff) AM_RAM_WRITE(text_videoram_w) AM_SHARE("textvideoram")
	AM_RANGE(0xff0000, 0xff3fff) AM_RAM_WRITE(back_videoram_w) AM_SHARE("backvideoram")
	AM_RANGE(0xff4000, 0xff7fff) AM_RAM_WRITE(fore_videoram_w) AM_SHARE("forevideoram")
	AM_RANGE(0xff8000, 0xff87ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xffc000, 0xffffff) AM_RAM /* working RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, supduck_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_WRITE(okibank_w)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( oki_map, AS_0, 8, supduck_state )
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END

WRITE8_MEMBER(supduck_state::okibank_w)
{
	// bit 0x80 is written on startup?

	membank("okibank")->set_entry(data&0x03);
}


static INPUT_PORTS_START( supduck )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") /* not sure, probably wrong */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIP-A:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DIP-A:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Game Sound" ) PORT_DIPLOCATION("DIP-A:6") /* Kills all sounds except for Coin-In */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0080, DEF_STR( Lives ) ) PORT_DIPLOCATION("DIP-A:7,8")
	PORT_DIPSETTING(      0x00c0, "2" )
	PORT_DIPSETTING(      0x0080, "3" )
	PORT_DIPSETTING(      0x0040, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Character Test" ) PORT_DIPLOCATION("DIP-B:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "DIP-B:8" )
INPUT_PORTS_END


static const gfx_layout spritelayout_bionicc=
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,4),   /* 2048 sprites */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{
		0,1,2,3,4,5,6,7,
		(16*8)+0,(16*8)+1,(16*8)+2,(16*8)+3,
		(16*8)+4,(16*8)+5,(16*8)+6,(16*8)+7
	},
	{
		0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
	},
	256   /* every sprite takes 256 consecutive bytes */
};

static const gfx_layout vramlayout_bionicc=
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),   /* 1024 character */
	2,      /* 2 bitplanes */
	{ 4,0 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128   /* every character takes 128 consecutive bytes */
};

// same as the ROM tilemap layout from tigeroad
static const gfx_layout tile_layout =
{
	32, 32,
	RGN_FRAC(1, 2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{
		0, 1, 2, 3, 8 + 0, 8 + 1, 8 + 2, 8 + 3,
		64 * 8 + 0, 64 * 8 + 1, 64 * 8 + 2, 64 * 8 + 3, 64 * 8 + 8 + 0, 64 * 8 + 8 + 1, 64 * 8 + 8 + 2, 64 * 8 + 8 + 3,
		2 * 64 * 8 + 0, 2 * 64 * 8 + 1, 2 * 64 * 8 + 2, 2 * 64 * 8 + 3, 2 * 64 * 8 + 8 + 0, 2 * 64 * 8 + 8 + 1, 2 * 64 * 8 + 8 + 2, 2 * 64 * 8 + 8 + 3,
		3 * 64 * 8 + 0, 3 * 64 * 8 + 1, 3 * 64 * 8 + 2, 3 * 64 * 8 + 3, 3 * 64 * 8 + 8 + 0, 3 * 64 * 8 + 8 + 1, 3 * 64 * 8 + 8 + 2, 3 * 64 * 8 + 8 + 3,
	},
	{
		0 * 16, 1 * 16, 2 * 16, 3 * 16, 4 * 16, 5 * 16, 6 * 16, 7 * 16,
		8 * 16, 9 * 16, 10 * 16, 11 * 16, 12 * 16, 13 * 16, 14 * 16, 15 * 16,
		16 * 16, 17 * 16, 18 * 16, 19 * 16, 20 * 16, 21 * 16, 22 * 16, 23 * 16,
		24 * 16, 25 * 16, 26 * 16, 27 * 16, 28 * 16, 29 * 16, 30 * 16, 31 * 16
	},
	256 * 8
};



static GFXDECODE_START( supduck )
	GFXDECODE_ENTRY( "gfx1", 0, vramlayout_bionicc,    768, 64 )    /* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout,   0,  16 )    /* colors   0-  63 */
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout, 256,  16 )    /* colors 256- 319 */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout_bionicc,  512, 16 )    /* colors 512- 767 */
GFXDECODE_END



void supduck_state::machine_start()
{
	membank("okibank")->configure_entries(0, 4, memregion("okibank")->base(), 0x20000);
	membank("okibank")->set_entry(0);
}

void supduck_state::machine_reset()
{
}


static MACHINE_CONFIG_START( supduck, supduck_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz) /* Verified on PCB */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", supduck_state,  irq2_line_hold) // 2 & 4?

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/4) /* 2MHz - verified on PCB */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(supduck_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", supduck)

	MCFG_DEVICE_ADD("spritegen", TIGEROAD_SPRITE, 0)

	MCFG_PALETTE_ADD("palette", 0x800/2)
	MCFG_PALETTE_FORMAT(xRGBRRRRGGGGBBBB_bit4)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_8MHz/8, OKIM6295_PIN7_HIGH) // 1MHz - Verified on PCB, pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki_map)

MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( supduck )
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "5.u16n", 0x00000, 0x20000, CRC(837a559a) SHA1(ed5ad744a4145dfbef56ad2e6eec3ff14c20de1c) )
	ROM_LOAD16_BYTE( "6.u16l", 0x00001, 0x20000, CRC(508e9905) SHA1(2da3f12caa29066b4d54b22573cfdfcea8916f99) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "4.su6",  0x00000, 0x8000, CRC(d75863ea) SHA1(497d11b86f4f69134943fc3448d195c6e7acbe8f) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "3.cu15",   0x00000, 0x8000, CRC(b1cacca4) SHA1(b4a486618197cf2b85a121b5640cd773b2d453fc) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "7.uu29",   0x00000, 0x20000, CRC(f3251b20) SHA1(8ebb9b98324de14356c9a57ae8a77dc4118fb5c2) )
	ROM_LOAD( "8.uu30",   0x20000, 0x20000, CRC(03c60cbd) SHA1(bf3be7161f69187350eb9d9d4209b93f8b67d0f1) )
	ROM_LOAD( "9.uu31",   0x40000, 0x20000, CRC(9b6d3430) SHA1(ade2decc5bcf817498b1198a2244d1c65bc20bea) )
	ROM_LOAD( "10.uu32",  0x60000, 0x20000, CRC(beed2616) SHA1(c077a3de4a6d451a568694ab70e85830d585a41d) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "11.ul29",   0x00000, 0x20000, CRC(1b6958a4) SHA1(ca93f898702e14ece24d5cfced38d622d3596d0f) )
	ROM_LOAD( "12.ul30",   0x20000, 0x20000, CRC(3e6bd24b) SHA1(f93b5c78d815bd30ecb9cfe2cd257548e467e852) )
	ROM_LOAD( "13.ul31",   0x40000, 0x20000, CRC(bff7b7cd) SHA1(2f65cadcfcc02fe31ba721eea9f45d4a729e4374) )
	ROM_LOAD( "14.ul32",   0x60000, 0x20000, CRC(97a7310b) SHA1(76b82bfea64b59890c0ba2e1688b7321507a4da7) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "15.u1d",   0x60000, 0x20000, CRC(81bf1f27) SHA1(7a66630a2da85387904917d3c136880dffcb9649) )
	ROM_LOAD( "16.u2d",   0x40000, 0x20000, CRC(9573d6ec) SHA1(9923be782bae47c49913d01554bcf3e5efb5395b) )
	ROM_LOAD( "17.u1c",   0x20000, 0x20000, CRC(21ef14d4) SHA1(66e389aaa1186921a07da9a9a9eda88a1083ad42) )
	ROM_LOAD( "18.u2c",   0x00000, 0x20000, CRC(33dd0674) SHA1(b95dfcc16d939bac77f338b8a8cada19328a1993) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "2.su12",   0x00000, 0x20000, CRC(745d42fb) SHA1(f9aee3ddbad3cc2f3a7002ee0d762eb041967e1e) ) // static sample data

	ROM_REGION( 0x80000, "okibank", 0 )
	ROM_LOAD( "1.su13",   0x00000, 0x80000, CRC(7fb1ed42) SHA1(77ec86a6454398e329066aa060e9b6a39085ce71) ) // banked sample data
ROM_END

GAME( 1992, supduck, 0, supduck, supduck, driver_device, 0, ROT0, "Comad", "Super Duck", MACHINE_SUPPORTS_SAVE )
