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


static TILE_GET_INFO( VS920A_get_tile_info )
{
	gstriker_state *state = machine.driver_data<gstriker_state>();
	int data;
	int tileno, pal;

	data = state->m_VS920A_cur_chip->vram[tile_index];

	tileno = data & 0xFFF;
	pal =   (data >> 12) & 0xF;

	SET_TILE_INFO(state->m_VS920A_cur_chip->gfx_region, tileno, state->m_VS920A_cur_chip->pal_base + pal, 0);
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

static void VS920A_init(running_machine &machine, int numchips)
{
	gstriker_state *state = machine.driver_data<gstriker_state>();
	int i;

	if (numchips > MAX_VS920A)
		numchips = MAX_VS920A;

	for (i=0;i<numchips;i++)
	{
		state->m_VS920A[i].tmap = tilemap_create(machine, VS920A_get_tile_info,tilemap_scan_rows,8,8,64,32);

		state->m_VS920A[i].tmap->set_transparent_pen(0);
	}
}

static tilemap_t* VS920A_get_tilemap(gstriker_state *state, int numchip)
{
	return state->m_VS920A[numchip].tmap;
}

static void VS920A_set_pal_base(gstriker_state *state, int numchip, int pal_base)
{
	state->m_VS920A[numchip].pal_base = pal_base;
}

static void VS920A_set_gfx_region(gstriker_state *state, int numchip, int gfx_region)
{
	state->m_VS920A[numchip].gfx_region = gfx_region;
}

static void VS920A_draw(gstriker_state *state, int numchip, bitmap_ind16& screen, const rectangle &cliprect, int priority)
{
	state->m_VS920A_cur_chip = &state->m_VS920A[numchip];

	state->m_VS920A_cur_chip->tmap->draw(screen, cliprect, 0, priority);
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



static TILE_GET_INFO( MB60553_get_tile_info )
{
	gstriker_state *state = machine.driver_data<gstriker_state>();
	int data, bankno;
	int tileno, pal;

	data = state->m_MB60553_cur_chip->vram[tile_index];

	tileno = data & 0x1FF;
	pal = (data >> 12) & 0xF;
	bankno = (data >> 9) & 0x7;

	SET_TILE_INFO(state->m_MB60553->gfx_region, tileno + state->m_MB60553_cur_chip->bank[bankno] * 0x200, pal + state->m_MB60553->pal_base, 0);
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
static TILEMAP_MAPPER( twc94_scan )
{
	/* logical (col,row) -> memory offset */
	return (row*64) + (col&63) + ((col&64)<<6);
}

static void MB60553_init(running_machine &machine, int numchips)
{
	gstriker_state *state = machine.driver_data<gstriker_state>();
	int i;

	if (numchips > MAX_MB60553)
		numchips = MAX_MB60553;

	for (i=0;i<numchips;i++)
	{
		state->m_MB60553[i].tmap = tilemap_create(machine, MB60553_get_tile_info,twc94_scan, 16,16,128,64);

		state->m_MB60553[i].tmap->set_transparent_pen(0);
	}
}

static void MB60553_set_pal_base(gstriker_state *state, int numchip, int pal_base)
{
	state->m_MB60553[numchip].pal_base = pal_base;
}

static void MB60553_set_gfx_region(gstriker_state *state, int numchip, int gfx_region)
{
	state->m_MB60553[numchip].gfx_region = gfx_region;
}

/* THIS IS STILL WRONG! */
static void MB60553_draw(running_machine &machine, int numchip, bitmap_ind16& screen, const rectangle &cliprect, int priority)
{
	gstriker_state *state = machine.driver_data<gstriker_state>();
	int line;
	rectangle clip;
	state->m_MB60553_cur_chip = &state->m_MB60553[numchip];

	clip.min_x = machine.primary_screen->visible_area().min_x;
	clip.max_x = machine.primary_screen->visible_area().max_x;

	for (line = 0; line < 224;line++)
	{
//      int scrollx;
//      int scrolly;

		UINT32 startx,starty;

		UINT32 incxx,incyy;

		startx = state->m_MB60553_cur_chip->regs[0];
		starty = state->m_MB60553_cur_chip->regs[1];

		startx += (24<<4); // maybe not..

		startx -=  state->m_lineram[(line)*8+7]/2;

		incxx = state->m_lineram[(line)*8+0]<<4;
		incyy = state->m_lineram[(line)*8+3]<<4;

		clip.min_y = clip.max_y = line;

		state->m_MB60553_cur_chip->tmap->draw_roz(screen, clip, startx<<12,starty<<12,
				incxx,0,0,incyy,
				1,
				0,priority);

	}



}

static tilemap_t* MB60553_get_tilemap(gstriker_state *state, int numchip)
{
	return state->m_MB60553[numchip].tmap;
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



/*** Fujitsu CG10103 **********************************************/

/*
    Fujitsu CG10103 sprite generator
    --------------------------------

- Tile based
- 16x16 4bpp tiles
- Up to 7x7 in each block
- 5 bit of palette selection for the mixer
- Scaling (x/y)
- Flipping
- Indipendent sorting list
- 1 bit of priority for the mixer

Note that this chip can be connected to a VS9210 which adds a level of indirection for
tile numbers. Basically, the VS9210 indirects the tile number through a table in its attached
memory, before accessing the ROMs.


    Sorting list format (VideoRAM offset 0)
    ---------------------------------------

?e-- ---f ssss ssss

e=end of list
f=sprite present in this position
s=sprite index
?=used together with 'e' almost always


    Sprite format (VideoRAM offset 0x400)
    -------------------------------------

0: nnnn jjjy yyyy yyyy
1: mmmm iiix xxxx xxxx
2: fFpc cccc ---- ---t
3: tttt tttt tttt tttt

t=tile, x=posx, y=posy, i=blockx, j=blocky
c=color, m=zoomx, n=zoomy, p=priority

The zoom (scaling) is probably non-linear, it would require a hand-made table unless we find the correct
formula. I'd probably try 1/x. I'm almost sure that it scales between full size (value=0) and half size
(value=0xF) but I couldn't get much more than that from a soccer game.


TODO:
Priorities should be right, but they probably need to be orthogonal with the mixer priorities.
Zoom factor is not correct, the scale is probably non-linear
Horizontal wrapping is just a hack. The chip probably calculates if it needs to draw the sprite at the
  normal position, or wrapped along X/Y.
Abstracts the VS9210

*/


static void CG10103_draw_sprite(running_machine &machine, bitmap_ind16& screen, const rectangle &cliprect, UINT16* spr, int drawpri)
{
	gstriker_state *state = machine.driver_data<gstriker_state>();
	int ypos = spr[0] & 0x1FF;
	int xpos = (spr[1] & 0x1FF);
	UINT32 tile = (spr[3] & 0xFFFF) | ((spr[2] & 1) << 16);
	int ynum = (spr[0] >> 9) & 0x7;
	int xnum = (spr[1] >> 9) & 0x7;
	int color = (spr[2] >> 8) & 0x1F;
	int flipx = (spr[2] >> 14) & 1;
	int flipy = (spr[2] >> 15) & 1;
	int yzoom = (spr[0] >> 12) & 0xF;
	int xzoom = (spr[1] >> 12) & 0xF;
	int pri = (spr[2] >> 13) & 1;
	int x, y;
	int xstep, ystep;
	int xfact, yfact;

	// Check if we want to draw this sprite now
	if (pri != drawpri)
		return;

	// Convert in fixed point to handle the scaling
	xpos <<= 16;
	ypos <<= 16;

	xnum++;
	ynum++;
	xstep = ystep = 16;

	// Linear scale, surely wrong
	xfact = 0x10000 - ((0x8000 * xzoom) / 15);
	yfact = 0x10000 - ((0x8000 * yzoom) / 15);

	xstep *= xfact;
	ystep *= yfact;

	// Handle flipping
	if (flipy)
	{
		ypos += (ynum-1) * ystep;
		ystep = -ystep;
	}

	if (flipx)
	{
		xpos += (xnum-1) * xstep;
		xstep = -xstep;
	}

	// @@@ Add here optional connection to the VS9210 for extra level of tile number indirection
#if 0
	if (state->m_CG10103_cur_chip->connected_vs9210)
	{
		// ...
	}
#endif

	// Draw the block
	for (y=0;y<ynum;y++)
	{
		int xp = xpos;

		for (x=0;x<xnum;x++)
		{
			// Hack to handle horizontal wrapping
			drawgfxzoom_transpen(screen, cliprect, machine.gfx[state->m_CG10103_cur_chip->gfx_region], tile, color+state->m_CG10103_cur_chip->pal_base, flipx, flipy, xp>>16, ypos>>16, xfact, yfact, state->m_CG10103_cur_chip->transpen);
			drawgfxzoom_transpen(screen, cliprect, machine.gfx[state->m_CG10103_cur_chip->gfx_region], tile, color+state->m_CG10103_cur_chip->pal_base, flipx, flipy, (xp>>16) - 0x200, ypos>>16, xfact, yfact, state->m_CG10103_cur_chip->transpen);
			xp += xstep;
			tile++;
		}

		ypos += ystep;
	}
}


static void CG10103_draw(running_machine &machine, int numchip, bitmap_ind16& screen, const rectangle &cliprect, int priority)
{
	gstriker_state *state = machine.driver_data<gstriker_state>();
	UINT16* splist;
	int i;

	state->m_CG10103_cur_chip = &state->m_CG10103[numchip];

	splist = state->m_CG10103_cur_chip->vram;

	// Parse the sorting list
	for (i=0;i<0x400;i++)
	{
		UINT16 cmd = *splist++;

		// End of list
		if (cmd & 0x4000)
			break;

		// Normal sprite here
		if (cmd & 0x100)
		{
			// Extract sprite index
			int num = cmd & 0xFF;

			// Draw the sprite
			CG10103_draw_sprite(machine, screen, cliprect, state->m_CG10103_cur_chip->vram + 0x400 + num*4, priority);
		}
	}
}

static void CG10103_init(int numchips)
{
	int i;

	if (numchips > MAX_CG10103)
		numchips = MAX_CG10103;

	for (i=0;i<numchips;i++)
	{
		// No initalization required, as for now. I'll keep the init function in case we later
		//  need something
	}
}

static void CG10103_set_pal_base(gstriker_state *state, int numchip, int pal_base)
{
	state->m_CG10103[numchip].pal_base = pal_base;
}

static void CG10103_set_gfx_region(gstriker_state *state, int numchip, int gfx_region)
{
	state->m_CG10103[numchip].gfx_region = gfx_region;
}

static void CG10103_set_transpen(gstriker_state *state, int numchip, int transpen)
{
	state->m_CG10103[numchip].transpen = transpen;
}


/*** VIDEO UPDATE/START **********************************************/


#ifdef UNUSED_FUNCTION
WRITE16_MEMBER(gstriker_state::gsx_videoram3_w)
{
	// This memory appears to be empty in gstriker
	gs_videoram3[offset] = data;
}
#endif


SCREEN_UPDATE_IND16(gstriker)
{
	gstriker_state *state = screen.machine().driver_data<gstriker_state>();
	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	// Sandwitched screen/sprite0/score/sprite1. Surely wrong, probably
	//  needs sprite orthogonality
	MB60553_draw(screen.machine(), 0, bitmap,cliprect, 0);

	CG10103_draw(screen.machine(), 0, bitmap, cliprect, 0);

	VS920A_draw(state, 0, bitmap, cliprect, 0);

	CG10103_draw(screen.machine(), 0, bitmap, cliprect, 1);

#if 0
	popmessage("%04x %04x %04x %04x %04x %04x %04x %04x",
		(UINT16)state->m_MB60553[0].regs[0], (UINT16)state->m_MB60553[0].regs[1], (UINT16)state->m_MB60553[0].regs[2], (UINT16)state->m_MB60553[0].regs[3],
		(UINT16)state->m_MB60553[0].regs[4], (UINT16)state->m_MB60553[0].regs[5], (UINT16)state->m_MB60553[0].regs[6], (UINT16)state->m_MB60553[0].regs[7]
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

VIDEO_START(gstriker)
{
	gstriker_state *state = machine.driver_data<gstriker_state>();

	// Palette bases are hardcoded, but should be probably extracted from the mixer registers

	// Initalize the chip for the score plane
	VS920A_init(machine, 1);
	VS920A_set_gfx_region(state, 0, 0);
	VS920A_set_pal_base(state, 0, 0x30);
	VS920A_get_tilemap(state, 0)->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	MB60553_init(machine, 1);
	MB60553_set_gfx_region(state, 0, 1);
	MB60553_set_pal_base(state, 0, 0);
	MB60553_get_tilemap(state, 0)->set_transparent_pen(0xf);

	// Initialize the sprite generator
	CG10103_init(1);
	CG10103_set_gfx_region(state, 0, 2);
	CG10103_set_pal_base(state, 0, 0x10);
	CG10103_set_transpen(state, 0, 0x0);
}

VIDEO_START(twrldc94)
{
	gstriker_state *state = machine.driver_data<gstriker_state>();

	// Palette bases are hardcoded, but should be probably extracted from the mixer registers

	// Initalize the chip for the score plane
	VS920A_init(machine, 1);
	VS920A_set_gfx_region(state, 0, 0);
	VS920A_set_pal_base(state, 0, 0x40);
	VS920A_get_tilemap(state, 0)->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	MB60553_init(machine, 1);
	MB60553_set_gfx_region(state, 0, 1);
	MB60553_set_pal_base(state, 0, 0x50);
	MB60553_get_tilemap(state, 0)->set_transparent_pen(0xf);

	// Initialize the sprite generator
	CG10103_init(1);
	CG10103_set_gfx_region(state, 0, 2);
	CG10103_set_pal_base(state, 0, 0x60);
	CG10103_set_transpen(state, 0, 0x0);
}

VIDEO_START(vgoalsoc)
{
	gstriker_state *state = machine.driver_data<gstriker_state>();

	// Palette bases are hardcoded, but should be probably extracted from the mixer registers

	// Initalize the chip for the score plane
	VS920A_init(machine, 1);
	VS920A_set_gfx_region(state, 0, 0);
	VS920A_set_pal_base(state, 0, 0x30);
	VS920A_get_tilemap(state, 0)->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	MB60553_init(machine, 1);
	MB60553_set_gfx_region(state, 0, 1);
	MB60553_set_pal_base(state, 0, 0x20);
	MB60553_get_tilemap(state, 0)->set_transparent_pen(0xf);

	// Initialize the sprite generator
	CG10103_init(1);
	CG10103_set_gfx_region(state, 0, 2);
	CG10103_set_pal_base(state, 0, 0x00);
	CG10103_set_transpen(state, 0, 0xf);
}
