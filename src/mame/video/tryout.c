/***************************************************************

 Pro Yakyuu Nyuudan Test Tryout (JPN Ver.)
 video hardware emulation

****************************************************************/

#include "emu.h"
#include "includes/tryout.h"


PALETTE_INIT( tryout )
{
	int i;

	for (i = 0;i < machine.total_colors();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

static TILE_GET_INFO( get_fg_tile_info )
{
	tryout_state *state = machine.driver_data<tryout_state>();
	UINT8 *videoram = state->m_videoram;
	int code, attr, color;

	code = videoram[tile_index];
	attr = videoram[tile_index + 0x400];
	code |= ((attr & 0x03) << 8);
	color = ((attr & 0x4)>>2)+6;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	tryout_state *state = machine.driver_data<tryout_state>();
	SET_TILE_INFO(2, state->m_vram[tile_index] & 0x7f, 2, 0);
}

READ8_MEMBER(tryout_state::tryout_vram_r)
{
	return m_vram[offset]; // debug only
}

WRITE8_MEMBER(tryout_state::tryout_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(tryout_state::tryout_vram_w)
{
	/*  There are eight banks of vram - in bank 0 the first 0x400 bytes
    is reserved for the tilemap.  In banks 2, 4 and 6 the game never
    writes to the first 0x400 bytes - I suspect it's either
    unused, or it actually mirrors the tilemap ram from the first bank.

    The rest of the vram is tile data which has the bitplanes arranged
    in a very strange format.  For Mame's sake we reformat this on
    the fly for easier gfx decode.

    Bit 0 of the bank register seems special - it's kept low when uploading
    gfx data and then set high from that point onwards.

    */
	const UINT8 bank=(m_vram_bank>>1)&0x7;


	if ((bank==0 || bank==2 || bank==4 || bank==6) && (offset&0x7ff)<0x400) {
		int newoff=offset&0x3ff;

		m_vram[newoff]=data;
		m_bg_tilemap->mark_tile_dirty(newoff);
		return;
	}

	/*
        Bit planes for tiles are arranged as follows within vram (split into high/low nibbles):
            0x0400 (0) + 0x0400 (4) + 0x0800(0) - tiles 0x00 to 0x0f
            0x0800 (4) + 0x0c00 (0) + 0x0c00(4) - tiles 0x10 to 0x1f
            0x1400 (0) + 0x1400 (4) + 0x1800(0) - tiles 0x20 to 0x2f
            0x1800 (4) + 0x1c00 (0) + 0x1c00(4) - tiles 0x30 to 0x3f
            etc.
    */

	offset=(offset&0x7ff) | (bank<<11);
	m_vram[offset]=data;

	switch (offset&0x1c00) {
	case 0x0400:
		m_vram_gfx[(offset&0x3ff) + 0x0000 + ((offset&0x2000)>>1)]=(~data&0xf);
		m_vram_gfx[(offset&0x3ff) + 0x2000 + ((offset&0x2000)>>1)]=(~data&0xf0)>>4;
		break;
	case 0x0800:
		m_vram_gfx[(offset&0x3ff) + 0x4000 + ((offset&0x2000)>>1)]=(~data&0xf);
		m_vram_gfx[(offset&0x3ff) + 0x4400 + ((offset&0x2000)>>1)]=(~data&0xf0)>>4;
		break;
	case 0x0c00:
		m_vram_gfx[(offset&0x3ff) + 0x0400 + ((offset&0x2000)>>1)]=(~data&0xf);
		m_vram_gfx[(offset&0x3ff) + 0x2400 + ((offset&0x2000)>>1)]=(~data&0xf0)>>4;
		break;
	case 0x1400:
		m_vram_gfx[(offset&0x3ff) + 0x0800 + ((offset&0x2000)>>1)]=(~data&0xf);
		m_vram_gfx[(offset&0x3ff) + 0x2800 + ((offset&0x2000)>>1)]=(~data&0xf0)>>4;
		break;
	case 0x1800:
		m_vram_gfx[(offset&0x3ff) + 0x4800 + ((offset&0x2000)>>1)]=(~data&0xf);
		m_vram_gfx[(offset&0x3ff) + 0x4c00 + ((offset&0x2000)>>1)]=(~data&0xf0)>>4;
		break;
	case 0x1c00:
		m_vram_gfx[(offset&0x3ff) + 0x0c00 + ((offset&0x2000)>>1)]=(~data&0xf);
		m_vram_gfx[(offset&0x3ff) + 0x2c00 + ((offset&0x2000)>>1)]=(~data&0xf0)>>4;
		break;
	}

	gfx_element_mark_dirty(machine().gfx[2], (offset-0x400/64)&0x7f);
}

WRITE8_MEMBER(tryout_state::tryout_vram_bankswitch_w)
{
	m_vram_bank = data;
}

WRITE8_MEMBER(tryout_state::tryout_flipscreen_w)
{
	flip_screen_set(machine(), data & 1);
}

static TILEMAP_MAPPER( get_fg_memory_offset )
{
	return (row ^ 0x1f) + (col << 5);
}

static TILEMAP_MAPPER( get_bg_memory_offset )
{
	int a;
//  if (col&0x20)
//      a= (7 - (row & 7)) + ((0x8 - (row & 0x8)) << 4) + ((col & 0xf) << 3) + (( (  0x10 - (col & 0x10) ) ) << 4) + ((( (col & 0x20))) << 4);
//  else
		a= (7 - (row & 7)) + ((0x8 - (row & 0x8)) << 4) + ((col & 0xf) << 3) + (( (  (col & 0x10) ) ) << 4) + ((( (col & 0x20))) << 4);

//  mame_printf_debug("%d %d -> %d\n",col,row, a);
	return a;
}

VIDEO_START( tryout )
{
	tryout_state *state = machine.driver_data<tryout_state>();
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,get_fg_memory_offset,8,8,32,32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,get_bg_memory_offset,16,16,64,16);

	state->m_vram=auto_alloc_array(machine, UINT8, 8 * 0x800);
	state->m_vram_gfx=auto_alloc_array(machine, UINT8, 0x6000);

	gfx_element_set_source(machine.gfx[2], state->m_vram_gfx);

	state->m_fg_tilemap->set_transparent_pen(0);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	tryout_state *state = machine.driver_data<tryout_state>();
	UINT8 *spriteram = state->m_spriteram;
	UINT8 *spriteram_2 = state->m_spriteram2;
	int offs,fx,fy,x,y,color,sprite,inc;

	for (offs = 0;offs < 0x7f;offs += 4)
	{
		if (!(spriteram[offs]&1))
			continue;

		sprite = spriteram[offs+1] + ((spriteram_2[offs]&7)<<8);
		x = spriteram[offs+3]-3;
		y = spriteram[offs+2];
		color = 0;//(spriteram[offs] & 8)>>3;
		fx = (spriteram[offs] & 8)>>3;
		fy = 0;
		inc = 16;

		if (flip_screen_get(machine))
		{
			x = 240 - x;
			fx = !fx;

			y = 240 - y;
			fy = !fy;

			inc = -inc;
		}

		/* Double Height */
		if(spriteram[offs] & 0x10)
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				sprite,
				color,fx,fy,x,y + inc,0);

			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				sprite+1,
				color,fx,fy,x,y,0);
		}
		else
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				sprite,
				color,fx,fy,x,y,0);
		}
	}
}

