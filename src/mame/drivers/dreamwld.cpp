// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    SemiCom 68020 based hardware
    Driver by David Haywood

Baryon - Future Assault          (c) 1997 SemiCom / Tirano
Cute Fighter                     (c) 1998 SemiCom
Rolling Crush                    (c) 1999 Trust / SemiCom
Gaia - The Last Choice of Earth  (c) 1999 SemiCom / XESS
Dream World                      (c) 2000 SemiCom

Note: There is a SemiCom game known as Lode Quest 1998(?). This game is very similar to Dream World.
      It's not known if Lode Quest is a alternate title or a prequel of Dream World.

Note: this hardware is a copy of Psikyo's 68020 based hardware,
      the Strikers 1945 bootleg has the same unknown rom!

      It isn't quite as flexible as the original Psikyo hardware
      by the looks of it, there are various subtle changes to how
      things work, for example the tilemap sizes and missing
      transparent pen modification.  This makes it rather hard to
      merge with psikyo.c and it should probably be left separate.


Stephh's notes (based on the game M68EC020 code and some tests) :

  - Don't trust the "test mode" as it displays Dip Switches infos
    that are in fact unused by the game ! Leftover from another game ?

    PORT_START("DSW")
    PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
    PORT_DIPSETTING(      0x0002, "1" )
    PORT_DIPSETTING(      0x0003, "2" )
    PORT_DIPSETTING(      0x0001, "3" )
    PORT_DIPSETTING(      0x0000, "4" )
    PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
    PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
    PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
    PORT_DIPNAME( 0x0060, 0x0060, "Ticket Payout" )         PORT_DIPLOCATION("SW2:6,7")
    PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
    PORT_DIPSETTING(      0x0020, "Little" )
    PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
    PORT_DIPSETTING(      0x0040, "Much" )
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
    PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
    PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
    PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
    PORT_DIPSETTING(      0x2000, "Level 1" )
    PORT_DIPSETTING(      0x1000, "Level 2" )
    PORT_DIPSETTING(      0x0000, "Level 3" )
    PORT_DIPSETTING(      0x7000, "Level 4" )
    PORT_DIPSETTING(      0x6000, "Level 5" )
    PORT_DIPSETTING(      0x5000, "Level 6" )
    PORT_DIPSETTING(      0x4000, "Level 7" )
    PORT_DIPSETTING(      0x3000, "Level 8" )
    PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )


   note:

   Baryon has some annoying sound looping clicks / cutouts, these need to
    be verified against the HW (it's a very cheap sound system, so it might
    be accurate)

   Baryon has playfield background which fade in with very rough / visible
    edges.  In this case the tilemap size registers from the original
    psikyo hardware are set to be the alternate tilemap size, however that
    doesn't make sense in the context of the data in RAM, which doesn't
    appear to wrap properly anyway, again, it's likely this is just how the
    game is.  Furthermore the BG test in the test menu indicates that it
    tests alternate tilemap sizes, but doesn't even write to the register,
    probably a leftover from hardware development as the test menu is mostly
    incomplete.

   All: sprite priority, the original psikyo.c HW has sprite<->tilemap
    priority but we don't support it here, does the clone HW support it?

   All: sprite zooming, again the original psikyo.c HW supports this, but we
    don't support it here.  The Strikers 1945 bootleg in psikyo.c doesn't
    appear to support it properly either, so it might be missing on these
    clone boards.

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"


class dreamwld_state : public driver_device
{
public:
	dreamwld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg2_videoram(*this, "bg2_videoram"),
		m_vregs(*this, "vregs"),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* memory pointers */
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_bg_videoram;
	required_shared_ptr<UINT32> m_bg2_videoram;
	required_shared_ptr<UINT32> m_vregs;
	required_shared_ptr<UINT32> m_workram;

	UINT16* m_lineram16;

	DECLARE_READ16_MEMBER(lineram16_r) { return m_lineram16[offset]; }
	DECLARE_WRITE16_MEMBER(lineram16_w) { COMBINE_DATA(&m_lineram16[offset]); }

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_bg2_tilemap;
	int      m_tilebank[2];
	int      m_tilebankold[2];

	std::unique_ptr<UINT32[]> m_spritebuf1;
	std::unique_ptr<UINT32[]> m_spritebuf2;

	/* misc */
	int      m_protindex;
	DECLARE_WRITE32_MEMBER(dreamwld_bg_videoram_w);
	DECLARE_WRITE32_MEMBER(dreamwld_bg2_videoram_w);
	DECLARE_READ32_MEMBER(dreamwld_protdata_r);
	DECLARE_WRITE32_MEMBER(dreamwld_6295_0_bank_w);
	DECLARE_WRITE32_MEMBER(dreamwld_6295_1_bank_w);
	TILE_GET_INFO_MEMBER(get_dreamwld_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_dreamwld_bg2_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_dreamwld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_dreamwld(screen_device &screen, bool state);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};



void dreamwld_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	UINT32 *source = m_spritebuf1.get();
	UINT32 *finish = m_spritebuf1.get() + 0x1000 / 4;
	UINT16 *redirect = (UINT16 *)memregion("spritelut")->base();
	int xoffset = 4;

