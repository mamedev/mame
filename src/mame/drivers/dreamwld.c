/*  Semicom Baryon / Dream World hardware

Note: this hardware is a copy of Psikyo's 68020 based hardware,
      the Strikers 1945 bootleg has the same unknown rom!

      It isn't quite as flexible as the original Psikyo hardware
      by the looks of it, there are various subtle changes to how
      things work, for example the tilemap sizes and missing
      transparent pen modification.  This makes it rather hard to
      merge with psikyo.c and it should probably be left separate.

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
|              6264                               |
| DSW1                      ROM11                 |
|        8752        ROM7   ROM9                  |
| DSW2               ROM8                         |
|-------------------------------------------------|
Notes:
      68020 @ 16.0MHz [32/2]
      M6295 (both) @ 1.0MHz [32/32]. pin 7 LOW
      8752 @ 16.0MHz [32/2]
      HSync @ 15.2kHz
      VSync @ 58Hz


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

#define MASTER_CLOCK 32000000

class dreamwld_state : public driver_device
{
public:
	dreamwld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg2_videoram(*this, "bg2_videoram"),
		m_vregs(*this, "vregs"),
		m_workram(*this, "workram"){ }

	/* memory pointers */
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_paletteram;
	required_shared_ptr<UINT32> m_bg_videoram;
	required_shared_ptr<UINT32> m_bg2_videoram;
	required_shared_ptr<UINT32> m_vregs;
	required_shared_ptr<UINT32> m_workram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_bg2_tilemap;
	int      m_tilebank[2];
	int      m_tilebankold[2];

	UINT32* m_spritebuf1;
	UINT32* m_spritebuf2;

	/* misc */
	int      m_protindex;
	DECLARE_WRITE32_MEMBER(dreamwld_bg_videoram_w);
	DECLARE_WRITE32_MEMBER(dreamwld_bg2_videoram_w);
	DECLARE_READ32_MEMBER(dreamwld_protdata_r);
	DECLARE_WRITE32_MEMBER(dreamwld_6295_0_bank_w);
	DECLARE_WRITE32_MEMBER(dreamwld_6295_1_bank_w);
	DECLARE_WRITE32_MEMBER(dreamwld_palette_w);
};



static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	dreamwld_state *state = machine.driver_data<dreamwld_state>();
	const gfx_element *gfx = machine.gfx[0];
	UINT32 *source = state->m_spritebuf1;
	UINT32 *finish = state->m_spritebuf1 + 0x1000 / 4;
	UINT16 *redirect = (UINT16 *)state->memregion("spritelut")->base();

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

		tileno = (source[1] & 0x0000ffff) >>0;
		colour = (source[1] & 0x3f000000) >>24;
		xflip  = (source[1] & 0x40000000);
		yflip  = (source[1] & 0x80000000);

		xinc = 16;
		yinc = 16;

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
				drawgfx_transpen(bitmap, cliprect, gfx, redirect[tileno], colour, xflip, yflip, xpos + xct * xinc, ypos + yct * yinc, 0);
				drawgfx_transpen(bitmap, cliprect, gfx, redirect[tileno], colour, xflip, yflip, (xpos + xct * xinc) - 0x200, ypos + yct * yinc, 0);
				drawgfx_transpen(bitmap, cliprect, gfx, redirect[tileno], colour, xflip, yflip, (xpos + xct * xinc) - 0x200, (ypos + yct * yinc) - 0x200, 0);
				drawgfx_transpen(bitmap, cliprect, gfx, redirect[tileno], colour, xflip, yflip, xpos + xct * xinc, (ypos + yct * yinc) - 0x200 , 0);

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

static TILE_GET_INFO( get_dreamwld_bg_tile_info )
{
	dreamwld_state *state = machine.driver_data<dreamwld_state>();
	int tileno, colour;
	tileno = (tile_index & 1) ? (state->m_bg_videoram[tile_index >> 1] & 0xffff) : ((state->m_bg_videoram[tile_index >> 1] >> 16) & 0xffff);
	colour = tileno >> 13;
	tileno &= 0x1fff;
	SET_TILE_INFO(1, tileno + state->m_tilebank[0] * 0x2000, 0x80 + colour, 0);
}


WRITE32_MEMBER(dreamwld_state::dreamwld_bg2_videoram_w)
{
	COMBINE_DATA(&m_bg2_videoram[offset]);
	m_bg2_tilemap->mark_tile_dirty(offset * 2);
	m_bg2_tilemap->mark_tile_dirty(offset * 2 + 1);
}

