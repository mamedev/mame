#include "emu.h"
#include "includes/gstriker.h"


/*** VS920A (score tilemap) **********************************************/

/*

    VS920A - (Very) Simple tilemap chip
    -----------------------------------

- 1 Plane
- Tiles 8x8, 4bpp
- Map 64x64
- No scrolling or other effects (at least in gstriker)


    Videoram format:
    ----------------

pppp tttt tttt tttt

t=tile, p=palette

*/


TILE_GET_INFO_MEMBER(gstriker_state::VS920A_get_tile_info)
{
	int data;
	int tileno, pal;

	data = m_VS920A_cur_chip->vram[tile_index];

	tileno = data & 0xFFF;
	pal =   (data >> 12) & 0xF;

	SET_TILE_INFO_MEMBER(m_VS920A_cur_chip->gfx_region, tileno, m_VS920A_cur_chip->pal_base + pal, 0);
}

WRITE16_MEMBER(gstriker_state::VS920A_0_vram_w)
{
	COMBINE_DATA(&m_VS920A[0].vram[offset]);
	m_VS920A[0].tmap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(gstriker_state::VS920A_1_vram_w)
{
	COMBINE_DATA(&m_VS920A[1].vram[offset]);
	m_VS920A[1].tmap->mark_tile_dirty(offset);
}

void gstriker_state::VS920A_init(int numchips)
{
	int i;

	if (numchips > MAX_VS920A)
		numchips = MAX_VS920A;

	for (i=0;i<numchips;i++)
	{
		m_VS920A[i].tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gstriker_state::VS920A_get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);

		m_VS920A[i].tmap->set_transparent_pen(0);
	}
}

tilemap_t* gstriker_state::VS920A_get_tilemap(int numchip)
{
	return m_VS920A[numchip].tmap;
}

void gstriker_state::VS920A_set_pal_base(int numchip, int pal_base)
{
	m_VS920A[numchip].pal_base = pal_base;
}

void gstriker_state::VS920A_set_gfx_region(int numchip, int gfx_region)
{
	m_VS920A[numchip].gfx_region = gfx_region;
}

void gstriker_state::VS920A_draw(int numchip, screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority)
{
	m_VS920A_cur_chip = &m_VS920A[numchip];

	m_VS920A_cur_chip->tmap->draw(screen, bitmap, cliprect, 0, priority);
}



/*** Fujitsu MB60553 (screen tilemap) **********************************************/

/*

    Fujitsu MB60553 - Tilemap chip
    ------------------------------

- 1 Plane
- Tiles 16x16, 4bpp
- Map 64x64
- Scrolling
- Indexed banking (8 banks)
- Surely another effect like roz/tilezoom, yet to be implemented


    Videoram format
    ---------------

pppp bbbt tttt tttt

t=tile, b=bank, p=palette


    Registers
    ---------

0 - Scroll X
Fixed point 12.4 (seems wrong)

1 - Scroll Y
Fixed point 12.4 (seems wrong)

2 - ????

3 - ????

4 - Tilebank #0/#1   ---a aaaa  ---b bbbb
5 - Tilebank #2/#3   ---a aaaa  ---b bbbb
6 - Tilebank #4/#5   ---a aaaa  ---b bbbb
7 - Tilebank #6/#7   ---a aaaa  ---b bbbb

Indexed tilebank. Each bank is 0x200 tiles wide. Notice that within each register, the bank with the lower
index is in the MSB. gstriker uses 5 bits for banking, but the chips could be able to do more.

*/



TILE_GET_INFO_MEMBER(gstriker_state::MB60553_get_tile_info)
{
	int data, bankno;
	int tileno, pal;

	data = m_MB60553_cur_chip->vram[tile_index];

	tileno = data & 0x1FF;
	pal = (data >> 12) & 0xF;
	bankno = (data >> 9) & 0x7;

	SET_TILE_INFO_MEMBER(m_MB60553->gfx_region, tileno + m_MB60553_cur_chip->bank[bankno] * 0x200, pal + m_MB60553->pal_base, 0);
}