	while (source < finish)
	{
		int xpos, ypos, tileno;
		int xsize, ysize, xinc, yinc;
		int xct, yct;
		int xflip;
		int yflip;
		int colour;

		xpos  = (source[0] & 0x000001ff) >> 0;
		ypos  = (source[0] & 0x01ff0000) >> 16;
		xsize = (source[0] & 0x00000e00) >> 9;
		ysize = (source[0] & 0x0e000000) >> 25;

		tileno = (source[1] & 0x0001ffff) >>0;
		colour = (source[1] & 0x3f000000) >>24;
		xflip  = (source[1] & 0x40000000);
		yflip  = (source[1] & 0x80000000);

		xinc = 16;
		yinc = 16;

		xpos += xoffset;
		xpos &= 0x1ff;

		if (xflip)
		{
			xinc = -16;
			xpos += 16 * xsize;
		}

		if (yflip)
		{
			yinc = -16;
			ypos += 16 * ysize;
		}

		ysize++; xsize++; // size 0 = 1 tile

		xpos -=16;


		for (yct = 0; yct < ysize; yct++)
		{
			for (xct = 0; xct < xsize; xct++)
			{
					gfx->transpen(bitmap,cliprect, redirect[tileno], colour, xflip, yflip, xpos + xct * xinc, ypos + yct * yinc, 0);
					gfx->transpen(bitmap,cliprect, redirect[tileno], colour, xflip, yflip, (xpos + xct * xinc) - 0x200, ypos + yct * yinc, 0);
					gfx->transpen(bitmap,cliprect, redirect[tileno], colour, xflip, yflip, (xpos + xct * xinc) - 0x200, (ypos + yct * yinc) - 0x200, 0);
					gfx->transpen(bitmap,cliprect, redirect[tileno], colour, xflip, yflip, xpos + xct * xinc, (ypos + yct * yinc) - 0x200 , 0);

				tileno++;
			}
		}

		source += 2;
	}
}


WRITE32_MEMBER(dreamwld_state::dreamwld_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset * 2);
	m_bg_tilemap->mark_tile_dirty(offset * 2 + 1);
}

TILE_GET_INFO_MEMBER(dreamwld_state::get_dreamwld_bg_tile_info)
{
	int tileno, colour;
	tileno = (tile_index & 1) ? (m_bg_videoram[tile_index >> 1] & 0xffff) : ((m_bg_videoram[tile_index >> 1] >> 16) & 0xffff);
	colour = tileno >> 13;
	tileno &= 0x1fff;
	SET_TILE_INFO_MEMBER(1, tileno + m_tilebank[0] * 0x2000, 0x80 + colour, 0);
}


WRITE32_MEMBER(dreamwld_state::dreamwld_bg2_videoram_w)
{
	COMBINE_DATA(&m_bg2_videoram[offset]);
	m_bg2_tilemap->mark_tile_dirty(offset * 2);
	m_bg2_tilemap->mark_tile_dirty(offset * 2 + 1);
}

TILE_GET_INFO_MEMBER(dreamwld_state::get_dreamwld_bg2_tile_info)
{
	UINT16 tileno, colour;
	tileno = (tile_index & 1) ? (m_bg2_videoram[tile_index >> 1] & 0xffff) : ((m_bg2_videoram[tile_index >> 1] >> 16) & 0xffff);
	colour = tileno >> 13;
	tileno &= 0x1fff;
	SET_TILE_INFO_MEMBER(1, tileno + m_tilebank[1] * 0x2000, 0xc0 + colour, 0);
}

void dreamwld_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dreamwld_state::get_dreamwld_bg_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16, 64,64);
	m_bg2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dreamwld_state::get_dreamwld_bg2_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16, 64,64);
	m_bg2_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scroll_rows(64*16); // line scrolling
	m_bg_tilemap->set_scroll_cols(1);

	m_bg2_tilemap->set_scroll_rows(64*16);    // line scrolling
	m_bg2_tilemap->set_scroll_cols(1);

	m_spritebuf1 = std::make_unique<UINT32[]>(0x2000 / 4);
	m_spritebuf2 = std::make_unique<UINT32[]>(0x2000 / 4);

	m_lineram16 = (UINT16*)auto_alloc_array_clear(this->machine(), UINT16, 0x400 / 2);
	save_pointer(NAME(m_lineram16), 0x400/2);

}

void dreamwld_state::screen_eof_dreamwld(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_spritebuf2.get(), m_spritebuf1.get(), 0x2000);
		memcpy(m_spritebuf1.get(), m_spriteram, 0x2000);
	}
}


UINT32 dreamwld_state::screen_update_dreamwld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  int tm0size, tm1size;

	tilemap_t *tmptilemap0, *tmptilemap1;

	tmptilemap0 = m_bg_tilemap;
	tmptilemap1 = m_bg2_tilemap;

	int layer0_scrolly = m_vregs[(0x000 / 4)]+32;
	int layer1_scrolly = m_vregs[(0x008 / 4)]+32;

	int layer0_scrollx = m_vregs[(0x004 / 4)] + 0;
	int layer1_scrollx = m_vregs[(0x00c / 4)] + 2;

	UINT32 layer0_ctrl = m_vregs[0x010 / 4];
	UINT32 layer1_ctrl = m_vregs[0x014 / 4];

	m_tilebank[0] = (layer0_ctrl >> 6) & 1;
	m_tilebank[1] = (layer1_ctrl >> 6) & 1;

	if (m_tilebank[0] != m_tilebankold[0])
	{
		m_tilebankold[0] = m_tilebank[0];
		m_bg_tilemap->mark_all_dirty();
	}

	if (m_tilebank[1] != m_tilebankold[1])
	{
		m_tilebankold[1] = m_tilebank[1];
		m_bg2_tilemap->mark_all_dirty();
	}



	tmptilemap0->set_scrolly(0, layer0_scrolly);
	tmptilemap1->set_scrolly(0, layer1_scrolly);

	// not on this hw?