static TILE_GET_INFO( get_dreamwld_bg2_tile_info )
{
	dreamwld_state *state = machine.driver_data<dreamwld_state>();
	UINT16 tileno, colour;
	tileno = (tile_index & 1) ? (state->m_bg2_videoram[tile_index >> 1] & 0xffff) : ((state->m_bg2_videoram[tile_index >> 1] >> 16) & 0xffff);
	colour = tileno >> 13;
	tileno &= 0x1fff;
	SET_TILE_INFO(1, tileno + state->m_tilebank[1] * 0x2000, 0xc0 + colour, 0);
}

static VIDEO_START( dreamwld )
{
	dreamwld_state *state = machine.driver_data<dreamwld_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_dreamwld_bg_tile_info,tilemap_scan_rows, 16, 16, 64,32);
	state->m_bg2_tilemap = tilemap_create(machine, get_dreamwld_bg2_tile_info,tilemap_scan_rows, 16, 16, 64,32);
	state->m_bg2_tilemap->set_transparent_pen(0);

	state->m_bg_tilemap->set_scroll_rows(256);	// line scrolling
	state->m_bg_tilemap->set_scroll_cols(1);

	state->m_bg2_tilemap->set_scroll_rows(256);	// line scrolling
	state->m_bg2_tilemap->set_scroll_cols(1);

	state->m_spritebuf1 = auto_alloc_array(machine, UINT32, 0x2000 / 4);
	state->m_spritebuf2 = auto_alloc_array(machine, UINT32, 0x2000 / 4);


}

SCREEN_VBLANK( dreamwld )
{
	// rising edge
	if (vblank_on)
	{
		dreamwld_state *state = screen.machine().driver_data<dreamwld_state>();
		memcpy(state->m_spritebuf2, state->m_spritebuf1, 0x2000);
		memcpy(state->m_spritebuf1, state->m_spriteram, 0x2000);
	}
}


static SCREEN_UPDATE_IND16( dreamwld )
{
	dreamwld_state *state = screen.machine().driver_data<dreamwld_state>();
//  int tm0size, tm1size;

	tilemap_t *tmptilemap0, *tmptilemap1;

	tmptilemap0 = state->m_bg_tilemap;
	tmptilemap1 = state->m_bg2_tilemap;

	int layer0_scrolly = state->m_vregs[(0x400 / 4)] + 32;
	int layer1_scrolly = state->m_vregs[(0x400 / 4) + 2] + 32;

	int layer0_scrollx = state->m_vregs[(0x400 / 4) + 1] + 3;
	int layer1_scrollx = state->m_vregs[(0x400 / 4) + 3] + 5;
	UINT32 layer0_ctrl = state->m_vregs[0x412 / 4];
	UINT32 layer1_ctrl = state->m_vregs[0x416 / 4];

	tmptilemap0->set_scrolly(0, layer0_scrolly);
	tmptilemap1->set_scrolly(0, layer1_scrolly);

	// not on this hw?
#if 0
	switch ((layer0_ctrl & 0x00c0) >> 6)
	{
	case 0:	tm0size = 1;	break;
	case 1:	tm0size = 2;	break;
	case 2:	tm0size = 3;	break;
	default:	tm0size = 0;	break;
	}

	switch ((layer1_ctrl & 0x00c0) >> 6)
	{
	case 0:	tm1size = 1;	break;
	case 1:	tm1size = 2;	break;
	case 2:	tm1size = 3;	break;
	default:	tm1size = 0;	break;
	}
#endif
	//popmessage("sizes %d %d\n", tm0size, tm1size);

	for (int i = 0; i < 256; i++)	/* 256 screen lines */
	{
		int x0 = 0, x1 = 0;

		/* layer 0 */
		UINT16 *vregs = reinterpret_cast<UINT16 *>(state->m_vregs.target());
		if (layer0_ctrl & 0x0300)
		{
			if (layer0_ctrl & 0x0200)
				/* per-tile rowscroll */
				x0 = vregs[BYTE_XOR_BE(0x000/2 + i/16)];
			else
				/* per-line rowscroll */
				x0 = vregs[BYTE_XOR_BE(0x000/2 + ((i + layer0_scrolly)&0xff))]; // different handling to psikyo.c? ( + scrolly )
		}


			tmptilemap0->set_scrollx(
			(i + layer0_scrolly) % 256 /*tilemap_width(tm0size) */,
			layer0_scrollx + x0 );


		/* layer 1 */
		if (layer1_ctrl & 0x0300)
		{
			if (layer1_ctrl & 0x0200)
				/* per-tile rowscroll */
				x1 = vregs[BYTE_XOR_BE(0x200/2 + i/16)];
			else
				/* per-line rowscroll */
				x1 = vregs[BYTE_XOR_BE(0x200/2 + ((i + layer1_scrolly)&0xff))];  // different handling to psikyo.c? ( + scrolly )
		}


			tmptilemap1->set_scrollx(
			(i + layer1_scrolly) % 256 /* tilemap_width(tm1size) */,
			layer1_scrollx + x1 );
	}


	state->m_tilebank[0] = (state->m_vregs[(0x400 / 4) + 4] >> 6) & 1;
	state->m_tilebank[1] = (state->m_vregs[(0x400 / 4) + 5] >> 6) & 1;

	if (state->m_tilebank[0] != state->m_tilebankold[0])
	{
		state->m_tilebankold[0] = state->m_tilebank[0];
		state->m_bg_tilemap->mark_all_dirty();
	}

	if (state->m_tilebank[1] != state->m_tilebankold[1])
	{
		state->m_tilebankold[1] = state->m_tilebank[1];
		state->m_bg2_tilemap->mark_all_dirty();
	}

	tmptilemap0->draw(bitmap, cliprect, 0, 0);
	tmptilemap1->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen.machine(), bitmap, cliprect);

	return 0;
}



