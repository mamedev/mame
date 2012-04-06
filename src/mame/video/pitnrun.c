/***************************************************************************

  - BG layer 32x128 , 8x8 tiles 4bpp , 2 palettes  (2nd is black )
  - TXT layer 32x32 , 8x8 tiles 4bpp , 2 palettes (2nd is black)
  - Sprites 16x16 3bpp, 8 palettes (0-3 are black)

  'Special' effects :

  - spotlight - gfx(BG+Sprites) outside spotlight is using black pals
                spotlight masks are taken from ROM pr8
                simulated using bitmaps and custom clipping rect

  - lightning - BG color change (darkening ?) - simple analog circ.
                            simulated by additional palette

In debug build press 'w' for spotlight and 'e' for lightning

***************************************************************************/
#include "emu.h"
#include "includes/pitnrun.h"



static TILE_GET_INFO( get_tile_info1 )
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	UINT8 *videoram = state->m_videoram;
	int code;
	code = videoram[tile_index];
	SET_TILE_INFO(
		0,
		code,
		0,
		0);
}

static TILE_GET_INFO( get_tile_info2 )
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	int code;
	code = state->m_videoram2[tile_index];
	SET_TILE_INFO(
		1,
		code + (state->m_char_bank<<8),
		state->m_color_select&1,
		0);
}

WRITE8_MEMBER(pitnrun_state::pitnrun_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_fg ->mark_all_dirty();
}

WRITE8_MEMBER(pitnrun_state::pitnrun_videoram2_w)
{
	m_videoram2[offset] = data;
	m_bg ->mark_all_dirty();
}

WRITE8_MEMBER(pitnrun_state::pitnrun_char_bank_select)
{
	if(m_char_bank!=data)
	{
		m_bg ->mark_all_dirty();
		m_char_bank=data;
	}
}


WRITE8_MEMBER(pitnrun_state::pitnrun_scroll_w)
{
	m_scroll = (m_scroll & (0xff<<((offset)?0:8))) |( data<<((offset)?8:0));
	m_bg->set_scrollx(0, m_scroll);
}

WRITE8_MEMBER(pitnrun_state::pitnrun_ha_w)
{
	m_ha=data;
}

WRITE8_MEMBER(pitnrun_state::pitnrun_h_heed_w)
{
	m_h_heed=data;
}

WRITE8_MEMBER(pitnrun_state::pitnrun_v_heed_w)
{
	m_v_heed=data;
}

WRITE8_MEMBER(pitnrun_state::pitnrun_color_select_w)
{
	m_color_select=data;
	machine().tilemap().mark_all_dirty();
}

static void pitnrun_spotlights(running_machine &machine)
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	int x,y,i,b,datapix;
	UINT8 *ROM = machine.region("user1")->base();
	for(i=0;i<4;i++)
	 for(y=0;y<128;y++)
	  for(x=0;x<16;x++)
	  {
		datapix=ROM[128*16*i+x+y*16];
		for(b=0;b<8;b++)
		{
			state->m_tmp_bitmap[i]->pix16(y, x*8+(7-b)) = (datapix&1);
			datapix>>=1;
		}
	  }
}


PALETTE_INIT (pitnrun)
{
	int i;
	int bit0,bit1,bit2,r,g,b;
	for (i = 0;i < 32*3; i++)
	{
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	/* fake bg palette for lightning effect*/
	for(i=2*16;i<2*16+16;i++)
	{
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		r/=3;
		g/=3;
		b/=3;

		palette_set_color_rgb(machine,i+16,(r>0xff)?0xff:r,(g>0xff)?0xff:g,(b>0xff)?0xff:b);

	}
}

VIDEO_START(pitnrun)
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	state->m_fg = tilemap_create( machine, get_tile_info1,tilemap_scan_rows,8,8,32,32 );
	state->m_bg = tilemap_create( machine, get_tile_info2,tilemap_scan_rows,8,8,32*4,32 );
	state->m_fg->set_transparent_pen(0 );
	state->m_tmp_bitmap[0] = auto_bitmap_ind16_alloc(machine,128,128);
	state->m_tmp_bitmap[1] = auto_bitmap_ind16_alloc(machine,128,128);
	state->m_tmp_bitmap[2] = auto_bitmap_ind16_alloc(machine,128,128);
	state->m_tmp_bitmap[3] = auto_bitmap_ind16_alloc(machine,128,128);
	pitnrun_spotlights(machine);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	UINT8 *spriteram = state->m_spriteram;
	int sx, sy, flipx, flipy, offs,pal;

	for (offs = 0 ; offs < 0x100; offs+=4)
	{

		pal=spriteram[offs+2]&0x3;

		sy = 256-spriteram[offs+0]-16;
		sx = spriteram[offs+3];
		flipy = (spriteram[offs+1]&0x80)>>7;
		flipx = (spriteram[offs+1]&0x40)>>6;

		if (flip_screen_x_get(machine))
		{
			sx = 256 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y_get(machine))
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
			(spriteram[offs+1]&0x3f)+((spriteram[offs+2]&0x80)>>1)+((spriteram[offs+2]&0x40)<<1),
			pal,
			flipx,flipy,
			sx,sy,0);
	}
}

SCREEN_UPDATE_IND16( pitnrun )
{
	pitnrun_state *state = screen.machine().driver_data<pitnrun_state>();
	int dx=0,dy=0;
	rectangle myclip=cliprect;

#ifdef MAME_DEBUG
	if (screen.machine().input().code_pressed_once(KEYCODE_Q))
	{
		UINT8 *ROM = screen.machine().region("maincpu")->base();
		ROM[0x84f6]=0; /* lap 0 - normal */
	}

	if (screen.machine().input().code_pressed_once(KEYCODE_W))
	{
		UINT8 *ROM = screen.machine().region("maincpu")->base();
		ROM[0x84f6]=6; /* lap 6 = spotlight */
	}

	if (screen.machine().input().code_pressed_once(KEYCODE_E))
	{
		UINT8 *ROM = screen.machine().region("maincpu")->base();
		ROM[0x84f6]=2; /* lap 3 (trial 2)= lightnings */
		ROM[0x8102]=1;
	}
#endif

	bitmap.fill(0, cliprect);

	if(!(state->m_ha&4))
		state->m_bg->draw(bitmap, cliprect, 0,0);
	else
	{
		dx=128-state->m_h_heed+((state->m_ha&8)<<5)+3;
		dy=128-state->m_v_heed+((state->m_ha&0x10)<<4);

		if (flip_screen_x_get(screen.machine()))
			dx=128-dx+16;

		if (flip_screen_y_get(screen.machine()))
			dy=128-dy;

		myclip.set(dx, dx+127, dy, dy+127);
		myclip &= cliprect;

		state->m_bg->draw(bitmap, myclip, 0,0);
	}

	draw_sprites(screen.machine(),bitmap,myclip);

	if(state->m_ha&4)
		copybitmap_trans(bitmap,*state->m_tmp_bitmap[state->m_ha&3],flip_screen_x_get(screen.machine()),flip_screen_y_get(screen.machine()),dx,dy,myclip, 1);
	state->m_fg->draw(bitmap, cliprect, 0,0);
	return 0;
}