#if 0
	switch ((layer0_ctrl & 0x00c0) >> 6)
	{
	case 0: tm0size = 1;    break;
	case 1: tm0size = 2;    break;
	case 2: tm0size = 3;    break;
	default:    tm0size = 0;    break;
	}

	switch ((layer1_ctrl & 0x00c0) >> 6)
	{
	case 0: tm1size = 1;    break;
	case 1: tm1size = 2;    break;
	case 2: tm1size = 3;    break;
	default:    tm1size = 0;    break;
	}
#endif
	//popmessage("sizes %d %d\n", tm0size, tm1size);

	for (int i = 0; i < 256; i++)   /* 256 screen lines */
	{
		int x0 = 0, x1 = 0;

		UINT16* linebase;



		/* layer 0 */
		linebase = &m_lineram16[0x000];

		if (layer0_ctrl & 0x0300)
		{
			if (layer0_ctrl & 0x0200)
				/* per-tile rowscroll */
				x0 = linebase[((i+32)&0xff)/16];
			else
				/* per-line rowscroll */
				x0 = linebase[(i+32)&0xff];
		}

		tmptilemap0->set_scrollx(
		(i + layer0_scrolly) & 0x3ff,
		layer0_scrollx + x0 );


		/* layer 1 */
		linebase = &m_lineram16[0x200/2];

		if (layer1_ctrl & 0x0300)
		{
			if (layer1_ctrl & 0x0200)
				/* per-tile rowscroll */
				x1 = linebase[((i+32)&0xff)/16];
			else
				/* per-line rowscroll */
				x1 = linebase[(i+32)&0xff];
		}


		tmptilemap1->set_scrollx(
		(i + layer1_scrolly) & 0x3ff,
		layer1_scrollx + x1 );

	}



	tmptilemap0->draw(screen, bitmap, cliprect, 0, 0);
	tmptilemap1->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	return 0;
}



READ32_MEMBER(dreamwld_state::dreamwld_protdata_r)
{
	//static int count = 0;

	UINT8 *protdata = memregion("user1")->base();
	size_t protsize = memregion("user1")->bytes();
	UINT8 dat = protdata[(m_protindex++) % protsize];

	//printf("protection read %04x %02x\n", count, dat);
	//count++;

	// real hw returns 00 after end of data, I haven't checked if it's possible to overflow the read counter
	// and read out the internal rom.

	return dat << 24;
}

static ADDRESS_MAP_START( oki1_map, AS_0, 8, dreamwld_state )
	AM_RANGE(0x00000, 0x2ffff) AM_ROM
	AM_RANGE(0x30000, 0x3ffff) AM_ROMBANK("oki1bank")
ADDRESS_MAP_END

static ADDRESS_MAP_START( oki2_map, AS_0, 8, dreamwld_state )
	AM_RANGE(0x00000, 0x2ffff) AM_ROM
	AM_RANGE(0x30000, 0x3ffff) AM_ROMBANK("oki2bank")
ADDRESS_MAP_END

WRITE32_MEMBER(dreamwld_state::dreamwld_6295_0_bank_w)
{
	if (ACCESSING_BITS_0_7)
		membank("oki1bank")->set_entry(data&3);
	else
		logerror("OKI0: unk bank write %x mem_mask %8x\n", data, mem_mask);
}

WRITE32_MEMBER(dreamwld_state::dreamwld_6295_1_bank_w)
{
	if (ACCESSING_BITS_0_7)
		membank("oki2bank")->set_entry(data&3);
	else
		logerror("OKI1: unk bank write %x mem_mask %8x\n", data, mem_mask);
}


static ADDRESS_MAP_START( baryon_map, AS_PROGRAM, 32, dreamwld_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM  AM_WRITENOP

	AM_RANGE(0x400000, 0x401fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x600000, 0x601fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x800000, 0x801fff) AM_RAM_WRITE(dreamwld_bg_videoram_w ) AM_SHARE("bg_videoram")
	AM_RANGE(0x802000, 0x803fff) AM_RAM_WRITE(dreamwld_bg2_videoram_w ) AM_SHARE("bg2_videoram")
	AM_RANGE(0x804000, 0x8043ff) AM_READWRITE16(lineram16_r, lineram16_w, 0xffffffff)  // linescroll
	AM_RANGE(0x804400, 0x805fff) AM_RAM AM_SHARE("vregs")

	AM_RANGE(0xc00000, 0xc00003) AM_READ_PORT("INPUTS")
	AM_RANGE(0xc00004, 0xc00007) AM_READ_PORT("c00004")

	AM_RANGE(0xc0000c, 0xc0000f) AM_WRITE(dreamwld_6295_0_bank_w) // sfx
	AM_RANGE(0xc00018, 0xc0001b) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0xff000000) // sfx

	AM_RANGE(0xc00030, 0xc00033) AM_READ(dreamwld_protdata_r) // it reads protection data (irq code) from here and puts it at ffd000

	AM_RANGE(0xfe0000, 0xffffff) AM_RAM AM_SHARE("workram") // work ram
ADDRESS_MAP_END

static ADDRESS_MAP_START( dreamwld_map, AS_PROGRAM, 32, dreamwld_state )
	AM_IMPORT_FROM( baryon_map )

	AM_RANGE(0xc0002c, 0xc0002f) AM_WRITE(dreamwld_6295_1_bank_w) // sfx
	AM_RANGE(0xc00028, 0xc0002b) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0xff000000) // sfx
ADDRESS_MAP_END


