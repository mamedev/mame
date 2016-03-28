// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Spy Hunter(Tecfri bootleg)
single PCB with 2x Z80

significant changes compared to original HW
non-interlaced

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/z80ctc.h" // not actually present here?

#define MASTER_CLOCK        XTAL_20MHz // ??

class spyhuntertec_state : public driver_device
{
public:
	spyhuntertec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_spyhunt_alpharam(*this, "spyhunt_alpha"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen")
	{ }


	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_spyhunt_alpharam;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spyhuntertec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);


	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;

	UINT8 m_spyhunt_sprite_color_mask;
	INT16 m_spyhunt_scroll_offset;
	INT16 m_spyhunt_scrollx;
	INT16 m_spyhunt_scrolly;

	int mcr_cocktail_flip;

	tilemap_t *m_alpha_tilemap;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(spyhuntertec_paletteram_w);
	DECLARE_DRIVER_INIT(spyhuntertec);
//	DECLARE_VIDEO_START(spyhuntertec);
//	UINT32 screen_update_spyhuntertec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(spyhuntertec_port04_w);
	DECLARE_WRITE8_MEMBER(spyhuntertec_fd00_w);
	
	DECLARE_WRITE8_MEMBER(spyhunt_videoram_w);
	DECLARE_WRITE8_MEMBER(spyhunt_alpharam_w);
	DECLARE_WRITE8_MEMBER(spyhunt_scroll_value_w);

	TILEMAP_MAPPER_MEMBER(spyhunt_bg_scan);
	TILE_GET_INFO_MEMBER(spyhunt_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(spyhunt_get_alpha_tile_info);
	void mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced);


};



WRITE8_MEMBER(spyhuntertec_state::spyhunt_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(spyhuntertec_state::spyhunt_alpharam_w)
{
	m_spyhunt_alpharam[offset] = data;
	m_alpha_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(spyhuntertec_state::spyhunt_scroll_value_w)
{
	switch (offset)
	{
		case 0:
			/* low 8 bits of horizontal scroll */
			m_spyhunt_scrollx = (m_spyhunt_scrollx & ~0xff) | data;
			break;

		case 1:
			/* upper 3 bits of horizontal scroll and upper 1 bit of vertical scroll */
			m_spyhunt_scrollx = (m_spyhunt_scrollx & 0xff) | ((data & 0x07) << 8);
			m_spyhunt_scrolly = (m_spyhunt_scrolly & 0xff) | ((data & 0x80) << 1);
			break;

		case 2:
			/* low 8 bits of vertical scroll */
			m_spyhunt_scrolly = (m_spyhunt_scrolly & ~0xff) | data;
			break;
	}
}


WRITE8_MEMBER(spyhuntertec_state::spyhuntertec_paletteram_w)
{
	m_paletteram[offset] = data;
	offset = (offset & 0x0f) | (offset & 0x60) >> 1;

	int r = (data & 0x07) >> 0;
	int g = (data & 0x38) >> 3;
	int b = (data & 0xc0) >> 6;

	m_palette->set_pen_color(offset^0xf, rgb_t(r<<5,g<<5,b<<6));
}


TILEMAP_MAPPER_MEMBER(spyhuntertec_state::spyhunt_bg_scan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) | ((col & 0x3f) << 4) | ((row & 0x10) << 6);
}


TILE_GET_INFO_MEMBER(spyhuntertec_state::spyhunt_get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int data = videoram[tile_index];
	int code = (data & 0x3f) | ((data >> 1) & 0x40);
	SET_TILE_INFO_MEMBER(0, code, 0, (data & 0x40) ? TILE_FLIPY : 0);
}


TILE_GET_INFO_MEMBER(spyhuntertec_state::spyhunt_get_alpha_tile_info)
{
	SET_TILE_INFO_MEMBER(2, m_spyhunt_alpharam[tile_index], 0, 0);
}



void spyhuntertec_state::video_start()
{	
	/* initialize the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(spyhuntertec_state::spyhunt_get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(spyhuntertec_state::spyhunt_bg_scan),this),  64,16, 64,32);

	/* initialize the text tilemap */
	m_alpha_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(spyhuntertec_state::spyhunt_get_alpha_tile_info),this), TILEMAP_SCAN_COLS,  16,8, 32,32);
	m_alpha_tilemap->set_transparent_pen(0);
	m_alpha_tilemap->set_scrollx(0, 16);

	save_item(NAME(m_spyhunt_sprite_color_mask));
	save_item(NAME(m_spyhunt_scrollx));
	save_item(NAME(m_spyhunt_scrolly));
	save_item(NAME(m_spyhunt_scroll_offset));
}




