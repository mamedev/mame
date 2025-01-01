// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

                     -= 68020 + Imagetek I5000 Games =-

            driver by   David Haywood
            partly based on metro.cpp driver by Luca Elia


Main  CPU    :  MC68020

Video Chips   : Imagetek I5000 |
Sound Chips   : Imagetek I5000 |\- both sound and gfx
Other Features: Memory Blitter (part of Imagetek I5000)

---------------------------------------------------------------------------
Year + Game                     PCB         Video Chip  Issues / Notes
---------------------------------------------------------------------------
97  Rabbit                      VG5330-B    I5000
97? Tokimeki Mahjong Paradise(1)VG5550-B    I5000
---------------------------------------------------------------------------
Not dumped yet:
unknown

To Do:

- raster effects (rabbit only?, see left side of one of the levels in rabbit)
- clean up zoom code and make zoom effect more accurate

Notes:

(1) This is currently in its own driver "tmmjprd.cpp" because it uses the
    chip in a completely different way to Rabbit.  They should be merged
    again later, once the chip is better understood.



Rabbit PCB Layout
-----------------

VG5330-B
|---------------------------------|
|    62256  62256        61    60 |
|    62256  62256        51    50 |
|    62256  62256                 |
|    62256  62256        43    42 |
|                        41    40 |
|                                 |
|J     IMAGETEK          33    32 |
|A      I5000            23    22 |
|M            40MHz               |
|M                       13    12 |
|A             68EC020   03    02 |
|     ALTERA                      |
|     EPM7032 24MHz      31    30 |
|     93C46              21    20 |
|                                 |
| JPR2  JPR0  62256      11    10 |
| JPR3  JPR1  62256      01    00 |
|---------------------------------|

Notes:
      68020 clock: 24.000MHz
      VSync: 60Hz

      Only ROMs positions 60, 50, 40, 02, 03, 10, 11, 01, 00 are populated.

Tokimeki Mahjong Paradise - Dear My Love Board Notes
----------------------------------------------------

Board:  VG5550-B

CPU:    MC68EC020FG25
OSC:    40.00000MHz
        24.00000MHz

Custom: Imagetek I5000 (2ch video & 2ch sound)

*/

#include "emu.h"
#include "cpu/m68000/m68020.h"
#include "machine/eepromser.h"
#include "sound/i5000.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class rabbit_state : public driver_device
{
public:
	rabbit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_viewregs0(*this, "viewregs0"),
		m_viewregs6(*this, "viewregs6"),
		m_viewregs7(*this, "viewregs7"),
		m_viewregs9(*this, "viewregs9"),
		m_viewregs10(*this, "viewregs10"),
		m_tilemap_regs(*this, "tilemap_regs.%u", 0),
		m_spriteregs(*this, "spriteregs"),
		m_blitterregs(*this, "blitterregs"),
		m_spriteram(*this, "spriteram")
	{ }

	void rabbit(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(blit_done);

	void tilemap0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void tilemap1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void tilemap2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void tilemap3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t tilemap0_r(offs_t offset);
	uint32_t tilemap1_r(offs_t offset);
	uint32_t tilemap2_r(offs_t offset);
	uint32_t tilemap3_r(offs_t offset);
	uint32_t randomrabbits();
	void rombank_w(uint32_t data);
	void blitter_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void eeprom_write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void rabbit_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint32_t> m_viewregs0;
	required_shared_ptr<uint32_t> m_viewregs6;
	required_shared_ptr<uint32_t> m_viewregs7;
	required_shared_ptr<uint32_t> m_viewregs9;
	required_shared_ptr<uint32_t> m_viewregs10;
	required_shared_ptr_array<uint32_t, 4> m_tilemap_regs;
	required_shared_ptr<uint32_t> m_spriteregs;
	required_shared_ptr<uint32_t> m_blitterregs;
	required_shared_ptr<uint32_t> m_spriteram;

	std::unique_ptr<bitmap_ind16> m_sprite_bitmap;
	rectangle m_sprite_clip;
	int m_vblirqlevel = 0;
	int m_bltirqlevel = 0;
	int m_banking = 0;
	std::unique_ptr<uint32_t[]> m_tilemap_ram[4];
	tilemap_t *m_tilemap[4]{};
	emu_timer *m_blit_done_timer = nullptr;

	TILE_GET_INFO_MEMBER(get_tilemap0_tile_info);
	TILE_GET_INFO_MEMBER(get_tilemap1_tile_info);
	TILE_GET_INFO_MEMBER(get_tilemap2_tile_info);
	TILE_GET_INFO_MEMBER(get_tilemap3_tile_info);

	INTERRUPT_GEN_MEMBER(vblank_interrupt);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tilemap_info(tile_data &tileinfo, int tile_index, int whichtilemap, int tilesize);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void clearspritebitmap( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprite_bitmap( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void drawtilemap( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int whichtilemap );
	void do_blit();
};

void rabbit_state::machine_start()
{
	m_banking = 1;
	m_vblirqlevel = 6;
	m_bltirqlevel = 4;
	/* 5 and 1 are also valid and might be raster related */
}

/* call with tilesize = 0 for 8x8 or 1 for 16x16 */
void rabbit_state::get_tilemap_info(tile_data &tileinfo, int tile_index, int whichtilemap, int tilesize)
{
	/* fedcba98 76543210 fedcba98 76543210
	   x                                    color mask? how exactly does it relate to color bits?
	    xx                                  flip
	      x                                 depth
	       xxxx xxxx                        color
	                xxxx                    bank
	                     xxxxxxxx xxxxxxxx  tile
	*/
	int depth = (m_tilemap_ram[whichtilemap][tile_index]&0x10000000)>>28;
	int tileno = m_tilemap_ram[whichtilemap][tile_index]&0xffff;
	int bank = (m_tilemap_ram[whichtilemap][tile_index]&0x000f0000)>>16;
	int colour = (m_tilemap_ram[whichtilemap][tile_index]>>20)&0xff;
	int cmask = m_tilemap_ram[whichtilemap][tile_index]>>31&1;
	int flipxy = (m_tilemap_ram[whichtilemap][tile_index]>>29)&3;

	if (m_banking)
	{
		switch (bank)
		{
			case 0x0:
				break;

			case 0x8:
				tileno += 0x10000;
				break;

			case 0xc:
				tileno += 0x20000;
				break;

			default:
				//printf("tilebank %x\n",bank);
				break;
		}
	}
	else
	{
		tileno += (bank << 16);
	}

	if (depth)
	{
		tileno >>=(1+tilesize*2);
		colour &= 0x0f;
		colour += 0x20;
		tileinfo.group = 1;
		tileinfo.set(6+tilesize,tileno,colour,TILE_FLIPXY(flipxy));
	}
	else
	{
		tileno >>=(0+tilesize*2);
		if (cmask) colour&=0x3f; // see health bars
		colour += 0x200;
		tileinfo.group = 0;
		tileinfo.set(4+tilesize,tileno,colour,TILE_FLIPXY(flipxy));
	}
}

TILE_GET_INFO_MEMBER(rabbit_state::get_tilemap0_tile_info)
{
	get_tilemap_info(tileinfo,tile_index,0,1);
}

TILE_GET_INFO_MEMBER(rabbit_state::get_tilemap1_tile_info)
{
	get_tilemap_info(tileinfo,tile_index,1,1);
}

TILE_GET_INFO_MEMBER(rabbit_state::get_tilemap2_tile_info)
{
	get_tilemap_info(tileinfo,tile_index,2,1);
}

TILE_GET_INFO_MEMBER(rabbit_state::get_tilemap3_tile_info)
{
	get_tilemap_info(tileinfo,tile_index,3,0);
}

void rabbit_state::tilemap0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tilemap_ram[0][offset]);
	m_tilemap[0]->mark_tile_dirty(offset);
}

void rabbit_state::tilemap1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tilemap_ram[1][offset]);
	m_tilemap[1]->mark_tile_dirty(offset);
}

