/*******************************************************************

Namco System 1 Video Hardware

*******************************************************************/

#include "emu.h"
#include "includes/namcos1.h"


/*
  video ram map
  0000-1fff : scroll playfield (0) : 64*64*2
  2000-3fff : scroll playfield (1) : 64*64*2
  4000-5fff : scroll playfield (2) : 64*64*2
  6000-6fff : scroll playfield (3) : 64*32*2
  7000-700f : ?
  7010-77ef : fixed playfield (4)  : 36*28*2
  77f0-77ff : ?
  7800-780f : ?
  7810-7fef : fixed playfield (5)  : 36*28*2
  7ff0-7fff : ?
*/

/*
  paletteram map (s1ram  0x0000-0x7fff)
  0000-17ff : palette page0 : sprite
  2000-37ff : palette page1 : playfield
  4000-57ff : palette page2 : playfield (shadow)
  6000-77ff : palette page3 : work RAM

  x800-x807 : CUS116 registers (visibility window)

  so there is just 3x0x2000 RAM, plus the CUS116 internal registers.
*/

/*
  spriteram map (s1ram 0x10000-0x10fff)
  0000-07ff : work ram
  0800-0fef : sprite ram    : 0x10 * 127
  0ff0-0fff : display control registers

  1000-1fff : playfield control registers
*/


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void bg_get_info(running_machine &machine,tile_data &tileinfo,int tile_index,UINT8 *info_vram)
{
	namcos1_state *state = machine.driver_data<namcos1_state>();
	int code;

	tile_index <<= 1;
	code = info_vram[tile_index + 1] + ((info_vram[tile_index] & 0x3f) << 8);
	SET_TILE_INFO(0,code,0,0);
	tileinfo.mask_data = &state->m_tilemap_maskdata[code << 3];
}

INLINE void fg_get_info(running_machine &machine,tile_data &tileinfo,int tile_index,UINT8 *info_vram)
{
	namcos1_state *state = machine.driver_data<namcos1_state>();
	int code;

	tile_index <<= 1;
	code = info_vram[tile_index + 1] + ((info_vram[tile_index] & 0x3f) << 8);
	SET_TILE_INFO(0,code,0,0);
	tileinfo.mask_data = &state->m_tilemap_maskdata[code << 3];
}

TILE_GET_INFO_MEMBER(namcos1_state::bg_get_info0)
{
	bg_get_info(machine(),tileinfo,tile_index,&m_videoram[0x0000]);
}

TILE_GET_INFO_MEMBER(namcos1_state::bg_get_info1)
{
	bg_get_info(machine(),tileinfo,tile_index,&m_videoram[0x2000]);
}

TILE_GET_INFO_MEMBER(namcos1_state::bg_get_info2)
{
	bg_get_info(machine(),tileinfo,tile_index,&m_videoram[0x4000]);
}

TILE_GET_INFO_MEMBER(namcos1_state::bg_get_info3)
{
	bg_get_info(machine(),tileinfo,tile_index,&m_videoram[0x6000]);
}

TILE_GET_INFO_MEMBER(namcos1_state::fg_get_info4)
{
	fg_get_info(machine(),tileinfo,tile_index,&m_videoram[0x7010]);
}