void gstriker_state::MB60553_reg_written(int numchip, int num_reg)
{
	tMB60553* cur = &m_MB60553[numchip];

	switch (num_reg)
	{
	case 0:
		cur->tmap->set_scrollx(0, cur->regs[0]>>4);
		break;

	case 1:
		cur->tmap->set_scrolly(0, cur->regs[1]>>4);
		break;

	case 2:
		mame_printf_debug("MB60553_reg chip %d, reg 2 %04x\n",numchip, cur->regs[2]);
		break;

	case 3:
		mame_printf_debug("MB60553_reg chip %d, reg 3 %04x\n",numchip, cur->regs[3]);
		break;

	case 4:
		cur->bank[0] = (cur->regs[4] >> 8) & 0x1F;
		cur->bank[1] = (cur->regs[4] >> 0) & 0x1F;
		cur->tmap->mark_all_dirty();
		break;

	case 5:
		cur->bank[2] = (cur->regs[5] >> 8) & 0x1F;
		cur->bank[3] = (cur->regs[5] >> 0) & 0x1F;
		cur->tmap->mark_all_dirty();
		break;

	case 6:
		cur->bank[4] = (cur->regs[6] >> 8) & 0x1F;
		cur->bank[5] = (cur->regs[6] >> 0) & 0x1F;
		cur->tmap->mark_all_dirty();
		break;

	case 7:
		cur->bank[6] = (cur->regs[7] >> 8) & 0x1F;
		cur->bank[7] = (cur->regs[7] >> 0) & 0x1F;
		cur->tmap->mark_all_dirty();
		break;
	}
}

/* twc94 has the tilemap made of 2 pages .. it needs this */
TILEMAP_MAPPER_MEMBER(gstriker_state::twc94_scan)
{
	/* logical (col,row) -> memory offset */
	return (row*64) + (col&63) + ((col&64)<<6);
}

void gstriker_state::MB60553_init(int numchips)
{
	int i;

	if (numchips > MAX_MB60553)
		numchips = MAX_MB60553;

	for (i=0;i<numchips;i++)
	{
		m_MB60553[i].tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gstriker_state::MB60553_get_tile_info),this),tilemap_mapper_delegate(FUNC(gstriker_state::twc94_scan),this), 16,16,128,64);

		m_MB60553[i].tmap->set_transparent_pen(0);
	}
}

void gstriker_state::MB60553_set_pal_base(int numchip, int pal_base)
{
	m_MB60553[numchip].pal_base = pal_base;
}

void gstriker_state::MB60553_set_gfx_region(int numchip, int gfx_region)
{
	m_MB60553[numchip].gfx_region = gfx_region;
}

/* THIS IS STILL WRONG! */
void gstriker_state::MB60553_draw(int numchip, screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority)
{
	int line;
	rectangle clip;
	m_MB60553_cur_chip = &m_MB60553[numchip];

	clip.min_x = m_screen->visible_area().min_x;
	clip.max_x = m_screen->visible_area().max_x;

	for (line = 0; line < 224;line++)
	{
//      int scrollx;
//      int scrolly;

		UINT32 startx,starty;

		UINT32 incxx,incyy;

		startx = m_MB60553_cur_chip->regs[0];
		starty = m_MB60553_cur_chip->regs[1];

		startx += (24<<4); // maybe not..

		startx -=  m_lineram[(line)*8+7]/2;

		incxx = m_lineram[(line)*8+0]<<4;
		incyy = m_lineram[(line)*8+3]<<4;

		clip.min_y = clip.max_y = line;

		m_MB60553_cur_chip->tmap->draw_roz(screen, bitmap, clip, startx<<12,starty<<12,
				incxx,0,0,incyy,
				1,
				0,priority);

	}



}

tilemap_t* gstriker_state::MB60553_get_tilemap(int numchip)
{
	return m_MB60553[numchip].tmap;
}


WRITE16_MEMBER(gstriker_state::MB60553_0_regs_w)
{
	UINT16 oldreg = m_MB60553[0].regs[offset];

	COMBINE_DATA(&m_MB60553[0].regs[offset]);

	if (m_MB60553[0].regs[offset] != oldreg)
		MB60553_reg_written(0, offset);
}