void rabbit_state::tilemap2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tilemap_ram[2][offset]);
	m_tilemap[2]->mark_tile_dirty(offset);
}


void rabbit_state::tilemap3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tilemap_ram[3][offset]);
	m_tilemap[3]->mark_tile_dirty(offset);
}

/*

'spriteregs' format (7 dwords)

00000XXX 0YYYAAAA 0000BBBB 0000CCCC 03065000 00720008 00a803bc
             zoom     zoom     zoom          Strt/End

XXX = global Xpos
YYY = global Ypos

AAAA = 0100 when normal, 018c when zoomed out
BBBB = 9f80 when normal, f6ba when zoomed out  0x9f80 / 128 = 319
CCCC = 6f80 when normal, ac7a when zoomed out  0x6f80 / 128 = 223


sprites invisible at the end of a round in rabbit, why?

*/

void rabbit_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int xpos,ypos,tileno,xflip,yflip, colr;
	gfx_element *gfx = m_gfxdecode->gfx(1);
	int todraw = (m_spriteregs[5]&0x0fff0000)>>16; // how many sprites to draw (start/end reg..) what is the other half?

	uint32_t *source = (m_spriteram+ (todraw*2))-2;
	uint32_t *finish = m_spriteram;

//  m_sprite_bitmap->fill(0x0, m_sprite_clip); // sloooow

	while( source>=finish )
	{
		xpos = (source[0]&0x00000fff);
		ypos = (source[0]&0x0fff0000)>>16;

		xflip = (source[0]&0x00008000)>>15;
		yflip = (source[0]&0x00004000)>>14;

		tileno = (source[1]&0x0001ffff);
		colr =   (source[1]&0x0ff00000)>>20;

		if(xpos&0x800)xpos-=0x1000;

		gfx->transpen(*m_sprite_bitmap,m_sprite_clip,tileno,colr,!xflip/*wrongdecode?*/,yflip,xpos+0x20-8/*-(m_spriteregs[0]&0x00000fff)*/,ypos-24/*-((m_spriteregs[1]&0x0fff0000)>>16)*/,15);
//      gfx->transpen(bitmap,cliprect,tileno,colr,!xflip/*wrongdecode?*/,yflip,xpos+0xa0-8/*-(m_spriteregs[0]&0x00000fff)*/,ypos-24+0x80/*-((m_spriteregs[1]&0x0fff0000)>>16)*/,0);


		source-=2;

	}

}

/* the sprite bitmap can probably be handled better than this ... */
void rabbit_state::clearspritebitmap( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/* clears a *sensible* amount of the sprite bitmap */
	int startx = (m_spriteregs[0]&0x00000fff);
	int starty = (m_spriteregs[1]&0x0fff0000)>>16;

	startx-=200;
	starty-=200;
	int amountx =650;
	int amounty =600;

	if (startx < 0) { amountx += startx; startx = 0; }
	if ((startx+amountx)>=0x1000) amountx-=(0x1000-(startx+amountx));

	for (int y=0; y<amounty;y++)
	{
		uint16_t *const dstline = &m_sprite_bitmap->pix((starty+y)&0xfff);
		memset(dstline+startx,0x00,amountx*2);
	}
}

