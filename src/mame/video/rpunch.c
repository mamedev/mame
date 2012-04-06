/***************************************************************************

  video/rpunch.c

  Functions to emulate the video hardware of the machine.

****************************************************************************/

#include "emu.h"
#include "includes/rpunch.h"


#define BITMAP_WIDTH	304
#define BITMAP_HEIGHT	224
#define BITMAP_XOFFSET	4


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_bg0_tile_info )
{
	rpunch_state *state = machine.driver_data<rpunch_state>();
	UINT16 *videoram = state->m_videoram;
	int data = videoram[tile_index];
	int code;
	if (state->m_videoflags & 0x0400)	code = (data & 0x0fff) | 0x2000;
	else						code = (data & 0x1fff);

	SET_TILE_INFO(
			0,
			code,
			((state->m_videoflags & 0x0010) >> 1) | ((data >> 13) & 7),
			0);
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	rpunch_state *state = machine.driver_data<rpunch_state>();
	UINT16 *videoram = state->m_videoram;
	int data = videoram[0x2000 / 2 + tile_index];
	int code;
	if (state->m_videoflags & 0x0800)	code = (data & 0x0fff) | 0x2000;
	else						code = (data & 0x1fff);

	SET_TILE_INFO(
			1,
			code,
			((state->m_videoflags & 0x0020) >> 2) | ((data >> 13) & 7),
			0);
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

static TIMER_CALLBACK( crtc_interrupt_gen )
{
	rpunch_state *state = machine.driver_data<rpunch_state>();
	cputag_set_input_line(machine, "maincpu", 1, HOLD_LINE);
	if (param != 0)
		state->m_crtc_timer->adjust(machine.primary_screen->frame_period() / param, 0, machine.primary_screen->frame_period() / param);
}


VIDEO_START( rpunch )
{
	rpunch_state *state = machine.driver_data<rpunch_state>();
	/* allocate tilemaps for the backgrounds */
	state->m_background[0] = tilemap_create(machine, get_bg0_tile_info,tilemap_scan_cols,8,8,64,64);
	state->m_background[1] = tilemap_create(machine, get_bg1_tile_info,tilemap_scan_cols,8,8,64,64);

	/* configure the tilemaps */
	state->m_background[1]->set_transparent_pen(15);

	if (state->m_bitmapram)
		memset(state->m_bitmapram, 0xff, state->m_bitmapram_size);

	/* reset the timer */
	state->m_crtc_timer = machine.scheduler().timer_alloc(FUNC(crtc_interrupt_gen));
}



/*************************************
 *
 *  Write handlers
 *
 *************************************/

WRITE16_MEMBER(rpunch_state::rpunch_videoram_w)
{
	UINT16 *videoram = m_videoram;
	int tmap = offset >> 12;
	int tile_index = offset & 0xfff;
	COMBINE_DATA(&videoram[offset]);
	m_background[tmap]->mark_tile_dirty(tile_index);
}


WRITE16_MEMBER(rpunch_state::rpunch_videoreg_w)
{
	int oldword = m_videoflags;
	COMBINE_DATA(&m_videoflags);

	if (m_videoflags != oldword)
	{
		/* invalidate tilemaps */
		if ((oldword ^ m_videoflags) & 0x0410)
			m_background[0]->mark_all_dirty();
		if ((oldword ^ m_videoflags) & 0x0820)
			m_background[1]->mark_all_dirty();
	}
}


WRITE16_MEMBER(rpunch_state::rpunch_scrollreg_w)
{
	if (ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15)
		switch (offset)
		{
			case 0:
				m_background[0]->set_scrolly(0, data & 0x1ff);
				break;

			case 1:
				m_background[0]->set_scrollx(0, data & 0x1ff);
				break;

			case 2:
				m_background[1]->set_scrolly(0, data & 0x1ff);
				break;

			case 3:
				m_background[1]->set_scrollx(0, data & 0x1ff);
				break;
		}
}


WRITE16_MEMBER(rpunch_state::rpunch_crtc_data_w)
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		switch (m_crtc_register)
		{
			/* only register we know about.... */
			case 0x0b:
				m_crtc_timer->adjust(machine().primary_screen->time_until_vblank_start(), (data == 0xc0) ? 2 : 1);
				break;

			default:
				logerror("CRTC register %02X = %02X\n", m_crtc_register, data & 0xff);
				break;
		}
	}
}


WRITE16_MEMBER(rpunch_state::rpunch_crtc_register_w)
{
	if (ACCESSING_BITS_0_7)
		m_crtc_register = data & 0xff;
}


WRITE16_MEMBER(rpunch_state::rpunch_ins_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (offset == 0)
		{
			m_gins = data & 0x3f;
			logerror("GINS = %02X\n", data & 0x3f);
		}
		else
		{
			m_bins = data & 0x3f;
			logerror("BINS = %02X\n", data & 0x3f);
		}
	}
}


/*************************************
 *
 *  Sprite routines
 *
 *************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int stop)
{
	rpunch_state *state = machine.driver_data<rpunch_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int offs;

	start *= 4;
	stop *= 4;

	/* draw the sprites */
	for (offs = start; offs < stop; offs += 4)
	{
		int data1 = spriteram16[offs + 1];
		int code = data1 & 0x7ff;

		int data0 = spriteram16[offs + 0];
		int data2 = spriteram16[offs + 2];
		int x = (data2 & 0x1ff) + 8;
		int y = 513 - (data0 & 0x1ff);
		int xflip = data1 & 0x1000;
		int yflip = data1 & 0x0800;
		int color = ((data1 >> 13) & 7) | ((state->m_videoflags & 0x0040) >> 3);

		if (x >= BITMAP_WIDTH) x -= 512;
		if (y >= BITMAP_HEIGHT) y -= 512;

		drawgfx_transpen(bitmap, cliprect, machine.gfx[2],
				code, color + (state->m_sprite_palette / 16), xflip, yflip, x, y, 15);
	}
}


/*************************************
 *
 *  Bitmap routines
 *
 *************************************/

static void draw_bitmap(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rpunch_state *state = machine.driver_data<rpunch_state>();
	int colourbase;
	int xxx=512/4;
	int yyy=256;
	int x,y,count;

	colourbase = 512 + ((state->m_videoflags & 15) * 16);

	count = 0;

	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			int coldat;
			coldat = (state->m_bitmapram[count]>>12)&0xf; if (coldat!=15) bitmap.pix16(y, ((x*4+0)-4)&0x1ff) = coldat+colourbase;
			coldat = (state->m_bitmapram[count]>>8 )&0xf; if (coldat!=15) bitmap.pix16(y, ((x*4+1)-4)&0x1ff) = coldat+colourbase;
			coldat = (state->m_bitmapram[count]>>4 )&0xf; if (coldat!=15) bitmap.pix16(y, ((x*4+2)-4)&0x1ff) = coldat+colourbase;
			coldat = (state->m_bitmapram[count]>>0 )&0xf; if (coldat!=15) bitmap.pix16(y, ((x*4+3)-4)&0x1ff) = coldat+colourbase;
			count++;
		}
	}
}


/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( rpunch )
{
	rpunch_state *state = screen.machine().driver_data<rpunch_state>();
	int effbins;

	/* this seems like the most plausible explanation */
	effbins = (state->m_bins > state->m_gins) ? state->m_gins : state->m_bins;

	state->m_background[0]->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap, cliprect, 0, effbins);
	state->m_background[1]->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap, cliprect, effbins, state->m_gins);
	if (state->m_bitmapram)
		draw_bitmap(screen.machine(), bitmap, cliprect);
	return 0;
}