TILE_GET_INFO_MEMBER(namcos1_state::fg_get_info5)
{
	fg_get_info(machine(),tileinfo,tile_index,&m_videoram[0x7810]);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void namcos1_state::video_start()
{
	int i;

	m_tilemap_maskdata = (UINT8 *)memregion("gfx1")->base();

	/* allocate videoram */
	m_videoram = auto_alloc_array(machine(), UINT8, 0x8000);
	m_spriteram = auto_alloc_array(machine(), UINT8, 0x1000);

	/* initialize playfields */
	m_bg_tilemap[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(namcos1_state::bg_get_info0),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_bg_tilemap[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(namcos1_state::bg_get_info1),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_bg_tilemap[2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(namcos1_state::bg_get_info2),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_bg_tilemap[3] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(namcos1_state::bg_get_info3),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_bg_tilemap[4] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(namcos1_state::fg_get_info4),this),TILEMAP_SCAN_ROWS,8,8,36,28);
	m_bg_tilemap[5] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(namcos1_state::fg_get_info5),this),TILEMAP_SCAN_ROWS,8,8,36,28);

	m_bg_tilemap[4]->set_scrolldx(73,512-73);
	m_bg_tilemap[5]->set_scrolldx(73,512-73);
	m_bg_tilemap[4]->set_scrolldy(0x10,0x110);
	m_bg_tilemap[5]->set_scrolldy(0x10,0x110);

	/* register videoram to the save state system (post-allocation) */
	state_save_register_global_pointer(machine(), m_videoram, 0x8000);
	state_save_register_global_array(machine(), m_cus116);
	state_save_register_global_pointer(machine(), m_spriteram, 0x1000);
	state_save_register_global_array(machine(), m_playfield_control);

	/* set table for sprite color == 0x7f */
	for (i = 0;i < 15;i++)
		m_drawmode_table[i] = DRAWMODE_SHADOW;
	m_drawmode_table[15] = DRAWMODE_NONE;

	/* clear paletteram */
	memset(m_paletteram, 0, 0x8000);
	memset(m_cus116, 0, 0x10);
	for (i = 0; i < 0x2000; i++)
		palette_set_color(machine(), i, MAKE_RGB(0, 0, 0));

	/* all palette entries are not affected by shadow sprites... */
	for (i = 0;i < 0x2000;i++)
		machine().shadow_table[i] = i;
	/* ... except for tilemap colors */
	for (i = 0x0800;i < 0x1000;i++)
		machine().shadow_table[i] = i + 0x0800;

	memset(m_playfield_control, 0, sizeof(m_playfield_control));
	m_copy_sprites = 0;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_HANDLER( namcos1_videoram_r )
{
	namcos1_state *state = space->machine().driver_data<namcos1_state>();
	return state->m_videoram[offset];
}

WRITE8_HANDLER( namcos1_videoram_w )
{
	namcos1_state *state = space->machine().driver_data<namcos1_state>();
	state->m_videoram[offset] = data;
	if (offset < 0x7000)
	{   /* background 0-3 */
		int layer = offset >> 13;
		int num = (offset & 0x1fff) >> 1;
		state->m_bg_tilemap[layer]->mark_tile_dirty(num);
	}
	else
	{   /* foreground 4-5 */
		int layer = (offset >> 11 & 1) + 4;
		int num = ((offset & 0x7ff) - 0x10) >> 1;
		if (num >= 0 && num < 0x3f0)
			state->m_bg_tilemap[layer]->mark_tile_dirty(num);
	}
}


WRITE8_HANDLER( namcos1_paletteram_w )
{
	namcos1_state *state = space->machine().driver_data<namcos1_state>();
	if (state->m_paletteram[offset] == data)
		return;

	if ((offset & 0x1800) != 0x1800)
	{
		int r,g,b;
		int color = ((offset & 0x6000) >> 2) | (offset & 0x7ff);

		state->m_paletteram[offset] = data;

		offset &= ~0x1800;
		r = state->m_paletteram[offset];
		g = state->m_paletteram[offset + 0x0800];
		b = state->m_paletteram[offset + 0x1000];
		palette_set_color(space->machine(),color,MAKE_RGB(r,g,b));
	}
	else
	{
		int i, j;

		state->m_cus116[offset & 0x0f] = data;

		for (i = 0x1800; i < 0x8000; i += 0x2000)
		{
			offset = (offset & 0x0f) | i;

			for (j = 0; j < 0x80; j++, offset += 0x10)
				state->m_paletteram[offset] = data;
		}
	}
}




READ8_HANDLER( namcos1_spriteram_r )
{
	namcos1_state *state = space->machine().driver_data<namcos1_state>();
	/* 0000-07ff work ram */
	/* 0800-0fff sprite ram */
	if (offset < 0x1000)
		return state->m_spriteram[offset];
	/* 1xxx playfield control ram */
	else
		return state->m_playfield_control[offset & 0x1f];
}

WRITE8_HANDLER( namcos1_spriteram_w )
{
	namcos1_state *state = space->machine().driver_data<namcos1_state>();
	/* 0000-07ff work ram */
	/* 0800-0fff sprite ram */
	if (offset < 0x1000)
	{
		state->m_spriteram[offset] = data;

		/* a write to this offset tells the sprite chip to buffer the sprite list */
		if (offset == 0x0ff2)
			state->m_copy_sprites = 1;
	}
	/* 1xxx playfield control ram */
	else
		state->m_playfield_control[offset & 0x1f] = data;
}



/***************************************************************************

  Display refresh

***************************************************************************/

/*
sprite format:

0-3  scratchpad RAM
4-9  CPU writes here, hardware copies from here to 10-15
10   xx------  X size (16, 8, 32, 4)
10   --x-----  X flip
10   ---xx---  X offset inside 32x32 tile
10   -----xxx  tile bank
11   xxxxxxxx  tile number
12   xxxxxxx-  color
12   -------x  X position MSB
13   xxxxxxxx  X position
14   xxx-----  priority
14   ---xx---  Y offset inside 32x32 tile
14   -----xx-  Y size (16, 8, 32, 4)
14   -------x  Y flip
15   xxxxxxxx  Y position
*/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	namcos1_state *state = machine.driver_data<namcos1_state>();
	UINT8 *spriteram = state->m_spriteram + 0x800;
	const UINT8 *source = &spriteram[0x800-0x20];	/* the last is NOT a sprite */
	const UINT8 *finish = &spriteram[0];
	gfx_element *gfx = machine.gfx[1];

	int sprite_xoffs = spriteram[0x07f5] + ((spriteram[0x07f4] & 1) << 8);
	int sprite_yoffs = spriteram[0x07f7];

	while (source >= finish)
	{
		static const int sprite_size[4] = { 16, 8, 32, 4 };
		int attr1 = source[10];
		int attr2 = source[14];
		int color = source[12];
		int flipx = (attr1 & 0x20) >> 5;
		int flipy = (attr2 & 0x01);
		int sizex = sprite_size[(attr1 & 0xc0) >> 6];
		int sizey = sprite_size[(attr2 & 0x06) >> 1];
		int tx = (attr1 & 0x18) & (~(sizex-1));
		int ty = (attr2 & 0x18) & (~(sizey-1));
		int sx = source[13] + ((color & 0x01) << 8);
		int sy = -source[15] - sizey;
		int sprite = source[11];
		int sprite_bank = attr1 & 7;
		int priority = (source[14] & 0xe0) >> 5;
		int pri_mask = (0xff << (priority + 1)) & 0xff;

		sprite += sprite_bank * 256;
		color = color >> 1;

		sx += sprite_xoffs;
		sy -= sprite_yoffs;

		if (state->flip_screen())
		{
			sx = -sx - sizex;
			sy = -sy - sizey;
			flipx ^= 1;
			flipy ^= 1;
		}

		sy++;	/* sprites are buffered and delayed by one scanline */

		gfx->set_source_clip(tx, sizex, ty, sizey);
		if (color != 0x7f)
			pdrawgfx_transpen( bitmap, cliprect, gfx,
					sprite,
					color,
					flipx,flipy,
					sx & 0x1ff,
					((sy + 16) & 0xff) - 16,
					machine.priority_bitmap, pri_mask,
					0xf);
		else
			pdrawgfx_transtable( bitmap, cliprect, gfx,
					sprite,
					color,
					flipx,flipy,
					sx & 0x1ff,
					((sy + 16) & 0xff) - 16,
					machine.priority_bitmap, pri_mask,
					state->m_drawmode_table, machine.shadow_table);

		source -= 0x10;
	}
}