/* todo: fix zoom, it's inaccurate and this code is ugly */
void rabbit_state::draw_sprite_bitmap( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint32_t xsize, ysize;
	uint32_t xstep,ystep;

	int startx = ((m_spriteregs[0]&0x00000fff));
	int starty = ((m_spriteregs[1]&0x0fff0000)>>16);

	/* zoom compensation? */
	startx-=((m_spriteregs[1]&0x000001ff)>>1);
	starty-=((m_spriteregs[1]&0x000001ff)>>1);


	xsize = ((m_spriteregs[2]&0x0000ffff));
	ysize = ((m_spriteregs[3]&0x0000ffff));
	xsize+=0x80;
	ysize+=0x80;
	xstep = ((320*128)<<16) / xsize;
	ystep = ((224*128)<<16) / ysize;
	for (uint32_t y=0;y<ysize;y+=0x80)
	{
		uint32_t ydrawpos = ((y>>7)*ystep);
		ydrawpos >>=16;

		if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
		{
			uint16_t const *const srcline = &m_sprite_bitmap->pix((starty+(y>>7))&0xfff);
			uint16_t *const dstline = &bitmap.pix(ydrawpos);

			for (uint32_t x=0;x<xsize;x+=0x80)
			{
				uint32_t xdrawpos = ((x>>7)*xstep);
				xdrawpos >>=16;
				uint16_t const pixdata = srcline[(startx+(x>>7))&0xfff];

				if (pixdata)
					if ((xdrawpos >= cliprect.min_x) && (xdrawpos <= cliprect.max_x))
						dstline[xdrawpos] = pixdata;
			}
		}
	}



}
void rabbit_state::video_start()
{
	/* the tilemaps are bigger than the regions the cpu can see, need to allocate the ram here */
	/* or maybe not for this game/hw .... */
	m_tilemap_ram[0] = make_unique_clear<uint32_t[]>(0x20000/4);
	m_tilemap_ram[1] = make_unique_clear<uint32_t[]>(0x20000/4);
	m_tilemap_ram[2] = make_unique_clear<uint32_t[]>(0x20000/4);
	m_tilemap_ram[3] = make_unique_clear<uint32_t[]>(0x20000/4);

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rabbit_state::get_tilemap0_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rabbit_state::get_tilemap1_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rabbit_state::get_tilemap2_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 32);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rabbit_state::get_tilemap3_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 128, 32);

	/* the tilemaps mix 4bpp and 8bbp tiles, we split these into 2 groups, and set a different transpen for each group */
	m_tilemap[0]->map_pen_to_layer(0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	m_tilemap[0]->map_pen_to_layer(1, 255, TILEMAP_PIXEL_TRANSPARENT);
	m_tilemap[1]->map_pen_to_layer(0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	m_tilemap[1]->map_pen_to_layer(1, 255, TILEMAP_PIXEL_TRANSPARENT);
	m_tilemap[2]->map_pen_to_layer(0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	m_tilemap[2]->map_pen_to_layer(1, 255, TILEMAP_PIXEL_TRANSPARENT);
	m_tilemap[3]->map_pen_to_layer(0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	m_tilemap[3]->map_pen_to_layer(1, 255, TILEMAP_PIXEL_TRANSPARENT);

	m_sprite_bitmap = std::make_unique<bitmap_ind16>(0x1000,0x1000);
	m_sprite_clip.set(0, 0x1000-1, 0, 0x1000-1);

	m_blit_done_timer = timer_alloc(FUNC(rabbit_state::blit_done), this);

	save_pointer(NAME(m_tilemap_ram[0]), 0x20000/4);
	save_pointer(NAME(m_tilemap_ram[1]), 0x20000/4);
	save_pointer(NAME(m_tilemap_ram[2]), 0x20000/4);
	save_pointer(NAME(m_tilemap_ram[3]), 0x20000/4);
}

/*

info based on rabbit

tilemap regs format (6 dwords, 1 set for each tilemap)

each line represents the differences on each tilemap for unknown variables
(tilemap)
(0) 0P660000 YYYYXXXX 0000**00 0000AAAA BBBB0000 00fx0000   ** = 00 during title, 8f during game, x = 0 during title, 1 during game
(1)     1                 00                        0
(2)     2                 00                        0
(3)   263                 00                        0
 0123456789abcdef = never change?

 P = priority
 Y = Yscroll
 X = Xscroll
 A = Yzoom (maybe X) (0800 = normal, 0C60 = fully zoomed out)
 B = Xzoom (maybe Y) (0800 = normal, 0C60 = fully zoomed out)

*/

void rabbit_state::drawtilemap( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int whichtilemap )
{
	int32_t startx, starty, incxx, incxy, incyx, incyy, tran;

	startx=((m_tilemap_regs[whichtilemap][1]&0x0000ffff));  // >>4 for nonzoomed pixel scroll value
	starty=((m_tilemap_regs[whichtilemap][1]&0xffff0000)>>16); // >> 20 for nonzoomed pixel scroll value
	incxx= ((m_tilemap_regs[whichtilemap][3]&0x00000fff)); // 0x800 when non-zoomed
	incyy= ((m_tilemap_regs[whichtilemap][4]&0x0fff0000)>>16);

	incxy = 0; incyx = 0;
	tran = 1;


	/* incxx and incyy and standard zoom, 16.16, a value of 0x10000 means no zoom
	   startx/starty are also 16.16 scrolling
	  */

	m_tilemap[whichtilemap]->draw_roz(screen, bitmap, cliprect, startx << 12,starty << 12,
			incxx << 5,incxy << 8,incyx << 8,incyy << 5,
			1,  /* wraparound */
			tran ? 0 : TILEMAP_DRAW_OPAQUE,0);
}

uint32_t rabbit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int prilevel;

	bitmap.fill(m_palette->black_pen(), cliprect);

//  popmessage("%08x %08x", m_viewregs0[0], m_viewregs0[1]);
//  popmessage("%08x %08x %08x %08x %08x %08x", m_tilemap_regs[0][0],m_tilemap_regs[0][1],m_tilemap_regs[0][2],m_tilemap_regs[0][3],m_tilemap_regs[0][4],m_tilemap_regs[0][5]);
//  popmessage("%08x %08x %08x %08x %08x %08x", m_tilemap_regs[1][0],m_tilemap_regs[1][1],m_tilemap_regs[1][2],m_tilemap_regs[1][3],m_tilemap_regs[1][4],m_tilemap_regs[1][5]);
//  popmessage("%08x %08x %08x %08x %08x %08x", m_tilemap_regs[2][0],m_tilemap_regs[2][1],m_tilemap_regs[2][2],m_tilemap_regs[2][3],m_tilemap_regs[2][4],m_tilemap_regs[2][5]);
//  popmessage("%08x %08x %08x %08x %08x %08x", m_tilemap_regs[3][0],m_tilemap_regs[3][1],m_tilemap_regs[3][2],m_tilemap_regs[3][3],m_tilemap_regs[3][4],m_tilemap_regs[3][5]);
//  popmessage("%08x %08x %08x %08x %08x %08x %08x", m_spriteregs[0],m_spriteregs[1],m_spriteregs[2],m_spriteregs[3],m_spriteregs[4],m_spriteregs[5], m_spriteregs[6]);
//  popmessage("%08x %08x %08x %08x %08x", m_viewregs6[0],m_viewregs6[1],m_viewregs6[2],m_viewregs6[3],m_viewregs6[4]);
//  popmessage("%08x", m_viewregs7[0]);
//  popmessage("%08x %08x %08x %08x", m_blitterregs[0],m_blitterregs[1],m_blitterregs[2],m_blitterregs[3]);
//  popmessage("%08x %08x %08x %08x", m_viewregs9[0],m_viewregs9[1],m_viewregs9[2],m_viewregs9[3]);

//  popmessage("%08x %08x %08x %08x %08x", m_viewregs10[0],m_viewregs10[1],m_viewregs10[2],m_viewregs10[3],m_viewregs10[4]);

	/* prio isnt certain but seems to work.. */
	for (prilevel = 0xf; prilevel >0; prilevel--)
	{
		if (prilevel == ((m_tilemap_regs[3][0]&0x0f000000)>>24)) drawtilemap(screen,bitmap,cliprect, 3);
		if (prilevel == ((m_tilemap_regs[2][0]&0x0f000000)>>24)) drawtilemap(screen,bitmap,cliprect, 2);
		if (prilevel == ((m_tilemap_regs[1][0]&0x0f000000)>>24)) drawtilemap(screen,bitmap,cliprect, 1);
		if (prilevel == ((m_tilemap_regs[0][0]&0x0f000000)>>24)) drawtilemap(screen,bitmap,cliprect, 0);

		if (prilevel == 0x09) // should it be selectable?
		{
			clearspritebitmap(bitmap,cliprect);
			draw_sprites(bitmap,cliprect);  // render to bitmap
			draw_sprite_bitmap(bitmap,cliprect); // copy bitmap to screen
		}
	}
	return 0;
}




uint32_t rabbit_state::tilemap0_r(offs_t offset)
{
	return m_tilemap_ram[0][offset];
}

uint32_t rabbit_state::tilemap1_r(offs_t offset)
{
	return m_tilemap_ram[1][offset];
}

uint32_t rabbit_state::tilemap2_r(offs_t offset)
{
	return m_tilemap_ram[2][offset];
}

uint32_t rabbit_state::tilemap3_r(offs_t offset)
{
	return m_tilemap_ram[3][offset];
}

uint32_t rabbit_state::randomrabbits()
{
	return machine().rand();
}

/* rom bank is used when testing roms, not currently hooked up */
void rabbit_state::rombank_w(uint32_t data)
{
	uint8_t *dataroms = memregion("gfx1")->base();
#if 0
	int bank;
	printf("rabbit rombank %08x\n",data);
	bank = data & 0x3ff;

	membank("bank1")->set_base(&dataroms[0x40000*(bank&0x3ff)]);
#else
	membank("bank1")->set_base(&dataroms[0]);
#endif

}


#define BLITCMDLOG 0
#define BLITLOG 0

TIMER_CALLBACK_MEMBER(rabbit_state::blit_done)
{
	m_maincpu->set_input_line(m_bltirqlevel, HOLD_LINE);
}

void rabbit_state::do_blit()
{
	uint8_t *blt_data = memregion("gfx1")->base();
	int blt_source = (m_blitterregs[0]&0x000fffff)>>0;
	int blt_column = (m_blitterregs[1]&0x00ff0000)>>16;
	int blt_line   = (m_blitterregs[1]&0x000000ff);
	int blt_tilemp = (m_blitterregs[2]&0x0000e000)>>13;
	int blt_oddflg = (m_blitterregs[2]&0x00000001)>>0;
	int mask,shift;


	if(BLITCMDLOG) osd_printf_debug("BLIT command %08x %08x %08x\n", m_blitterregs[0], m_blitterregs[1], m_blitterregs[2]);

	if (blt_oddflg&1)
	{
		mask = 0xffff0000;
		shift= 0;
	}
	else
	{
		mask = 0x0000ffff;
		shift= 16;
	}

	blt_oddflg>>=1; /* blt_oddflg is now in dword offsets*/
	blt_oddflg+=0x80*blt_line;

	blt_source<<=1; /* blitsource is in word offsets */

	while(1)
	{
		int blt_commnd = blt_data[blt_source+1];
		int blt_amount = blt_data[blt_source+0];
		int blt_value;
		int loopcount;
		int writeoffs;
		blt_source+=2;

		switch (blt_commnd)
		{
			case 0x00: /* copy nn bytes */
				if (!blt_amount)
				{
					if(BLITLOG) osd_printf_debug("end of blit list\n");
					m_blit_done_timer->adjust(attotime::from_usec(500));
					return;
				}

				if(BLITLOG) osd_printf_debug("blit copy %02x bytes\n", blt_amount);
				for (loopcount=0;loopcount<blt_amount;loopcount++)
				{
					blt_value = ((blt_data[blt_source+1]<<8)|(blt_data[blt_source+0]));
					blt_source+=2;
					writeoffs=blt_oddflg+blt_column;
					m_tilemap_ram[blt_tilemp][writeoffs]=(m_tilemap_ram[blt_tilemp][writeoffs]&mask)|(blt_value<<shift);
					m_tilemap[blt_tilemp]->mark_tile_dirty(writeoffs);

					blt_column++;
					blt_column&=0x7f;
				}

				break;

			case 0x02: /* fill nn bytes */
				if(BLITLOG) osd_printf_debug("blit fill %02x bytes\n", blt_amount);
				blt_value = ((blt_data[blt_source+1]<<8)|(blt_data[blt_source+0]));
				blt_source+=2;

				for (loopcount=0;loopcount<blt_amount;loopcount++)
				{
					writeoffs=blt_oddflg+blt_column;
					m_tilemap_ram[blt_tilemp][writeoffs]=(m_tilemap_ram[blt_tilemp][writeoffs]&mask)|(blt_value<<shift);
					m_tilemap[blt_tilemp]->mark_tile_dirty(writeoffs);
					blt_column++;
					blt_column&=0x7f;
				}

				break;

			case 0x03: /* next line */
				if(BLITLOG) osd_printf_debug("blit: move to next line\n");
				blt_column = (m_blitterregs[1]&0x00ff0000)>>16; /* --CC---- */
				blt_oddflg+=128;
				break;

			default: /* unknown / illegal */
				if(BLITLOG) osd_printf_debug("unknown blit command %02x\n",blt_commnd);
				break;
		}
	}

}



void rabbit_state::blitter_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_blitterregs[offset]);

	if (offset == 0x0c/4)
	{
		do_blit();
	}
}

