/*
    Video Hardware for Shoot Out
    prom GB09.K6 may be related to background tile-sprite priority
*/

#include "emu.h"
#include "includes/shootout.h"


PALETTE_INIT( shootout )
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



static TILE_GET_INFO( get_bg_tile_info )
{
	shootout_state *state = machine.driver_data<shootout_state>();
	int attributes = state->m_videoram[tile_index+0x400]; /* CCCC -TTT */
	int tile_number = state->m_videoram[tile_index] + 256*(attributes&7);
	int color = attributes>>4;

	SET_TILE_INFO(
			2,
			tile_number,
			color,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	shootout_state *state = machine.driver_data<shootout_state>();
	int attributes = state->m_textram[tile_index+0x400]; /* CCCC --TT */
	int tile_number = state->m_textram[tile_index] + 256*(attributes&0x3);
	int color = attributes>>4;

	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
}

WRITE8_MEMBER(shootout_state::shootout_videoram_w)
{

	m_videoram[offset] = data;
	m_background->mark_tile_dirty(offset&0x3ff );
}

WRITE8_MEMBER(shootout_state::shootout_textram_w)
{

	m_textram[offset] = data;
	m_foreground->mark_tile_dirty(offset&0x3ff );
}

VIDEO_START( shootout )
{
	shootout_state *state = machine.driver_data<shootout_state>();

	state->m_background = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_foreground = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_foreground->set_transparent_pen(0 );
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_bits )
{
	shootout_state *state = machine.driver_data<shootout_state>();
	UINT8 *spriteram = state->m_spriteram;
	const gfx_element *gfx = machine.gfx[1];
	const UINT8 *source = spriteram+127*4;
	int count;

	state->m_bFlicker = !state->m_bFlicker;

	for( count=0; count<128; count++ )
	{
		int attributes = source[1];
		/*
            76543210
            xxx-----    bank
            ---x----    vertical size
            ----x---    priority
            -----x--    horizontal flip
            ------x-    flicker
            -------x    enable
        */
		if ( attributes & 0x01 ){ /* visible */
			if( state->m_bFlicker || (attributes&0x02)==0 ){
				int priority_mask = (attributes&0x08)?0x2:0;
				int sx = (240 - source[2])&0xff;
				int sy = (240 - source[0])&0xff;
				int vx, vy;
				int number = source[3] | ((attributes<<bank_bits)&0x700);
				int flipx = (attributes & 0x04);
				int flipy = 0;

				if (flip_screen_get(machine)) {
					flipx = !flipx;
					flipy = !flipy;
				}

				if( attributes & 0x10 ){ /* double height */
					number = number&(~1);
					sy -= 16;

					vx = sx;
					vy = sy;
					if (flip_screen_get(machine)) {
						vx = 240 - vx;
						vy = 240 - vy;
					}

					pdrawgfx_transpen(bitmap,cliprect,gfx,
						number,
						0 /*color*/,
						flipx,flipy,
						vx,vy,
						machine.priority_bitmap,
						priority_mask,0);

					number++;
					sy += 16;
				}

				vx = sx;
				vy = sy;
				if (flip_screen_get(machine)) {
					vx = 240 - vx;
					vy = 240 - vy;
				}

				pdrawgfx_transpen(bitmap,cliprect,gfx,
						number,
						0 /*color*/,
						flipx,flipy,
						vx,vy,
						machine.priority_bitmap,
						priority_mask,0);
			}
		}
		source -= 4;
	}
}

SCREEN_UPDATE_IND16( shootout )
{
	shootout_state *state = screen.machine().driver_data<shootout_state>();

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_background->draw(bitmap, cliprect, 0,0);
	state->m_foreground->draw(bitmap, cliprect, 0,1);
	draw_sprites(screen.machine(), bitmap,cliprect,3/*bank bits */);
	return 0;
}

SCREEN_UPDATE_IND16( shootouj )
{
	shootout_state *state = screen.machine().driver_data<shootout_state>();

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_background->draw(bitmap, cliprect, 0,0);
	state->m_foreground->draw(bitmap, cliprect, 0,1);
	draw_sprites(screen.machine(), bitmap,cliprect,2/*bank bits*/);
	return 0;
}