READ32_MEMBER(dreamwld_state::dreamwld_protdata_r)
{
	//static int count = 0;


	//printf("protection read %04x\n", count);
	//count++;

	UINT8 *protdata = memregion("user1")->base();
	size_t protsize = memregion("user1")->bytes();
	UINT8 dat = protdata[(m_protindex++) % protsize];

	// real hw returns 00 after end of data, I haven't checked if it's possible to overflow the read counter
	// and read out the internal rom.

	return dat << 24;
}


WRITE32_MEMBER(dreamwld_state::dreamwld_6295_0_bank_w)
{
	UINT8 *sound = memregion("oki1")->base();

	if (ACCESSING_BITS_0_7)
		memcpy(sound + 0x30000, sound + 0xb0000 + 0x10000 * (data&0x3), 0x10000);
	else
		logerror("OKI0: unk bank write %x mem_mask %8x\n", data, mem_mask);
}

WRITE32_MEMBER(dreamwld_state::dreamwld_6295_1_bank_w)
{
	UINT8 *sound = memregion("oki2")->base();

	if (ACCESSING_BITS_0_7)
		memcpy(sound + 0x30000, sound + 0xb0000 + 0x10000 * (data&0x3), 0x10000);
	else
		logerror("OKI1: unk bank write %x mem_mask %8x\n", data, mem_mask);
}

// why doesn't using paletteram_xRRRRRGGGGGBBBBB_word_w with a 16-bit handler work? colours are
// severely corrupt on dream world's semicom screen + many sprites, seems palette values get duplicated.
WRITE32_MEMBER(dreamwld_state::dreamwld_palette_w)
{
	UINT16 dat;
	int color;

	COMBINE_DATA(&m_paletteram[offset]);
	color = offset * 2;

	dat = m_paletteram[offset] & 0x7fff;
	palette_set_color_rgb(machine(), color+1, pal5bit(dat >> 10), pal5bit(dat >> 5), pal5bit(dat >> 0));

	dat = (m_paletteram[offset] >> 16) & 0x7fff;
	palette_set_color_rgb(machine(), color, pal5bit(dat >> 10), pal5bit(dat >> 5), pal5bit(dat >> 0));
}