SCREEN_UPDATE_IND16( tryout )
{
	tryout_state *state = screen.machine().driver_data<tryout_state>();
	int scrollx = 0;

	if (!flip_screen_get(screen.machine()))
		state->m_fg_tilemap->set_scrollx(0, 16); /* Assumed hard-wired */
	else
		state->m_fg_tilemap->set_scrollx(0, -8); /* Assumed hard-wired */

	scrollx = state->m_gfx_control[1] + ((state->m_gfx_control[0]&1)<<8) + ((state->m_gfx_control[0]&4)<<7) - ((state->m_gfx_control[0] & 2) ? 0 : 0x100);

	/* wrap-around */
	if(state->m_gfx_control[1] == 0) { scrollx+=0x100; }

	state->m_bg_tilemap->set_scrollx(0, scrollx+2); /* why +2? hard-wired? */
	state->m_bg_tilemap->set_scrolly(0, -state->m_gfx_control[2]);

	if(!(state->m_gfx_control[0] & 0x8)) // screen disable
	{
		/* TODO: Color might be different, needs a video from an original pcb. */
		bitmap.fill(screen.machine().pens[0x10], cliprect);
	}
	else
	{
		state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
		state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);
		draw_sprites(screen.machine(), bitmap,cliprect);
	}

//  popmessage("%02x %02x %02x %02x",state->m_gfx_control[0],state->m_gfx_control[1],state->m_gfx_control[2],scrollx);
	return 0;
}