void rabbit_state::eeprom_write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// don't disturb the EEPROM if we're not actually writing to it
	// (in particular, data & 0x100 here with mask = ffff00ff looks to be the watchdog)
	if (mem_mask == 0xff000000)
	{
		// latch the bit
		m_eeprom->di_write((data & 0x01000000) >> 24);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x04000000) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x02000000) ? ASSERT_LINE : CLEAR_LINE );
	}
}

void rabbit_state::rabbit_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x000000, 0x000003).nopw(); // bug in code / emulation?
	map(0x000010, 0x000013).nopw(); // bug in code / emulation?
	map(0x000024, 0x000027).nopw(); // bug in code / emulation?
	map(0x00719c, 0x00719f).nopw(); // bug in code / emulation?
	map(0x200000, 0x200003).portr("INPUTS").w(FUNC(rabbit_state::eeprom_write));
	map(0x400010, 0x400013).r(FUNC(rabbit_state::randomrabbits)); // gfx chip status?
	/* this lot are probably gfxchip/blitter etc. related */
	map(0x400010, 0x400013).writeonly().share("viewregs0");
	map(0x400100, 0x400117).writeonly().share("tilemap_regs.0"); // tilemap regs1
	map(0x400120, 0x400137).writeonly().share("tilemap_regs.1"); // tilemap regs2
	map(0x400140, 0x400157).writeonly().share("tilemap_regs.2"); // tilemap regs3
	map(0x400160, 0x400177).writeonly().share("tilemap_regs.3"); // tilemap regs4
	map(0x400200, 0x40021b).writeonly().share("spriteregs"); // sprregs?
	map(0x400300, 0x400303).w(FUNC(rabbit_state::rombank_w)); // used during rom testing, rombank/area select + something else?
	map(0x400400, 0x400413).writeonly().share("viewregs6"); // some global controls? (brightness etc.?)
	map(0x400500, 0x400503).writeonly().share("viewregs7");
	map(0x400700, 0x40070f).w(FUNC(rabbit_state::blitter_w)).share("blitterregs");
	map(0x400800, 0x40080f).writeonly().share("viewregs9"); // never changes?
	map(0x400900, 0x4009ff).rw("i5000snd", FUNC(i5000snd_device::read), FUNC(i5000snd_device::write));
	/* hmm */
	map(0x479700, 0x479713).writeonly().share("viewregs10");

	map(0x440000, 0x47ffff).bankr("bank1"); // data (gfx / sound) rom readback for ROM testing
	/* tilemaps */
	map(0x480000, 0x483fff).rw(FUNC(rabbit_state::tilemap0_r), FUNC(rabbit_state::tilemap0_w));
	map(0x484000, 0x487fff).rw(FUNC(rabbit_state::tilemap1_r), FUNC(rabbit_state::tilemap1_w));
	map(0x488000, 0x48bfff).rw(FUNC(rabbit_state::tilemap2_r), FUNC(rabbit_state::tilemap2_w));
	map(0x48c000, 0x48ffff).rw(FUNC(rabbit_state::tilemap3_r), FUNC(rabbit_state::tilemap3_w));
	map(0x494000, 0x497fff).ram().share("spriteram"); // sprites?
	map(0x4a0000, 0x4affff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xff0000, 0xffffff).ram();
}