static ADDRESS_MAP_START( baryon_map, AS_PROGRAM, 32, dreamwld_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM  AM_WRITENOP

	AM_RANGE(0x400000, 0x401fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x600000, 0x601fff) AM_RAM AM_WRITE(dreamwld_palette_w) AM_SHARE("paletteram")
	AM_RANGE(0x800000, 0x801fff) AM_RAM_WRITE(dreamwld_bg_videoram_w ) AM_SHARE("bg_videoram")
	AM_RANGE(0x802000, 0x803fff) AM_RAM_WRITE(dreamwld_bg2_videoram_w ) AM_SHARE("bg2_videoram")
	AM_RANGE(0x804000, 0x805fff) AM_RAM AM_SHARE("vregs")  // scroll regs etc.

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



static MACHINE_START( dreamwld )
{
	dreamwld_state *state = machine.driver_data<dreamwld_state>();

	state->save_item(NAME(state->m_protindex));
	state->save_item(NAME(state->m_tilebank));
	state->save_item(NAME(state->m_tilebankold));
}

static MACHINE_RESET( dreamwld )
{
	dreamwld_state *state = machine.driver_data<dreamwld_state>();

	state->m_tilebankold[0] = state->m_tilebankold[1] = -1;
	state->m_tilebank[0] = state->m_tilebank[1] = 0;
	state->m_protindex = 0;
}




static MACHINE_CONFIG_START( baryon, dreamwld_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(baryon_map)
	MCFG_CPU_VBLANK_INT("screen", irq4_line_hold )

	MCFG_MACHINE_START(dreamwld)
	MCFG_MACHINE_RESET(dreamwld)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(512,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 304-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_STATIC(dreamwld)
	MCFG_SCREEN_VBLANK_STATIC(dreamwld)

	MCFG_PALETTE_LENGTH(0x1000)
	MCFG_GFXDECODE(dreamwld)

	MCFG_VIDEO_START(dreamwld)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", MASTER_CLOCK/32, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dreamwld, baryon )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dreamwld_map)
	MCFG_CPU_VBLANK_INT("screen", irq4_line_hold)

	MCFG_OKIM6295_ADD("oki2", MASTER_CLOCK/32, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

ROM_START( dreamwld )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "1.bin", 0x000002, 0x040000, CRC(35c94ee5) SHA1(3440a65a807622b619c97bc2a88fd7d875c26f66) )
	ROM_LOAD32_BYTE( "2.bin", 0x000003, 0x040000, CRC(5409e7fc) SHA1(2f94a6a8e4c94b36b43f0b94d58525f594339a9d) )
	ROM_LOAD32_BYTE( "3.bin", 0x000000, 0x040000, CRC(e8f7ae78) SHA1(cfd393cec6dec967c82e1131547b7e7fdc5d814f) )
	ROM_LOAD32_BYTE( "4.bin", 0x000001, 0x040000, CRC(3ef5d51b) SHA1(82a00b4ff7155f6d5553870dfd510fed9469d9b5) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped. */

	ROM_REGION( 0x6c9, "user1", 0 ) /* Protection data  */
	/* The MCU supplies this data.
      The 68k reads it through a port, taking the size and destination write address from the level 1
      and level 2 irq positions in the 68k vector table (there is code to check that they haven't been
      modified!)  It then decodes the data using the rom checksum previously calculated and puts it in
      ram.  The interrupt vectors point at the code placed in RAM. */
	ROM_LOAD( "protdata.bin", 0x000, 0x6c9 ,  CRC(f284b2fd) SHA1(9e8096c8aa8a288683f002311b38787b120748d1) ) /* extracted */

	ROM_REGION( 0x100000, "oki1", 0 ) /* OKI Samples - 1st chip*/
	ROM_LOAD( "5.bin", 0x000000, 0x80000, CRC(9689570a) SHA1(4414233da8f46214ca7e9022df70953922a63aa4) )
	ROM_RELOAD(0x80000,0x80000) // for the banks

	ROM_REGION( 0x100000, "oki2", 0 ) /* OKI Samples - 2nd chip*/
	ROM_LOAD( "6.bin", 0x000000, 0x80000, CRC(c8b91f30) SHA1(706004ca56d0a74bc7a3dfd73a21cdc09eb90f05) )
	ROM_RELOAD(0x80000,0x80000) // for the banks

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

ROM_START( baryon )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "3.bin", 0x000002, 0x040000, CRC(046d4231) SHA1(05056efe5fec7f43c400f05278de516b01be0fdf) )
	ROM_LOAD32_BYTE( "4.bin", 0x000000, 0x040000, CRC(59e0df20) SHA1(ff12f4adcf731f6984db7d0fbdd7fcc71ce66aa4) )
	ROM_LOAD32_BYTE( "5.bin", 0x000003, 0x040000, CRC(63d5e7cb) SHA1(269bf5ffe10f2464f823c4d377921e19cfb8bc46) )
	ROM_LOAD32_BYTE( "6.bin", 0x000001, 0x040000, CRC(abccbb3d) SHA1(01524f094543d872d775306024f51258a11e9240) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped. */

	ROM_REGION( 0x6bd, "user1", 0 ) /* Protection data  */
	ROM_LOAD( "protdata.bin", 0x000, 0x6bd, CRC(117f32a8) SHA1(837bea09d3e59ab9e13bd1103b1fc988edb361c0) ) /* extracted */

	ROM_REGION( 0x100000, "oki1", 0 ) /* OKI Samples */
	ROM_LOAD( "1.bin", 0x000000, 0x80000, CRC(e0349074) SHA1(f3d53d96dff586a0ad1632f52e5559cdce5ed0d8) )
	ROM_RELOAD(0x80000,0x80000) // for the banks

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




GAME( 1997, baryon,   0, baryon,   dreamwld, driver_device, 0, ROT270,  "SemiCom", "Baryon - Future Assault", GAME_SUPPORTS_SAVE )
GAME( 2000, dreamwld, 0, dreamwld, dreamwld, driver_device, 0, ROT0,  "SemiCom", "Dream World", GAME_SUPPORTS_SAVE )