static INPUT_PORTS_START( dreamwld )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* "Book" (when you get one of them) */
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* "Jump" */
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* "Dig" */
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) /* "Book" (when you get one of them) */
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* "Jump" */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* "Dig" */
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)

	PORT_START("c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" ) /* see notes - "Ticket Payout" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" ) /* see notes - "Ticket Payout" */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")     /* gives in fact 99 credits */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW1:1" ) /* see notes - "Demo Sounds" */
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW1:5" ) /* see notes - "Difficulty" */
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW1:6" ) /* see notes - "Difficulty" */
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW1:7" ) /* see notes - "Difficulty" */
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( baryon )
	PORT_INCLUDE(dreamwld)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0004, 0x0004, "Bomb Stock" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rolcrush )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)

	PORT_START("c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW2:1" ) /* As listed in service mode, but tested */
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW2:2" ) /* These might have some use, requires investigation of code */
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x2000, "Level 1" )
	PORT_DIPSETTING(      0x1000, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x7000, "Level 4" )
	PORT_DIPSETTING(      0x6000, "Level 5" )
	PORT_DIPSETTING(      0x5000, "Level 6" )
	PORT_DIPSETTING(      0x4000, "Level 7" )
	PORT_DIPSETTING(      0x3000, "Level 8" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( cutefght )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)

	PORT_START("c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW2:1" ) /* As listed in service mode, but tested */
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW2:2" ) /* These might have some use, requires investigation of code */
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x0060, 0x0060, "Ticket Payout" )         PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, "Little" )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, "Much" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1") /* Has no effect?? */
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x2000, "Level 1" )
	PORT_DIPSETTING(      0x1000, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x7000, "Level 4" )
	PORT_DIPSETTING(      0x6000, "Level 5" )
	PORT_DIPSETTING(      0x5000, "Level 6" )
	PORT_DIPSETTING(      0x4000, "Level 7" )
	PORT_DIPSETTING(      0x3000, "Level 8" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( gaialast )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)

	PORT_START("c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003,  0x0001, DEF_STR( Lives )  )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(       0x0002, "1" )
	PORT_DIPSETTING(       0x0003, "2" )
	PORT_DIPSETTING(       0x0001, "3" )
	PORT_DIPSETTING(       0x0000, "4" )
	PORT_DIPNAME( 0x0004,  0x0000, "Bomb Stock" )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(       0x0004, "2" )
	PORT_DIPSETTING(       0x0000, "3" )
	PORT_DIPNAME( 0x0008,  0x0000, "Lock Vertical Scroll" )  PORT_DIPLOCATION("SW2:4") // if Off the game pans the screen up/down when you move up/down
	PORT_DIPSETTING(       0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" ) // these are listed as ticket payout in test mode, but I believe
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" ) // that to be a leftover from some redemption game
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1") /* Title screen sounds only */
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x2000, "Level 1" ) // listed as 'Level 3' in service mode
	PORT_DIPSETTING(      0x1000, "Level 2" ) // ^^
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x7000, "Level 4" )
	PORT_DIPSETTING(      0x6000, "Level 5" )
	PORT_DIPSETTING(      0x5000, "Level 6" )
	PORT_DIPSETTING(      0x4000, "Level 7" )
	PORT_DIPSETTING(      0x3000, "Level 8" ) // listed as 'Little' in service mode, but still clearly the most difficult
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{0,1,2,3},
	{2*4,3*4,0*4,1*4,6*4,7*4,4*4,5*4,
		10*4,11*4,8*4,9*4,14*4,15*4,12*4,13*4},
	{0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,
		8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64},
	16*16*4
};

static GFXDECODE_START( dreamwld )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4, 0x000, 0x100 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4, 0x000, 0x100 ) // [1] Layer 0 + 1
GFXDECODE_END


void dreamwld_state::machine_start()
{
	if (subdevice("oki1"))
	{
		membank("oki1bank")->configure_entries(0, 4, memregion("oki1")->base()+0x30000, 0x10000);
		membank("oki1bank")->set_entry(0);
	}

	if (subdevice("oki2"))
	{
		membank("oki2bank")->configure_entries(0, 4, memregion("oki2")->base()+0x30000, 0x10000);
		membank("oki2bank")->set_entry(0);
	}

	save_item(NAME(m_protindex));
	save_item(NAME(m_tilebank));
	save_item(NAME(m_tilebankold));
}

void dreamwld_state::machine_reset()
{
	m_tilebankold[0] = m_tilebankold[1] = -1;
	m_tilebank[0] = m_tilebank[1] = 0;
	m_protindex = 0;
}


static MACHINE_CONFIG_START( baryon, dreamwld_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, XTAL_32MHz/2) /* 16MHz verified */
	MCFG_CPU_PROGRAM_MAP(baryon_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dreamwld_state,  irq4_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57.793)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(512,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 308-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(dreamwld_state, screen_update_dreamwld)
	MCFG_SCREEN_VBLANK_DRIVER(dreamwld_state, screen_eof_dreamwld)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dreamwld)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", XTAL_32MHz/32, OKIM6295_PIN7_LOW) /* 1MHz verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dreamwld, baryon )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dreamwld_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dreamwld_state,  irq4_line_hold)

	MCFG_OKIM6295_ADD("oki2", XTAL_32MHz/32, OKIM6295_PIN7_LOW) /* 1MHz verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)

MACHINE_CONFIG_END


/*

Baryon
SemiCom, 1997

PCB Layout
----------

|-------------------------------------------------|
|        1SEMICOM  62256   ACTEL        8SEMICOM  |
|VOL        M6295  62256   A1020B                 |
|    PAL  PAL              32MHz                  |
|    62256  62256             PAL                 |
| 2SEMICOM 4SEMICOM 68EC020   PAL    PAL          |
| 3SEMICOM 5SEMICOM           PAL    PAL          |
|J   62256  62256             PAL                 |
|A                            PAL    27MHz        |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A1020B   M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW1         6264                               |
| DSW2  P87C52                9SEMICOM            |
|                 6SEMICOM   10SEMICOM   27C160*  |
|3* 4*            7SEMICOM   11SEMICOM   27C160*  |
|-------------------------------------------------|

The PCB used for Baryon is an earlier version with a single OKI sound chip

* denotes unpopulated components
  3 & 4 are 10 pin headers

*/