void spyhuntertec_state::mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced)
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	m_screen->priority().fill(1, cliprect);

	/* loop over sprite RAM */
	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int code, color, flipx, flipy, sx, sy, flags;

		/* skip if zero */
		if (spriteram[offs] == 0)
			continue;

/*
    monoboard:
        flags.d0 -> ICG0~ -> PCG0~/PCG2~/PCG4~/PCG6~ -> bit 4 of linebuffer
        flags.d1 -> ICG1~ -> PCG1~/PCG3~/PCG5~/PCG7~ -> bit 5 of linebuffer
        flags.d2 -> IPPR  -> PPR0 /PPR1 /PPR2 /PPR3  -> bit 6 of linebuffer
        flags.d3 -> IRA15 ----------------------------> address line 15 of FG ROMs
        flags.d4 -> HFLIP
        flags.d5 -> VFLIP

*/

		/* extract the bits of information */
		flags = spriteram[offs + 1];
		code = spriteram[offs + 2] + 256 * ((flags >> 3) & 0x01);
		color = ~flags & color_mask;
		flipx = flags & 0x10;
		flipy = flags & 0x20;
		sx = (spriteram[offs + 3] - 3) * 2;
		sy = (241 - spriteram[offs]);

		if (interlaced == 1) sy *= 2;

		code ^= code_xor;

		sx += dx;
		sy += dy;

		/* sprites use color 0 for background pen and 8 for the 'under tile' pen.
		    The color 8 is used to cover over other sprites. */
		if (!mcr_cocktail_flip)
		{
			/* first draw the sprite, visible */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, flipx, flipy, sx, sy,
					screen.priority(), 0x00, 0x0101);

			/* then draw the mask, behind the background but obscuring following sprites */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, flipx, flipy, sx, sy,
					screen.priority(), 0x02, 0xfeff);
		}
		else
		{
			/* first draw the sprite, visible */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, !flipx, !flipy, 480 - sx, 452 - sy,
					screen.priority(), 0x00, 0x0101);

			/* then draw the mask, behind the background but obscuring following sprites */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, !flipx, !flipy, 480 - sx, 452 - sy,
					screen.priority(), 0x02, 0xfeff);
		}
	}
}