static INPUT_PORTS_START( rabbit )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // as per code at 4d932
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unlabeled in input test
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout sprite_8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4, 24,28,16,20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout sprite_8x8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 8,0, 40,32,24,16,56, 48 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};



static const gfx_layout sprite_16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0,1,2,3 },
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+12,RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+24,RGN_FRAC(1,2)+28,RGN_FRAC(1,2)+16,RGN_FRAC(1,2)+20, 8,12,0,4, 24,28,16,20  },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
	16*32
};

static const gfx_layout sprite_16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+24,RGN_FRAC(1,2)+16,RGN_FRAC(1,2)+40,RGN_FRAC(1,2)+32,RGN_FRAC(1,2)+56,RGN_FRAC(1,2)+48, 8,0,24,16, 40,32,56,48  },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};

static const gfx_layout _16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8, 20,16,28,24,36,32,44,40,52,48,60,56 },
	{ 0*64, 1*64, 2*64,3*64,4*64,5*64,6*64,7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};

static const gfx_layout _8x8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24,32,40,48,56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};

static const gfx_layout _16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24,32,40,48,56, 64, 72, 80, 88, 96, 104, 112, 120 },
	{ 0*128, 1*128, 2*128,3*128,4*128,5*128,6*128,7*128,8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*128
};



static GFXDECODE_START( gfx_rabbit )
	/* this seems to be sprites */
	GFXDECODE_ENTRY( "gfx1", 0, sprite_8x8x4_layout,   0x0, 0x1000  )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_16x16x4_layout, 0x0, 0x1000  )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_8x8x8_layout,   0x0, 0x1000  ) // wrong
	GFXDECODE_ENTRY( "gfx1", 0, sprite_16x16x8_layout, 0x0, 0x1000  ) // wrong

	/* this seems to be backgrounds and tilemap gfx */
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_lsb,   0x0, 0x1000  )
	GFXDECODE_ENTRY( "gfx2", 0, _16x16x4_layout,        0x0, 0x1000  )
	GFXDECODE_ENTRY( "gfx2", 0, _8x8x8_layout,          0x0, 0x1000  )
	GFXDECODE_ENTRY( "gfx2", 0, _16x16x8_layout,        0x0, 0x1000  )

GFXDECODE_END

/* irq 6 = vblank
   irq 4 = blitter done?
   irq 5 = ?
   irq 1 = ?

*/

INTERRUPT_GEN_MEMBER(rabbit_state::vblank_interrupt)
{
	m_maincpu->set_input_line(m_vblirqlevel, HOLD_LINE);
}

void rabbit_state::rabbit(machine_config &config)
{
	M68EC020(config, m_maincpu, XTAL(24'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &rabbit_state::rabbit_map);
	m_maincpu->set_vblank_int("screen", FUNC(rabbit_state::vblank_interrupt));

	EEPROM_93C46_16BIT(config, m_eeprom);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rabbit);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*16, 64*16);
	screen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
//  screen.set_visarea(0*8, 64*16-1, 0*16, 64*16-1);
//  screen.set_visarea(0*8, 20*16-1, 32*16, 48*16-1);
	screen.set_screen_update(FUNC(rabbit_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xGRB_888, 0x4000);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	i5000snd_device &i5000snd(I5000_SND(config, "i5000snd", XTAL(40'000'000)));
	i5000snd.add_route(0, "rspeaker", 1.0);
	i5000snd.add_route(1, "lspeaker", 1.0);
}



ROM_START( rabbit ) // This is the Asian version sold in Korea but the devs forgot to update the region disclaimer.
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "jpr0.0", 0x000000, 0x080000, CRC(52bb18c0) SHA1(625bc8a4daa6d08cacd92d9110cf67a95a91325a) )
	ROM_LOAD32_BYTE( "jpr1.1", 0x000001, 0x080000, CRC(38299d0d) SHA1(72ccd51781b47636bb16ac18037cb3121d17199f) )
	ROM_LOAD32_BYTE( "jpr2.2", 0x000002, 0x080000, CRC(fa3fd91a) SHA1(ac0e658af30b37b752ede833b44ff5423b93bdb1) )
	ROM_LOAD32_BYTE( "jpr3.3", 0x000003, 0x080000, CRC(d22727ca) SHA1(8415cb2d3864b11fe5623ac65f2e28fd62c61bd1) )


// the rom test tests as if things were mapped like this (video chip / blitter space?)
#if 0
	ROM_REGION( 0x9000000, "test", ROMREGION_ERASE )
	ROM_LOAD32_WORD( "jfv0.00", 0x0000002, 0x400000, CRC(b2a4d3d3) SHA1(0ab71d82a37ff94442b91712a28d3470619ba575) ) // sprite gfx
	ROM_LOAD32_WORD( "jfv1.01", 0x0000000, 0x400000, CRC(83f3926e) SHA1(b1c479e675d35fc08c9a7648ff40348a24654e7e) ) // sprite gfx
	ROM_LOAD32_WORD( "jsn0.11", 0x0800002, 0x400000, CRC(e1f726e8) SHA1(598d75f3ff9e43ec8ce6131ed37f4345bf2f2d8e) ) // sound
	ROM_LOAD32_WORD( "jfv2.02", 0x2000002, 0x400000, CRC(b264bfb5) SHA1(8fafedb6af74150465b1773e80aef0edc3da4678) ) // sprite gfx
	ROM_LOAD32_WORD( "jfv3.03", 0x2000000, 0x400000, CRC(3e1a9be2) SHA1(2082a4ae8cda84cec5ea0fc08753db387bb70d41) ) // sprite gfx
	ROM_LOAD16_BYTE( "jbg0.40", 0x4000001, 0x200000, CRC(89662944) SHA1(ca916ba38480fa588af19fc9682603f5195ad6c7) ) // bg gfx (fails check, but dumped from two different boards)
	ROM_LOAD16_BYTE( "jbg1.50", 0x6000000, 0x200000, CRC(1fc7f6e0) SHA1(b36062d2a9683683ffffd3003d5244a185f53280) ) // bg gfx
	ROM_LOAD16_BYTE( "jbg2.60", 0x8000001, 0x200000, CRC(aee265fc) SHA1(ec420ab30b9b5141162223fc1fbf663ad9f211e6) ) // bg gfx