ROM_START( baryon ) // this set had original SemiCom labels
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "4_semicom", 0x000000, 0x040000, CRC(6c1cdad0) SHA1(40c437507076ce52ec2240049d6b4bef180b104a) ) //  eprom type 27C020
	ROM_LOAD32_BYTE( "5_semicom", 0x000001, 0x040000, CRC(15917c9d) SHA1(6444be93e6a997070820e3c5a2e2e703e22883d9) )
	ROM_LOAD32_BYTE( "2_semicom", 0x000002, 0x040000, CRC(42b14a6c) SHA1(37e772a673732ef16767c14ad77a4faaa06d675a) )
	ROM_LOAD32_BYTE( "3_semicom", 0x000003, 0x040000, CRC(0ae6d86e) SHA1(410ad161688ec8516fe5ac7160a4a228dbb01936) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped. */

	ROM_REGION( 0x6bd, "user1", 0 ) /* Protection data - from baryona set, assumed to be the same */
	ROM_LOAD( "protdata.bin", 0x000, 0x6bd, CRC(117f32a8) SHA1(837bea09d3e59ab9e13bd1103b1fc988edb361c0) ) /* extracted */

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples */
	ROM_LOAD( "1_semicom", 0x000000, 0x80000, CRC(e0349074) SHA1(f3d53d96dff586a0ad1632f52e5559cdce5ed0d8) ) //  eprom type 27C040

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "10_semicom", 0x000000, 0x200000, CRC(28bf828f) SHA1(271390cc4f4015a3b69976f0d0527947f13c971b) ) //  eprom type 27C160
	ROM_LOAD16_WORD_SWAP( "11_semicom", 0x200000, 0x200000, CRC(d0ff1bc6) SHA1(4aeb795222eedeeba770cf725122e989f97119b2) ) //  eprom type 27C160

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "8_semicom",0x000000, 0x200000, CRC(684012e6) SHA1(4cb60907184b67be130b8385e4336320c0f6e4a7) ) //  eprom type 27C160

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "6_semicom", 0x000000, 0x020000, CRC(fdbb08b0) SHA1(4b3ac56c4c8370b1434fb6a481fce0d9c52313e0) ) //  eprom type 27C010
	ROM_LOAD16_BYTE( "7_semicom", 0x000001, 0x020000, CRC(c9d20480) SHA1(3f6170e8e08fb7508bd13c23f243ec6888a91f5e) ) //  eprom type 27C010

	ROM_REGION( 0x10000, "unknown", 0 )
	ROM_LOAD( "9_semicom", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) ) //  eprom type 27C512
ROM_END