UINT32 spyhuntertec_state::screen_update_spyhuntertec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	m_bg_tilemap->set_scrollx(0, m_spyhunt_scrollx * 2 + m_spyhunt_scroll_offset);
	m_bg_tilemap->set_scrolly(0, m_spyhunt_scrolly * 2);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	mcr3_update_sprites(screen, bitmap, cliprect, m_spyhunt_sprite_color_mask, 0, -12, 0, 0);

	/* render any characters on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}



WRITE8_MEMBER(spyhuntertec_state::spyhuntertec_fd00_w)
{
}

static ADDRESS_MAP_START( spyhuntertec_map, AS_PROGRAM, 8, spyhuntertec_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xa800, 0xa8ff) AM_RAM // the ROM is a solid fill in these areas, and they get tested as RAM, I think they moved the 'real' scroll regs here
	AM_RANGE(0xa900, 0xa9ff) AM_RAM

	AM_RANGE(0x0000, 0xdfff) AM_ROM

	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(spyhunt_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xebff) AM_MIRROR(0x0400) AM_RAM_WRITE(spyhunt_alpharam_w) AM_SHARE("spyhunt_alpha")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM //AM_SHARE("nvram")
	AM_RANGE(0xf800, 0xf9ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xfa00, 0xfa7f) AM_MIRROR(0x0180) AM_RAM_WRITE(spyhuntertec_paletteram_w) AM_SHARE("paletteram")

	AM_RANGE(0xfc00, 0xfc00) AM_READ_PORT("DSW0")
	AM_RANGE(0xfc01, 0xfc01) AM_READ_PORT("DSW1")
	AM_RANGE(0xfc02, 0xfc02) AM_READ_PORT("IN2")
	AM_RANGE(0xfc03, 0xfc03) AM_READ_PORT("IN3")

	AM_RANGE(0xfd00, 0xfd00) AM_WRITE( spyhuntertec_fd00_w )

	AM_RANGE(0xfe00, 0xffff) AM_RAM // a modified copy of spriteram for this hw??
ADDRESS_MAP_END

WRITE8_MEMBER(spyhuntertec_state::spyhuntertec_port04_w)
{
}

static ADDRESS_MAP_START( spyhuntertec_portmap, AS_IO, 8, spyhuntertec_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x04) AM_WRITE(spyhuntertec_port04_w)
	AM_RANGE(0x84, 0x86) AM_WRITE(spyhunt_scroll_value_w)
	AM_RANGE(0xe0, 0xe0) AM_WRITENOP // was watchdog
//  AM_RANGE(0xe8, 0xe8) AM_WRITENOP
//	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( spyhuntertec_sound_map, AS_PROGRAM, 8, spyhuntertec_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
//  AM_RANGE(0xfe00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( spyhuntertec_sound_portmap, AS_IO, 8, spyhuntertec_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	AM_RANGE(0x12, 0x13) AM_DEVWRITE("ay1", ay8912_device, address_data_w)
	AM_RANGE(0x14, 0x15) AM_DEVWRITE("ay2", ay8912_device, address_data_w)
	AM_RANGE(0x18, 0x19) AM_DEVWRITE("ay3", ay8912_device, address_data_w)

ADDRESS_MAP_END



static INPUT_PORTS_START( spyhuntertec )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DSW0-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW0-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW0-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW0-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW0-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW0-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW0-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW0-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW1-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "DSW1-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW1-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW1-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW1-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "start" ) // start
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "handbrake?" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "pedal inverse" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x0001, 0x0001, "3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "coin" ) // coin?
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "machineguns" ) // machine guns
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout spyhuntertec_alphalayout =
{
	16,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4},
	{ 0, 0, 1, 1, 2, 2, 3, 3, 8, 8, 9, 9, 10, 10, 11, 11 },
	{ 0, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


const gfx_layout spyhuntertec_sprite_layout =
{
	32,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 6,7,  4,5,  2,3,  0,1,  14,15,  12,13,  10,11,  8,9,    22,23, 20,21,  18,19,  16,17,  30,31,  28,29,  26,27,  24,25 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32   },

	16*32
};


static const UINT32 spyhuntp_charlayout_xoffset[64] =
{
		0x0000*8,0x0000*8,   0x0000*8+1,0x0000*8+1,   0x0000*8+2,0x0000*8+2,   0x0000*8+3,0x0000*8+3,   0x0000*8+4,0x0000*8+4,   0x0000*8+5,0x0000*8+5,   0x0000*8+6,0x0000*8+6,   0x0000*8+7,0x0000*8+7,
		0x1000*8,0x1000*8,   0x1000*8+1,0x1000*8+1,   0x1000*8+2,0x1000*8+2,   0x1000*8+3,0x1000*8+3,   0x1000*8+4,0x1000*8+4,   0x1000*8+5,0x1000*8+5,   0x1000*8+6,0x1000*8+6,   0x1000*8+7,0x1000*8+7,
		0x2000*8,0x2000*8,   0x2000*8+1,0x2000*8+1,   0x2000*8+2,0x2000*8+2,   0x2000*8+3,0x2000*8+3,   0x2000*8+4,0x2000*8+4,   0x2000*8+5,0x2000*8+5,   0x2000*8+6,0x2000*8+6,   0x2000*8+7,0x2000*8+7,
		0x3000*8,0x3000*8,   0x3000*8+1,0x3000*8+1,   0x3000*8+2,0x3000*8+2,   0x3000*8+3,0x3000*8+3,   0x3000*8+4,0x3000*8+4,   0x3000*8+5,0x3000*8+5,   0x3000*8+6,0x3000*8+6,   0x3000*8+7,0x3000*8+7,
};


static const gfx_layout spyhuntertec_charlayout =
{
	64,16,
	RGN_FRAC(1,8),
	4,
	{  0*8,  0x4000*8 + 2*8, 0x4000*8 + 0*8, 2*8  },
	EXTENDED_XOFFS,
	{ 0*8,  4*8,  8*8,  12*8,    16*8,  20*8,  24*8,  28*8,     1*8,  5*8, 9*8, 13*8,    17*8,  21*8,  25*8,  29*8    },
	32*8,
	spyhuntp_charlayout_xoffset,
	nullptr
};


static GFXDECODE_START( spyhuntertec )
	GFXDECODE_ENTRY( "gfx1", 0, spyhuntertec_charlayout,  3*16, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spyhuntertec_sprite_layout,   0*16, 4 )
	GFXDECODE_ENTRY( "gfx3", 0, spyhuntertec_alphalayout, 4*16, 1 )
GFXDECODE_END



void spyhuntertec_state::machine_start()
{
}

void spyhuntertec_state::machine_reset()
{
}




static MACHINE_CONFIG_START( spyhuntertec, spyhuntertec_state )

// note: no ctc, no nvram
// 2*z80, 3*ay8912

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(spyhuntertec_map)
	MCFG_CPU_IO_MAP(spyhuntertec_portmap)
//	MCFG_CPU_CONFIG(mcr_daisy_chain)
//	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", spyhuntertec_state, mcr_interrupt, "screen", 0, 1)

//	MCFG_DEVICE_ADD("ctc", Z80CTC, MASTER_CLOCK/4 /* same as "maincpu" */)
//	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
//	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("ctc", z80ctc_device, trg1))

	//MCFG_WATCHDOG_VBLANK_INIT(16)