#endif

	ROM_REGION( 0x1000000, "gfx1", 0 ) /* Sprite Roms (and Blitter Data) */
	ROM_LOAD32_WORD( "jfv0.00", 0x0000002, 0x400000, CRC(b2a4d3d3) SHA1(0ab71d82a37ff94442b91712a28d3470619ba575) )
	ROM_LOAD32_WORD( "jfv1.01", 0x0000000, 0x400000, CRC(83f3926e) SHA1(b1c479e675d35fc08c9a7648ff40348a24654e7e) )
	ROM_LOAD32_WORD( "jfv2.02", 0x0800002, 0x400000, CRC(b264bfb5) SHA1(8fafedb6af74150465b1773e80aef0edc3da4678) )
	ROM_LOAD32_WORD( "jfv3.03", 0x0800000, 0x400000, CRC(3e1a9be2) SHA1(2082a4ae8cda84cec5ea0fc08753db387bb70d41) )


	ROM_REGION( 0x600000, "gfx2", 0 ) /* BG Roms */
	ROM_LOAD( "jbg0.40", 0x000000, 0x200000, CRC(89662944) SHA1(ca916ba38480fa588af19fc9682603f5195ad6c7) )
	ROM_LOAD( "jbg1.50", 0x200000, 0x200000, CRC(1fc7f6e0) SHA1(b36062d2a9683683ffffd3003d5244a185f53280) )
	ROM_LOAD( "jbg2.60", 0x400000, 0x200000, CRC(aee265fc) SHA1(ec420ab30b9b5141162223fc1fbf663ad9f211e6) )

	ROM_REGION( 0x400000, "i5000snd", ROMREGION_ERASE ) /* sound rom */
	ROM_LOAD( "jsn0.11", 0x000000, 0x000018, CRC(e1f726e8) SHA1(598d75f3ff9e43ec8ce6131ed37f4345bf2f2d8e) ) // header "VCDT i5000"
	ROM_CONTINUE(        0x000000, 0x3fffe8 ) // sample data starts here

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "rabbit.nv", 0x0000, 0x0080, CRC(73d471ed) SHA1(45e045f5ea9036342b88013e021d402741d98537) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "epm7032.u1",   0x0000, 0x0798, CRC(bb1c930e) SHA1(7513ed6a0d797276dab1e3446fe346a9c340e69d) ) // unprotected
ROM_END

ROM_START( rabbita ) // Sourced from a Taiwanese parts sellers, is this another Asian version?
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "pv00_mst.u82", 0x000000, 0x080000, CRC(e23e738e) SHA1(d514f3085401db7221661c3936d1fbbc5ec94b06) ) // English text, handwritten labels, but copyright 1996
	ROM_LOAD32_BYTE( "pv01_mst.u84", 0x000001, 0x080000, CRC(8c7ff335) SHA1(ca636dc6545d453645476df8fe6e18bc71037ab8) ) // is this a 1/28 build like the rabbitjt set?
	ROM_LOAD32_BYTE( "pv02_mst.u83", 0x000002, 0x080000, CRC(c4bee278) SHA1(c20bfc3d90ae089ff029b328a344b7c3804ed228) )
	ROM_LOAD32_BYTE( "pv03_mst.u85", 0x000003, 0x080000, CRC(062f524e) SHA1(566bc3aa177ceb11e9558e212ea3ae11070f2db2) )


// the rom test tests as if things were mapped like this (video chip / blitter space?)
#if 0
	ROM_REGION( 0x9000000, "test", ROMREGION_ERASE )
	ROM_LOAD32_WORD( "jfv0.00", 0x0000002, 0x400000, CRC(b2a4d3d3) SHA1(0ab71d82a37ff94442b91712a28d3470619ba575) ) // sprite gfx
	ROM_LOAD32_WORD( "jfv1.01", 0x0000000, 0x400000, CRC(83f3926e) SHA1(b1c479e675d35fc08c9a7648ff40348a24654e7e) ) // sprite gfx
	ROM_LOAD32_WORD( "jsn0.11", 0x0800002, 0x400000, CRC(e1f726e8) SHA1(598d75f3ff9e43ec8ce6131ed37f4345bf2f2d8e) ) // sound
	ROM_LOAD32_WORD( "jfv2.02", 0x2000002, 0x400000, CRC(b264bfb5) SHA1(8fafedb6af74150465b1773e80aef0edc3da4678) ) // sprite gfx
	ROM_LOAD32_WORD( "jfv3.03", 0x2000000, 0x400000, CRC(3e1a9be2) SHA1(2082a4ae8cda84cec5ea0fc08753db387bb70d41) ) // sprite gfx
	ROM_LOAD16_BYTE( "jbg0.40", 0x4000001, 0x200000, CRC(89662944) SHA1(ca916ba38480fa588af19fc9682603f5195ad6c7) ) // bg gfx (fails check, but dumped from two different boards)
	ROM_LOAD16_BYTE( "jbg1.50", 0x6000000, 0x200000, CRC(1fc7f6e0) SHA1(b36062d2a9683683ffffd3003d5244a185f53280) ) // bg gfx
	ROM_LOAD16_BYTE( "jbg2.60", 0x8000001, 0x200000, CRC(aee265fc) SHA1(ec420ab30b9b5141162223fc1fbf663ad9f211e6) ) // bg gfx