WRITE16_MEMBER(gstriker_state::MB60553_1_regs_w)
{
	UINT16 oldreg = m_MB60553[1].regs[offset];

	COMBINE_DATA(&m_MB60553[1].regs[offset]);

	if (m_MB60553[1].regs[offset] != oldreg)
		MB60553_reg_written(1, offset);
}

WRITE16_MEMBER(gstriker_state::MB60553_0_vram_w)
{
	COMBINE_DATA(&m_MB60553[0].vram[offset]);

	m_MB60553[0].tmap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(gstriker_state::MB60553_1_vram_w)
{
	COMBINE_DATA(&m_MB60553[1].vram[offset]);

	m_MB60553[1].tmap->mark_tile_dirty(offset);
}





/*** VIDEO UPDATE/START **********************************************/


#ifdef UNUSED_FUNCTION
WRITE16_MEMBER(gstriker_state::gsx_videoram3_w)
{
	// This memory appears to be empty in gstriker
	gs_videoram3[offset] = data;
}
#endif


UINT32 gstriker_state::screen_update_gstriker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	// Sandwitched screen/sprite0/score/sprite1. Surely wrong, probably
	//  needs sprite orthogonality
	MB60553_draw(0, screen, bitmap,cliprect, 0);

	m_spr->draw_sprites(m_CG10103_vram, 0x2000, screen, bitmap, cliprect, 0x2, 0x0);

	VS920A_draw(0, screen, bitmap, cliprect, 0);

	m_spr->draw_sprites(m_CG10103_vram, 0x2000, screen, bitmap, cliprect, 0x2, 0x2);

#if 0
	popmessage("%04x %04x %04x %04x %04x %04x %04x %04x",
		(UINT16)m_MB60553[0].regs[0], (UINT16)m_MB60553[0].regs[1], (UINT16)m_MB60553[0].regs[2], (UINT16)m_MB60553[0].regs[3],
		(UINT16)m_MB60553[0].regs[4], (UINT16)m_MB60553[0].regs[5], (UINT16)m_MB60553[0].regs[6], (UINT16)m_MB60553[0].regs[7]
	);
#endif

#if 0
	popmessage("%04x %04x %04x %04x %04x %04x %04x %04x",
		(UINT16)gs_mixer_regs[8], (UINT16)gs_mixer_regs[9], (UINT16)gs_mixer_regs[10], (UINT16)gs_mixer_regs[11],
		(UINT16)gs_mixer_regs[12], (UINT16)gs_mixer_regs[13], (UINT16)gs_mixer_regs[14], (UINT16)gs_mixer_regs[15]
	);
#endif
	return 0;
}

VIDEO_START_MEMBER(gstriker_state,gstriker)
{
	// Palette bases are hardcoded, but should be probably extracted from the mixer registers

	// Initalize the chip for the score plane
	VS920A_init(1);
	VS920A_set_gfx_region(0, 0);
	VS920A_set_pal_base(0, 0x30);
	VS920A_get_tilemap(0)->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	MB60553_init(1);
	MB60553_set_gfx_region(0, 1);
	MB60553_set_pal_base(0, 0);
	MB60553_get_tilemap(0)->set_transparent_pen(0xf);
}

VIDEO_START_MEMBER(gstriker_state,twrldc94)
{
	// Palette bases are hardcoded, but should be probably extracted from the mixer registers

	// Initalize the chip for the score plane
	VS920A_init(1);
	VS920A_set_gfx_region(0, 0);
	VS920A_set_pal_base(0, 0x40);
	VS920A_get_tilemap(0)->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	MB60553_init(1);
	MB60553_set_gfx_region(0, 1);
	MB60553_set_pal_base(0, 0x50);
	MB60553_get_tilemap(0)->set_transparent_pen(0xf);
}

VIDEO_START_MEMBER(gstriker_state,vgoalsoc)
{
	// Palette bases are hardcoded, but should be probably extracted from the mixer registers

	// Initalize the chip for the score plane
	VS920A_init(1);
	VS920A_set_gfx_region(0, 0);
	VS920A_set_pal_base(0, 0x30);
	VS920A_get_tilemap(0)->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	MB60553_init(1);
	MB60553_set_gfx_region(0, 1);
	MB60553_set_pal_base(0, 0x20);
	MB60553_get_tilemap(0)->set_transparent_pen(0xf);
}