//	MCFG_MACHINE_START_OVERRIDE(spyhuntertec_state,mcr)
//	MCFG_MACHINE_RESET_OVERRIDE(spyhuntertec_state,mcr)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(30*16, 30*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 30*16-1, 0, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(spyhuntertec_state, screen_update_spyhuntertec)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", spyhuntertec)
	MCFG_PALETTE_ADD("palette", 64+4)

//	MCFG_PALETTE_INIT_OWNER(spyhuntertec_state,spyhunt)


	MCFG_CPU_ADD("audiocpu", Z80, 3000000 )
	MCFG_CPU_PROGRAM_MAP(spyhuntertec_sound_map)
	MCFG_CPU_IO_MAP(spyhuntertec_sound_portmap)
//  MCFG_CPU_PERIODIC_INT_DRIVER(spyhuntertec_state, irq0_line_hold, 4*60)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8912, 3000000/2) // AY-3-8912
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ay2", AY8912, 3000000/2) // "
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ay3", AY8912, 3000000/2) // "
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_CONFIG_END




ROM_START( spyhuntpr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x4000, CRC(2a2f77cb) SHA1(e1b74c951efb2a49bef0507ab3268b274515f339) )
	ROM_LOAD( "2.bin",   0x4000, 0x4000, CRC(00778aff) SHA1(7c0b24c393f841e8379d4bba57ba502e3d2512f9) )
	ROM_LOAD( "3.bin",   0x8000, 0x4000, CRC(2183b4af) SHA1(2b958afc40b26c9bc8d5254b0600426649f4ebf0) )
	ROM_LOAD( "4.bin",   0xc000, 0x2000, CRC(3ea6a65c) SHA1(1320ce17044307ed3c4f2459631a9aa1734f1f30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x2000, CRC(33fe2829) SHA1(e6950dbf681242bf23542ca6604e62eacb431101) )


	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "6.bin",   0x0000, 0x200, CRC(6b76f46a) SHA1(4b398084c42a60fcfa4a9bf14f844e36a3f42723) )
	ROM_CONTINUE(0x0001, 0x200)
	ROM_CONTINUE(0x0800, 0x200)
	ROM_CONTINUE(0x0801, 0x200)
	ROM_CONTINUE(0x1000, 0x200)
	ROM_CONTINUE(0x1001, 0x200)
	ROM_CONTINUE(0x1800, 0x200)
	ROM_CONTINUE(0x1801, 0x200)
	ROM_CONTINUE(0x2000, 0x200)
	ROM_CONTINUE(0x2001, 0x200)
	ROM_CONTINUE(0x2800, 0x200)
	ROM_CONTINUE(0x2801, 0x200)
	ROM_CONTINUE(0x3000, 0x200)
	ROM_CONTINUE(0x3001, 0x200)
	ROM_CONTINUE(0x3800, 0x200)
	ROM_CONTINUE(0x3801, 0x200)
	ROM_LOAD32_BYTE( "7.bin",   0x0002, 0x200, CRC(085bd7a7) SHA1(c35c309b6c6485baec54d4434dea44abf4d48f41) )
	ROM_CONTINUE(0x0003, 0x200)
	ROM_CONTINUE(0x0802, 0x200)
	ROM_CONTINUE(0x0803, 0x200)
	ROM_CONTINUE(0x1002, 0x200)
	ROM_CONTINUE(0x1003, 0x200)
	ROM_CONTINUE(0x1802, 0x200)
	ROM_CONTINUE(0x1803, 0x200)
	ROM_CONTINUE(0x2002, 0x200)
	ROM_CONTINUE(0x2003, 0x200)
	ROM_CONTINUE(0x2802, 0x200)
	ROM_CONTINUE(0x2803, 0x200)
	ROM_CONTINUE(0x3002, 0x200)
	ROM_CONTINUE(0x3003, 0x200)
	ROM_CONTINUE(0x3802, 0x200)
	ROM_CONTINUE(0x3803, 0x200)
	ROM_LOAD32_BYTE( "8.bin",   0x4000, 0x200, CRC(e699b329) SHA1(cb4b8c7b6fa1cb1144a18f1442dc3b267c408914) )
	ROM_CONTINUE(0x4001, 0x200)
	ROM_CONTINUE(0x4800, 0x200)
	ROM_CONTINUE(0x4801, 0x200)
	ROM_CONTINUE(0x5000, 0x200)
	ROM_CONTINUE(0x5001, 0x200)
	ROM_CONTINUE(0x5800, 0x200)
	ROM_CONTINUE(0x5801, 0x200)
	ROM_CONTINUE(0x6000, 0x200)
	ROM_CONTINUE(0x6001, 0x200)
	ROM_CONTINUE(0x6800, 0x200)
	ROM_CONTINUE(0x6801, 0x200)
	ROM_CONTINUE(0x7000, 0x200)
	ROM_CONTINUE(0x7001, 0x200)
	ROM_CONTINUE(0x7800, 0x200)
	ROM_CONTINUE(0x7801, 0x200)
	ROM_LOAD32_BYTE( "9.bin",   0x4002, 0x200, CRC(6d462ec7) SHA1(0ff37f75b0eeceb86177a3f7c93834d5c0e24515) )
	ROM_CONTINUE(0x4003, 0x200)
	ROM_CONTINUE(0x4802, 0x200)
	ROM_CONTINUE(0x4803, 0x200)
	ROM_CONTINUE(0x5002, 0x200)
	ROM_CONTINUE(0x5003, 0x200)
	ROM_CONTINUE(0x5802, 0x200)
	ROM_CONTINUE(0x5803, 0x200)
	ROM_CONTINUE(0x6002, 0x200)
	ROM_CONTINUE(0x6003, 0x200)
	ROM_CONTINUE(0x6802, 0x200)
	ROM_CONTINUE(0x6803, 0x200)
	ROM_CONTINUE(0x7002, 0x200)
	ROM_CONTINUE(0x7003, 0x200)
	ROM_CONTINUE(0x7802, 0x200)
	ROM_CONTINUE(0x7803, 0x200)

	ROM_REGION( 0x10000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "10.bin",   0x00000, 0x4000, CRC(6f9fd416) SHA1(a51c86e5b22c91fc44673f53400b58af40b18065) )
	ROM_LOAD( "11.bin",   0x04000, 0x4000, CRC(75526ffe) SHA1(ff1adf6f9b6595114d0bd06b72d9eb7bbf70144d) )
	ROM_LOAD( "12.bin",   0x08000, 0x4000, CRC(82ee7a4d) SHA1(184720de76680275bf7c4a171f03a0ce771d91fc) )
	ROM_LOAD( "13.bin",   0x0c000, 0x4000, CRC(0cc592a3) SHA1(b3563bde83432cdbaedb88d4d222da30bf679b08) )


	ROM_REGION( 0x01000, "gfx3", 0 )
	ROM_LOAD( "14.bin",  0x00000, 0x1000, CRC(87a4c130) SHA1(7792afdc36b0f3bd51c387d04d38f60c85fd2e93) )