#endif

	ROM_REGION( 0x1000000, "gfx1", 0 ) /* Sprite Roms (and Blitter Data) */
	ROM_LOAD32_WORD( "jfv0.00", 0x0000002, 0x400000, CRC(b2a4d3d3) SHA1(0ab71d82a37ff94442b91712a28d3470619ba575) )
	ROM_LOAD32_WORD( "jfv1.01", 0x0000000, 0x400000, CRC(83f3926e) SHA1(b1c479e675d35fc08c9a7648ff40348a24654e7e) )
	ROM_LOAD32_WORD( "jfv2.02", 0x0800002, 0x400000, CRC(b264bfb5) SHA1(8fafedb6af74150465b1773e80aef0edc3da4678) )
	ROM_LOAD32_WORD( "jfv3.03", 0x0800000, 0x400000, CRC(3e1a9be2) SHA1(2082a4ae8cda84cec5ea0fc08753db387bb70d41) )


	ROM_REGION( 0x600000, "gfx2", 0 ) /* BG Roms */
	ROM_LOAD( "jbg0.40", 0x000000, 0x200000, CRC(89662944) SHA1(ca916ba38480fa588af19fc9682603f5195ad6c7) )
	ROM_LOAD( "jbg1.50", 0x200000, 0x200000, CRC(1fc7f6e0) SHA1(b36062d2a9683683ffffd3003d5244a185f53280) )
	ROM_LOAD( "jbg2.60", 0x400000, 0x200000, CRC(aee265fc) SHA1(ec420ab30b9b5141162223fc1fbf663ad9f211e6) )

	ROM_REGION( 0x400000, "i5000snd", ROMREGION_ERASE ) /* sound rom */
	ROM_LOAD( "jsn0.11", 0x000000, 0x000018, CRC(e1f726e8) SHA1(598d75f3ff9e43ec8ce6131ed37f4345bf2f2d8e) ) // header "VCDT i5000"
	ROM_CONTINUE(        0x000000, 0x3fffe8 ) // sample data starts here

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "rabbit.nv", 0x0000, 0x0080, CRC(73d471ed) SHA1(45e045f5ea9036342b88013e021d402741d98537) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "epm7032.u1",   0x0000, 0x0798, CRC(bb1c930e) SHA1(7513ed6a0d797276dab1e3446fe346a9c340e69d) ) // unprotected
ROM_END

ROM_START( rabbitj )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68020 Code, handwritten labels */
	ROM_LOAD32_BYTE( "pvo0.u82", 0x000000, 0x080000, CRC(56ee441b) SHA1(07a02cf59a50f47f5cbdc8c624730d98c4fff48f) ) // Japanese text, no labels, unknown build date but copyright 1997 like
	ROM_LOAD32_BYTE( "pvo1.u84", 0x000001, 0x080000, CRC(7d8addc5) SHA1(45203a8651b9701735df8e6f3827c3440b4aa2f9) ) // the above Asia (parent) set, so it's newer then the location test below
	ROM_LOAD32_BYTE( "pvo2.u83", 0x000002, 0x080000, CRC(4a333d89) SHA1(7c7859b1bcfe4c8ba408f00ce369756db804548a) )
	ROM_LOAD32_BYTE( "pvo3.u85", 0x000003, 0x080000, CRC(791bf835) SHA1(3357af5619391323620e0c0b89a4b108f7772db7) )


// the rom test tests as if things were mapped like this (video chip / blitter space?)
#if 0
	ROM_REGION( 0x9000000, "test", ROMREGION_ERASE )
	ROM_LOAD32_WORD( "jfv0.00", 0x0000002, 0x400000, CRC(b2a4d3d3) SHA1(0ab71d82a37ff94442b91712a28d3470619ba575) ) // sprite gfx
	ROM_LOAD32_WORD( "jfv1.01", 0x0000000, 0x400000, CRC(83f3926e) SHA1(b1c479e675d35fc08c9a7648ff40348a24654e7e) ) // sprite gfx
	ROM_LOAD32_WORD( "jsn0.11", 0x0800002, 0x400000, CRC(e1f726e8) SHA1(598d75f3ff9e43ec8ce6131ed37f4345bf2f2d8e) ) // sound
	ROM_LOAD32_WORD( "jfv2.02", 0x2000002, 0x400000, CRC(b264bfb5) SHA1(8fafedb6af74150465b1773e80aef0edc3da4678) ) // sprite gfx
	ROM_LOAD32_WORD( "jfv3.03", 0x2000000, 0x400000, CRC(3e1a9be2) SHA1(2082a4ae8cda84cec5ea0fc08753db387bb70d41) ) // sprite gfx
	ROM_LOAD16_BYTE( "jbg0.40", 0x4000001, 0x200000, CRC(89662944) SHA1(ca916ba38480fa588af19fc9682603f5195ad6c7) ) // bg gfx (fails check, but dumped from two different boards)
	ROM_LOAD16_BYTE( "jbg1.50", 0x6000000, 0x200000, CRC(1fc7f6e0) SHA1(b36062d2a9683683ffffd3003d5244a185f53280) ) // bg gfx
	ROM_LOAD16_BYTE( "jbg2.60", 0x8000001, 0x200000, CRC(aee265fc) SHA1(ec420ab30b9b5141162223fc1fbf663ad9f211e6) ) // bg gfx
#endif

	ROM_REGION( 0x1000000, "gfx1", 0 ) /* Sprite Roms (and Blitter Data) */
	ROM_LOAD32_WORD( "jfv0.00", 0x0000002, 0x400000, CRC(b2a4d3d3) SHA1(0ab71d82a37ff94442b91712a28d3470619ba575) )
	ROM_LOAD32_WORD( "jfv1.01", 0x0000000, 0x400000, CRC(83f3926e) SHA1(b1c479e675d35fc08c9a7648ff40348a24654e7e) )
	ROM_LOAD32_WORD( "jfv2.02", 0x0800002, 0x400000, CRC(b264bfb5) SHA1(8fafedb6af74150465b1773e80aef0edc3da4678) )
	ROM_LOAD32_WORD( "jfv3.03", 0x0800000, 0x400000, CRC(3e1a9be2) SHA1(2082a4ae8cda84cec5ea0fc08753db387bb70d41) )


	ROM_REGION( 0x600000, "gfx2", 0 ) /* BG Roms */
	ROM_LOAD( "jbg0.40", 0x000000, 0x200000, CRC(89662944) SHA1(ca916ba38480fa588af19fc9682603f5195ad6c7) )
	ROM_LOAD( "jbg1.50", 0x200000, 0x200000, CRC(1fc7f6e0) SHA1(b36062d2a9683683ffffd3003d5244a185f53280) )
	ROM_LOAD( "jbg2.60", 0x400000, 0x200000, CRC(aee265fc) SHA1(ec420ab30b9b5141162223fc1fbf663ad9f211e6) )

	ROM_REGION( 0x400000, "i5000snd", ROMREGION_ERASE ) /* sound rom */
	ROM_LOAD( "jsn0.11", 0x000000, 0x000018, CRC(e1f726e8) SHA1(598d75f3ff9e43ec8ce6131ed37f4345bf2f2d8e) ) // header "VCDT i5000"
	ROM_CONTINUE(        0x000000, 0x3fffe8 ) // sample data starts here

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "rabbit.nv", 0x0000, 0x0080, CRC(73d471ed) SHA1(45e045f5ea9036342b88013e021d402741d98537) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "epm7032.u1",   0x0000, 0x0798, CRC(bb1c930e) SHA1(7513ed6a0d797276dab1e3446fe346a9c340e69d) ) // unprotected
ROM_END