SCREEN_UPDATE_IND16( namcos1 )
{
	namcos1_state *state = screen.machine().driver_data<namcos1_state>();
	int i, j, scrollx, scrolly, priority;
	rectangle new_clip = cliprect;

	/* flip screen is embedded in the sprite control registers */
	/* can't use state->flip_screen_set() because the visible area is asymmetrical */
	state->flip_screen_set_no_update(state->m_spriteram[0x0ff6] & 1);
	screen.machine().tilemap().set_flip_all(state->flip_screen() ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);


	/* background color */
	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	/* berabohm uses asymmetrical visibility windows to iris on the character */
	i = ((state->m_cus116[0] << 8) | state->m_cus116[1]) - 1;			// min x
	if (new_clip.min_x < i) new_clip.min_x = i;
	i = ((state->m_cus116[2] << 8) | state->m_cus116[3]) - 1 - 1;		// max x
	if (new_clip.max_x > i) new_clip.max_x = i;
	i = ((state->m_cus116[4] << 8) | state->m_cus116[5]) - 0x11;		// min y
	if (new_clip.min_y < i) new_clip.min_y = i;
	i = ((state->m_cus116[6] << 8) | state->m_cus116[7]) - 0x11 - 1;	// max y
	if (new_clip.max_y > i) new_clip.max_y = i;

	if (new_clip.empty())
		return 0;


	/* set palette base */
	for (i = 0;i < 6;i++)
		state->m_bg_tilemap[i]->set_palette_offset((state->m_playfield_control[i + 24] & 7) * 256);

	for (i = 0;i < 4;i++)
	{
		static const int disp_x[] = { 25, 27, 28, 29 };

		j = i << 2;
		scrollx = ( state->m_playfield_control[j+1] + (state->m_playfield_control[j+0]<<8) ) - disp_x[i];
		scrolly = ( state->m_playfield_control[j+3] + (state->m_playfield_control[j+2]<<8) ) + 8;

		if (state->flip_screen())
		{
			scrollx = -scrollx;
			scrolly = -scrolly;
		}

		state->m_bg_tilemap[i]->set_scrollx(0,scrollx);
		state->m_bg_tilemap[i]->set_scrolly(0,scrolly);
	}


	screen.machine().priority_bitmap.fill(0, new_clip);

	/* bit 0-2 priority */
	/* bit 3   disable  */
	for (priority = 0; priority < 8;priority++)
	{
		for (i = 0;i < 6;i++)
		{
			if (state->m_playfield_control[16 + i] == priority)
				state->m_bg_tilemap[i]->draw(bitmap, new_clip, 0,priority,0);
		}
	}

	draw_sprites(screen.machine(), bitmap, new_clip);
	return 0;
}


SCREEN_VBLANK( namcos1 )
{
	// rising edge
	if (vblank_on)
	{
		namcos1_state *state = screen.machine().driver_data<namcos1_state>();
		if (state->m_copy_sprites)
		{
			UINT8 *spriteram = state->m_spriteram + 0x800;
			int i,j;

			for (i = 0;i < 0x800;i += 16)
			{
				for (j = 10;j < 16;j++)
					spriteram[i+j] = spriteram[i+j - 6];
			}

			state->m_copy_sprites = 0;
		}
	}
}