ROM_END

DRIVER_INIT_MEMBER(spyhuntertec_state,spyhuntertec)
{
//	mcr_common_init();
//  machine().device<midway_ssio_device>("ssio")->set_custom_input(1, 0x60, read8_delegate(FUNC(spyhuntertec_state::spyhunt_ip1_r),this));
//  machine().device<midway_ssio_device>("ssio")->set_custom_input(2, 0xff, read8_delegate(FUNC(spyhuntertec_state::spyhunt_ip2_r),this));
//  machine().device<midway_ssio_device>("ssio")->set_custom_output(4, 0xff, write8_delegate(FUNC(spyhuntertec_state::spyhunt_op4_w),this));

	m_spyhunt_sprite_color_mask = 0x00;
	m_spyhunt_scroll_offset = 16;
}


// very different hardware, probably bootleg despite the license text printed on the PCB, similar to '1942p' in 1942.c.  Probably should be put in separate driver.
// PCB made by Tecfri for Recreativos Franco S.A. in Spain, has Bally Midway logo, and licensing text on the PCB.  Board is dated '85' so seems to be a low-cost rebuild? it is unclear if it made it to market.
GAME (1983, spyhuntpr,spyhunt,  spyhuntertec, spyhuntertec,spyhuntertec_state,  spyhuntertec,ROT90, "Bally Midway (Recreativos Franco S.A. license)", "Spy Hunter (Spain, Tecfri / Recreativos Franco S.A. PCB)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