ROM_START( rabbitjt )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68020 Code, handwritten labels */
	ROM_LOAD32_BYTE( "pvo0_mst_1-28.u82", 0x000000, 0x080000, BAD_DUMP CRC(a1c30c91) SHA1(fe35c5521acba902ca74c6d1ea2b92593138a10a) ) // This ROM fails the built in MEMORY (checksum) TEST
	ROM_LOAD32_BYTE( "pvo1_mst_1-28.u84", 0x000001, 0x080000, CRC(9b7697e6) SHA1(c60cdc3db7321b2846637d92ae864ca80796ad73) ) // actually 1/28, presumably build date
	ROM_LOAD32_BYTE( "pvo2_mst_1-28.u83", 0x000002, 0x080000, CRC(9809b825) SHA1(05f965c87782062e66e5a88313a12b230de90800) )
	ROM_LOAD32_BYTE( "pvo3_mst_1-28.u85", 0x000003, 0x080000, CRC(ce8ebb82) SHA1(69eb8410a3b7d9cbb682d0911bcad1a92ac3aca7) )


// the rom test tests as if things were mapped like this (video chip / blitter space?)
#if 0
	ROM_REGION( 0x9000000, "test", ROMREGION_ERASE )
	ROM_LOAD32_WORD( "jfv0.00", 0x0000002, 0x400000, CRC(b2a4d3d3) SHA1(0ab71d82a37ff94442b91712a28d3470619ba575) ) // sprite gfx
	ROM_LOAD32_WORD( "jfv1.01", 0x0000000, 0x400000, CRC(83f3926e) SHA1(b1c479e675d35fc08c9a7648ff40348a24654e7e) ) // sprite gfx
	ROM_LOAD32_WORD( "jsn0.11", 0x0800002, 0x400000, CRC(e1f726e8) SHA1(598d75f3ff9e43ec8ce6131ed37f4345bf2f2d8e) ) // sound
	ROM_LOAD32_WORD( "jfv2.02", 0x2000002, 0x400000, CRC(b264bfb5) SHA1(8fafedb6af74150465b1773e80aef0edc3da4678) ) // sprite gfx
	ROM_LOAD32_WORD( "jfv3.03", 0x2000000, 0x400000, CRC(3e1a9be2) SHA1(2082a4ae8cda84cec5ea0fc08753db387bb70d41) ) // sprite gfx
	ROM_LOAD16_BYTE( "jbg0.40", 0x4000001, 0x200000, CRC(89662944) SHA1(ca916ba38480fa588af19fc9682603f5195ad6c7) ) // bg gfx (fails check, but dumped from two different boards)
	ROM_LOAD16_BYTE( "jbg1.50", 0x6000000, 0x200000, CRC(1fc7f6e0) SHA1(b36062d2a9683683ffffd3003d5244a185f53280) ) // bg gfx
	ROM_LOAD16_BYTE( "jbg2.60", 0x8000001, 0x200000, CRC(aee265fc) SHA1(ec420ab30b9b5141162223fc1fbf663ad9f211e6) ) // bg gfx
#endif

	ROM_REGION( 0x1000000, "gfx1", 0 ) /* Sprite Roms (and Blitter Data) */
	ROM_LOAD32_WORD( "jfv0.00", 0x0000002, 0x400000, CRC(b2a4d3d3) SHA1(0ab71d82a37ff94442b91712a28d3470619ba575) )
	ROM_LOAD32_WORD( "jfv1.01", 0x0000000, 0x400000, CRC(83f3926e) SHA1(b1c479e675d35fc08c9a7648ff40348a24654e7e) )
	ROM_LOAD32_WORD( "jfv2.02", 0x0800002, 0x400000, CRC(b264bfb5) SHA1(8fafedb6af74150465b1773e80aef0edc3da4678) )
	ROM_LOAD32_WORD( "jfv3.03", 0x0800000, 0x400000, CRC(3e1a9be2) SHA1(2082a4ae8cda84cec5ea0fc08753db387bb70d41) )


	ROM_REGION( 0x600000, "gfx2", 0 ) /* BG Roms */
	ROM_LOAD( "jbg0.40", 0x000000, 0x200000, CRC(89662944) SHA1(ca916ba38480fa588af19fc9682603f5195ad6c7) )
	ROM_LOAD( "jbg1.50", 0x200000, 0x200000, CRC(1fc7f6e0) SHA1(b36062d2a9683683ffffd3003d5244a185f53280) )
	ROM_LOAD( "jbg2.60", 0x400000, 0x200000, CRC(aee265fc) SHA1(ec420ab30b9b5141162223fc1fbf663ad9f211e6) )

	ROM_REGION( 0x400000, "i5000snd", ROMREGION_ERASE ) /* sound rom */
	ROM_LOAD( "jsn0.11", 0x000000, 0x000018, CRC(e1f726e8) SHA1(598d75f3ff9e43ec8ce6131ed37f4345bf2f2d8e) ) // header "VCDT i5000"
	ROM_CONTINUE(        0x000000, 0x3fffe8 ) // sample data starts here

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "rabbit.nv", 0x0000, 0x0080, CRC(73d471ed) SHA1(45e045f5ea9036342b88013e021d402741d98537) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "epm7032.u1",   0x0000, 0x0798, CRC(bb1c930e) SHA1(7513ed6a0d797276dab1e3446fe346a9c340e69d) ) // unprotected
ROM_END

} // anonymous namespace


GAME( 1997, rabbit,   0,      rabbit,  rabbit, rabbit_state, empty_init, ROT0, "Aorn / Electronic Arts", "Rabbit (Asia 3/6)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // For use in Japan notice, English text, (C) 1997
GAME( 1996, rabbita,  rabbit, rabbit,  rabbit, rabbit_state, empty_init, ROT0, "Aorn / Electronic Arts", "Rabbit (Asia 1/28?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // For use in Japan notice, English text, (C) 1996
GAME( 1997, rabbitj,  rabbit, rabbit,  rabbit, rabbit_state, empty_init, ROT0, "Aorn / Electronic Arts", "Rabbit (Japan 3/6?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // For use in Japan notice, Japanese text, (C) 1997
GAME( 1996, rabbitjt, rabbit, rabbit,  rabbit, rabbit_state, empty_init, ROT0, "Aorn / Electronic Arts", "Rabbit (Japan 1/28, location test)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // For use in Japan notice, Japanese text, (C) 1996