ROM_START( baryona ) // replacment labels? no SemiCom logo
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "4.bin", 0x000000, 0x040000, CRC(59e0df20) SHA1(ff12f4adcf731f6984db7d0fbdd7fcc71ce66aa4) )
	ROM_LOAD32_BYTE( "6.bin", 0x000001, 0x040000, CRC(abccbb3d) SHA1(01524f094543d872d775306024f51258a11e9240) )
	ROM_LOAD32_BYTE( "3.bin", 0x000002, 0x040000, CRC(046d4231) SHA1(05056efe5fec7f43c400f05278de516b01be0fdf) )
	ROM_LOAD32_BYTE( "5.bin", 0x000003, 0x040000, CRC(63d5e7cb) SHA1(269bf5ffe10f2464f823c4d377921e19cfb8bc46) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped. */

	ROM_REGION( 0x6bd, "user1", 0 ) /* Protection data  */
	ROM_LOAD( "protdata.bin", 0x000, 0x6bd, CRC(117f32a8) SHA1(837bea09d3e59ab9e13bd1103b1fc988edb361c0) ) /* extracted */

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples */
	ROM_LOAD( "1.bin", 0x000000, 0x80000, CRC(e0349074) SHA1(f3d53d96dff586a0ad1632f52e5559cdce5ed0d8) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "9.bin",  0x000000, 0x200000, CRC(28bf828f) SHA1(271390cc4f4015a3b69976f0d0527947f13c971b) )
	ROM_LOAD16_WORD_SWAP( "11.bin", 0x200000, 0x200000, CRC(d0ff1bc6) SHA1(4aeb795222eedeeba770cf725122e989f97119b2) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "2.bin",0x000000, 0x200000, CRC(684012e6) SHA1(4cb60907184b67be130b8385e4336320c0f6e4a7) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "8.bin", 0x000000, 0x020000, CRC(fdbb08b0) SHA1(4b3ac56c4c8370b1434fb6a481fce0d9c52313e0) )
	ROM_LOAD16_BYTE( "10.bin",0x000001, 0x020000, CRC(c9d20480) SHA1(3f6170e8e08fb7508bd13c23f243ec6888a91f5e) )

	ROM_REGION( 0x10000, "unknown", 0 )
	ROM_LOAD( "7.bin", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

/*

Cute Fighter
SemiCom, 1998

PCB Layout
----------

|-------------------------------------------------|
|    M6295  SEMICOM1 62256    ACTEL     SEMICOM12 |
|VOL M6295  SEMICOM2 62256    A1020B              |
|    PAL  PAL        32MHz                        |
| 62256  62256              PAL                   |
| SEMICOM3 SEMICOM5 68EC020 PAL    PAL            |
| SEMICOM4 SEMICOM6         PAL    PAL            |
|J 62256 62256              PAL                   |
|A                          PAL    27MHz          |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A40MX04  M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW1             6264                           |
| DSW2   8752                  SEMICOM9           |
|                    SEMICOM7 SEMICOM10 SEMICOM13 |
|3* 4*               SEMICOM8 SEMICOM11 SEMICOM14 |
|-------------------------------------------------|

A later version of the SemiCom 68020 hardware added a second OKI sound chip and sample rom

Main CPU 68EC020FG16           @ 16MHz
AD-65 (OKI MSM6295 rebadged)   @ 1MHz
Atmel AT89C52 MCU (secured)    @ 16MHZ

* 3 & 4 are 10 pin headers for unknown use. One might be used to drive the ticket dispenser

*/

ROM_START( cutefght )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "5_semicom", 0x000000, 0x080000, CRC(c14fd5dc) SHA1(f332105f5f249d693e792e7115f9e6cffb6db19f) )
	ROM_LOAD32_BYTE( "6_semicom", 0x000001, 0x080000, CRC(47440088) SHA1(c45503c4b5f271b430263ca079edeaaeadf5d9f6) )
	ROM_LOAD32_BYTE( "3_semicom", 0x000002, 0x080000, CRC(e7e7a866) SHA1(a31751f4164a427de59f0c76c9a8cb34370d8183) )
	ROM_LOAD32_BYTE( "4_semicom", 0x000003, 0x080000, CRC(476a3bf5) SHA1(5be1c70bbf4fcfc534b7f20bfceaa8da2e961330) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped. */

	ROM_REGION( 0x1000, "user1", ROMREGION_ERASEFF ) /* Protection data  */
	ROM_LOAD( "protdata.bin", 0x000, 0x701 , CRC(764c3c0e) SHA1(ae044d016850b730b2d97ccb7845b6b438c1e074) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples - 1st chip */
	ROM_LOAD( "2_semicom", 0x000000, 0x80000, CRC(694ddaf9) SHA1(f9138e7e1d8f771c4e69c17f27fb2b70fbee076a) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* OKI Samples - 2nd chip */
	ROM_LOAD( "1_semicom", 0x000000, 0x80000, CRC(fa3b6890) SHA1(7534931c96d6fa05fee840a7ea07b87e2e2acc50) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "10_semicom",  0x000000, 0x200000, CRC(62bf1e6e) SHA1(fb4b0db313e26687f0ebc6a8505a02e5348776da) )
	ROM_LOAD16_WORD_SWAP( "11_semicom",  0x200000, 0x200000, CRC(796f23a7) SHA1(adaa4c8525de428599f4489ecc8e966fed0d514d) )
	ROM_LOAD16_WORD_SWAP( "13_semicom",  0x400000, 0x200000, CRC(24222b3c) SHA1(08163863890c01728db89b8f4447841ecb4f4f62) )
	ROM_LOAD16_WORD_SWAP( "14_semicom",  0x600000, 0x200000, CRC(385b69d7) SHA1(8e7cae5589e354bea0b77b061af1d0c81d796f7c) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "12_semicom",0x000000, 0x200000, CRC(45d29c22) SHA1(df719a061dcd14fb4388fb45dfee2054e56a1299) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "7_semicom", 0x000000, 0x020000, CRC(39454102) SHA1(347e9242fd7e2092cfaacdce92691cf6024471ac) )
	ROM_LOAD16_BYTE( "8_semicom", 0x000001, 0x020000, CRC(fccb1b13) SHA1(fd4aec4a660f9913651fcc084e3f13eb0adbddd6) )

	ROM_REGION( 0x10000, "unknown", 0 ) /* ???? - not decoded seems to be in blocks of 0x41 bytes.. */
	ROM_LOAD( "9_semicom", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

/*

Rolling Crush
Trust / SemiCom, 1999

PCB Layout
----------

|-------------------------------------------------|
|    M6295* 27C40*  62256   ACTEL           ROM10 |
|VOL M6295  ROM6    62256   A40MX04               |
|    PAL  PAL       32MHz                         |
| 62256  62256              PAL                   |
| ROM2 ROM4       68EC020   PAL    PAL            |
| ROM1 ROM3                 PAL    PAL            |
|J 62256 62256              PAL                   |
|A                          PAL    27MHz          |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A40MX04  M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW2         6264                               |
| DSW2   8752               ROM9                  |
|                    ROM7   ROM8    27C160*       |
|3* 4*               ROM6   27C160* 27C160*       |
|-------------------------------------------------|

Same PCB as Cute Fighter / Dream World PCB except one OKI M6295 and it's sample rom are unpopulated

* denotes unpopulated components
  3 & 4 are 10 pin headers

Main CPU 68EC020FG16           @ 16MHz
AD-65 (OKI MSM6295 rebadged)   @ 1MHz
Atmel AT89C52 MCU (secured)    @ 16MHZ

V-SYNC                         @57.793 Hz
H-SYNC                         @15.19 - 15.27KHz (floating)

*/

ROM_START( rolcrush )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "mx27c2000_4.bin", 0x000000, 0x040000, CRC(c47f0540) SHA1(76712f41046e5852ad6be6dbf171cf34471e2409) )
	ROM_LOAD32_BYTE( "mx27c2000_3.bin", 0x000001, 0x040000, CRC(7af59294) SHA1(f36b3d100e0d963bf51b7fbe8c4a0bdcf2180ba0) )
	ROM_LOAD32_BYTE( "mx27c2000_2.bin", 0x000002, 0x040000, CRC(5eb24adb) SHA1(0329a02e18490bfe72ff34a64722d7316814720b) )
	ROM_LOAD32_BYTE( "mx27c2000_1.bin", 0x000003, 0x040000, CRC(a37e15b2) SHA1(f0fc945a894d6ed58daf05390a17051d0f3cda20) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped. */

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 ) /* Protection data  */
	ROM_LOAD( "protdata.bin", 0x000, 0x745, CRC(06b8a880) SHA1(b7d4bf26d34cb544825270c2c474bbd4c81a6c9e) ) /* extracted */

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples - 1st chip*/
	ROM_LOAD( "mx27c4000_5.bin", 0x000000, 0x80000, CRC(7afa6adb) SHA1(d4049e1068a5f7abf0e14d0b9fbbbc6dfb5d0170) )

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 ) /* OKI Samples - 2nd chip (neither OKI or rom is present, empty sockets) */
	/* not populared */

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "m27c160.8.bin", 0x000000, 0x200000, CRC(a509bc36) SHA1(aaa008e07e4b24ff9dbcee5925d6516d1662931c) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "m27c160.10.bin",0x000000, 0x200000, CRC(739b0cb0) SHA1(a7cc48502d84218586afa7276fa7ba759242f05e) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "tms27c010_7.bin", 0x000000, 0x020000, CRC(4cb84384) SHA1(8dd02e2d9829c15cb19654779d2217a7d53d5971) )
	ROM_LOAD16_BYTE( "tms27c010_6.bin", 0x000001, 0x020000, CRC(0c9d197a) SHA1(da057c8d08f41c4a5b9cb4f8f00de7e1461d98f0) )

	ROM_REGION( 0x10000, "unknown", 0 ) /* ???? - not decoded seems to be in blocks of 0x41 bytes.. */
	ROM_LOAD( "mx27c512.9.bin", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

/*

Dream World
SemiCom, 2000

PCB Layout
----------

|-------------------------------------------------|
|    M6295  ROM5    62256   ACTEL           ROM10 |
|VOL M6295  ROM6    62256   A40MX04               |
|    PAL  PAL       32MHz                         |
| 62256  62256              PAL                   |
| ROM1 ROM3       68EC020   PAL    PAL            |
| ROM2 ROM4                 PAL    PAL            |
|J 62256 62256              PAL                   |
|A                          PAL    27MHz          |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A40MX04  M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW1         6264                               |
| DSW2   8752               ROM11                 |
|                    ROM7   ROM9    27C160*       |
|3* 4*               ROM8   27C160* 27C160*       |
|-------------------------------------------------|

* denotes unpopulated components
  3 & 4 are 10 pin headers

Notes:
      68020 @ 16.0MHz [32/2]
      M6295 (both) @ 1.0MHz [32/32]. pin 7 LOW
      8752 @ 16.0MHz [32/2]
      HSync @ 15.2kHz
      VSync @ 58Hz
*/

ROM_START( dreamwld )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "3.bin", 0x000000, 0x040000, CRC(e8f7ae78) SHA1(cfd393cec6dec967c82e1131547b7e7fdc5d814f) )
	ROM_LOAD32_BYTE( "4.bin", 0x000001, 0x040000, CRC(3ef5d51b) SHA1(82a00b4ff7155f6d5553870dfd510fed9469d9b5) )
	ROM_LOAD32_BYTE( "1.bin", 0x000002, 0x040000, CRC(35c94ee5) SHA1(3440a65a807622b619c97bc2a88fd7d875c26f66) )
	ROM_LOAD32_BYTE( "2.bin", 0x000003, 0x040000, CRC(5409e7fc) SHA1(2f94a6a8e4c94b36b43f0b94d58525f594339a9d) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped. */

	ROM_REGION( 0x6c9, "user1", 0 ) /* Protection data  */
	/* The MCU supplies this data.
	  The 68k reads it through a port, taking the size and destination write address from the level 1
	  and level 2 irq positions in the 68k vector table (there is code to check that they haven't been
	  modified!)  It then decodes the data using the rom checksum previously calculated and puts it in
	  ram.  The interrupt vectors point at the code placed in RAM. */
	ROM_LOAD( "protdata.bin", 0x000, 0x6c9 ,  CRC(f284b2fd) SHA1(9e8096c8aa8a288683f002311b38787b120748d1) ) /* extracted */

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples - 1st chip */
	ROM_LOAD( "5.bin", 0x000000, 0x80000, CRC(9689570a) SHA1(4414233da8f46214ca7e9022df70953922a63aa4) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* OKI Samples - 2nd chip */
	ROM_LOAD( "6.bin", 0x000000, 0x80000, CRC(c8b91f30) SHA1(706004ca56d0a74bc7a3dfd73a21cdc09eb90f05) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "9.bin", 0x000000, 0x200000, CRC(fa84e3af) SHA1(5978737d348fd382f4ec004d29870656c864d137) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "10.bin",0x000000, 0x200000, CRC(3553e4f5) SHA1(c335494f4a12a01a88e7cd578cae922954303cfd) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "8.bin", 0x000000, 0x020000, CRC(8d570df6) SHA1(e53e4b099c64eca11d027e0083caa101fcd99959) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x020000, CRC(a68bf35f) SHA1(f48540a5415a7d9723ca6e7e03cab039751dce17) )

	ROM_REGION( 0x10000, "unknown", 0 ) /* ???? - not decoded seems to be in blocks of 0x41 bytes.. */
	ROM_LOAD( "11.bin", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

/*

Gaia - The Last Choice of Earth
SemiCom / XESS, 1999

PCB Layout
----------

|-------------------------------------------------|
|    M6295* 27C40*  62256   ACTEL           ROM8  |
|VOL M6295  ROM1    62256   A40MX04               |
|    PAL  PAL       32MHz                         |
| 62256  62256              PAL                   |
| ROM2 ROM4       68EC020   PAL    PAL            |
| ROM3 ROM5                 PAL    PAL            |
|J 62256 62256              PAL                   |
|A                          PAL    27MHz          |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A40MX04  M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW2         6264                               |
| DSW2   8752               ROM9                  |
|                    ROM6   ROM10   ROM12         |
|3* 4*               ROM7   ROM11   27C160*       |
|-------------------------------------------------|

Same PCB as Cute Fighter / Dream World PCB except one OKI M6295 and it's sample rom are unpopulated
IE: Same configuration as Rolling Crush

No labels on any of the roms.

Program roms 2 through 5 are MX27C2000
Rom 1 is a TMS 27C040
Rom 9 is a MX26C512
Roms 6 & 7 are MX26C1000
Roms 8 and 10 through 12 are ST M27C160

* denotes unpopulated components
  3 & 4 are 10 pin headers

Main CPU 68EC020FG16           @ 16MHz [32/2]
AD-65 (OKI MSM6295 rebadged)   @ 1MHz [32/32]. pin 7 LOW
Atmel AT89C52 MCU (secured)    @ 16MHZ [32/2]

*/

ROM_START( gaialast )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "4", 0x000000, 0x040000, CRC(10cc2dee) SHA1(0719333ff391ee7935c6e4cea7e8e75369aeb9d0) )
	ROM_LOAD32_BYTE( "5", 0x000001, 0x040000, CRC(c55f6f11) SHA1(13d543b0770bebdd4c6e064b56fd6cc2ec929566) )
	ROM_LOAD32_BYTE( "2", 0x000002, 0x040000, CRC(549e594a) SHA1(728c6b51cc478ad7251bcbe6d7f4f4e6a2ee4a4e) )
	ROM_LOAD32_BYTE( "3", 0x000003, 0x040000, CRC(a8e845d8) SHA1(f8c7e702bd747a22e76c861effec4cd3cd2f3fc9) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped. */

	ROM_REGION( 0x6c9, "user1", ROMREGION_ERASEFF ) /* Protection data  */
	ROM_LOAD( "protdata.bin", 0x000, 0x6c9 , CRC(d3403b7b) SHA1(712a7f27fc41b632d584237f7641e8ae20035111) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples */
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2dbad410) SHA1(bb788ea14bb605be9af9c8f8adec94ad1c17ab55) )

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 ) /* OKI Samples - 2nd chip (neither OKI or rom is present, empty sockets) */
	/* not populared */

	ROM_REGION( 0x800000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "10", 0x000000, 0x200000, CRC(5822ef93) SHA1(8ce22c30f8027f35c5f72eb6ce57a74540dd55da) )
	ROM_LOAD16_WORD_SWAP( "11", 0x200000, 0x200000, CRC(f4f5770d) SHA1(ac850483cae321d286a09fe93ce7e49725722de0) )
	ROM_LOAD16_WORD_SWAP( "12", 0x400000, 0x200000, CRC(a1f04571) SHA1(c29b3b3c209b63ad44ebfa5afb4b1832965e0936) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD16_WORD_SWAP( "8",0x000000, 0x200000, CRC(32d16985) SHA1(2b7a20eea09e7d2debd42469e9f6ae49310f5747) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "6", 0x000000, 0x020000, CRC(5c82feed) SHA1(1857afecf1081adf015ade1efb5930e3a7deef78) )
	ROM_LOAD16_BYTE( "7", 0x000001, 0x020000, CRC(9d7f04ae) SHA1(55fb82626060fe0ddc03ed3ef402ccf998063d27) )

	ROM_REGION( 0x10000, "unknown", 0 )
	ROM_LOAD( "9", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

GAME( 1997, baryon,   0,      baryon,   baryon,   driver_device, 0, ROT270, "SemiCom / Tirano",         "Baryon - Future Assault (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, baryona,  baryon, baryon,   baryon,   driver_device, 0, ROT270, "SemiCom / Tirano",         "Baryon - Future Assault (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, cutefght, 0,      dreamwld, cutefght, driver_device, 0, ROT0,   "SemiCom",                  "Cute Fighter", MACHINE_SUPPORTS_SAVE )
GAME( 1999, rolcrush, 0,      baryon,   rolcrush, driver_device, 0, ROT0,   "Trust / SemiCom",          "Rolling Crush (version 1.07.E - 1999/02/11)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, gaialast, 0,      baryon,   gaialast, driver_device, 0, ROT0,   "SemiCom / XESS",           "Gaia - The Last Choice of Earth", MACHINE_SUPPORTS_SAVE )
GAME( 2000, dreamwld, 0,      dreamwld, dreamwld, driver_device, 0, ROT0,   "SemiCom",                  "Dream World", MACHINE_SUPPORTS_SAVE )
