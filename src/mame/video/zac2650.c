/*************************************************************/
/*                                                           */
/* Zaccaria/Zelco S2650 based games video                    */
/*                                                           */
/*************************************************************/

#include "emu.h"
#include "sound/s2636.h"
#include "includes/zac2650.h"


/**************************************************************/
/* The S2636 is a standard sprite chip used by several boards */
/* Emulation of this chip may be moved into a separate unit   */
/* once it's workings are fully understood.                   */
/**************************************************************/

WRITE8_MEMBER(zac2650_state::tinvader_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(zac2650_state::zac_s2636_r)
{
	if(offset!=0xCB) return m_s2636_0_ram[offset];
    else return m_CollisionSprite;
}

WRITE8_MEMBER(zac2650_state::zac_s2636_w)
{
	m_s2636_0_ram[offset] = data;
	gfx_element_mark_dirty(machine().gfx[1], offset/8);
	gfx_element_mark_dirty(machine().gfx[2], offset/8);
	if (offset == 0xc7)
	{
		s2636_soundport_w(machine().device("s2636snd"), 0, data);
	}
}

READ8_MEMBER(zac2650_state::tinvader_port_0_r)
{
	return input_port_read(machine(), "1E80") - m_CollisionBackground;
}

/*****************************************/
/* Check for Collision between 2 sprites */
/*****************************************/

static int SpriteCollision(running_machine &machine, int first,int second)
{
	zac2650_state *state = machine.driver_data<zac2650_state>();
	int Checksum=0;
	int x,y;
	const rectangle &visarea = machine.primary_screen->visible_area();

    if((state->m_s2636_0_ram[first * 0x10 + 10] < 0xf0) && (state->m_s2636_0_ram[second * 0x10 + 10] < 0xf0))
    {
    	int fx     = (state->m_s2636_0_ram[first * 0x10 + 10] * 4)-22;
        int fy     = (state->m_s2636_0_ram[first * 0x10 + 12] * 3)+3;
		int expand = (first==1) ? 2 : 1;

        /* Draw first sprite */

	    drawgfx_opaque(state->m_spritebitmap,state->m_spritebitmap.cliprect(), machine.gfx[expand],
			    first * 2,
			    0,
			    0,0,
			    fx,fy);

        /* Get fingerprint */

	    for (x = fx; x < fx + machine.gfx[expand]->width; x++)
	    {
		    for (y = fy; y < fy + machine.gfx[expand]->height; y++)
            {
			    if (visarea.contains(x, y))
	        	    Checksum += state->m_spritebitmap.pix16(y, x);
            }
	    }

        /* Blackout second sprite */

	    drawgfx_transpen(state->m_spritebitmap,state->m_spritebitmap.cliprect(), machine.gfx[1],
			    second * 2,
			    1,
			    0,0,
			    (state->m_s2636_0_ram[second * 0x10 + 10] * 4)-22,(state->m_s2636_0_ram[second * 0x10 + 12] * 3) + 3, 0);

        /* Remove fingerprint */

	    for (x = fx; x < fx + machine.gfx[expand]->width; x++)
	    {
		    for (y = fy; y < fy + machine.gfx[expand]->height; y++)
            {
			    if (visarea.contains(x, y))
	        	    Checksum -= state->m_spritebitmap.pix16(y, x);
            }
	    }

        /* Zero bitmap */

	    drawgfx_opaque(state->m_spritebitmap,state->m_spritebitmap.cliprect(), machine.gfx[expand],
			    first * 2,
			    1,
			    0,0,
			    fx,fy);
    }

	return Checksum;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	zac2650_state *state = machine.driver_data<zac2650_state>();
	UINT8 *videoram = state->m_videoram;
	int code = videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( tinvader )
{
	zac2650_state *state = machine.driver_data<zac2650_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 24, 24, 32, 32);

	machine.primary_screen->register_screen_bitmap(state->m_bitmap);
	machine.primary_screen->register_screen_bitmap(state->m_spritebitmap);

	gfx_element_set_source(machine.gfx[1], state->m_s2636_0_ram);
	gfx_element_set_source(machine.gfx[2], state->m_s2636_0_ram);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	zac2650_state *state = machine.driver_data<zac2650_state>();
	int offs;
	const rectangle &visarea = machine.primary_screen->visible_area();

    /* -------------------------------------------------------------- */
    /* There seems to be a strange setup with this board, in that it  */
    /* appears that the S2636 runs from a different clock than the    */
    /* background generator, When the program maps sprite position to */
    /* character position it only has 6 pixels of sprite for 8 pixels */
    /* of character.                                                  */
    /* -------------------------------------------------------------- */
    /* n.b. The original has several graphic glitches as well, so it  */
    /* does not seem to be a fault of the emulation!                  */
    /* -------------------------------------------------------------- */

    state->m_CollisionBackground = 0;	/* Read from 0x1e80 bit 7 */

	// for collision detection checking
	copybitmap(state->m_bitmap,bitmap,0,0,0,0,visarea);

    for(offs=0;offs<0x50;offs+=0x10)
    {
    	if((state->m_s2636_0_ram[offs+10]<0xF0) && (offs!=0x30))
		{
            int spriteno = (offs / 8);
			int expand   = ((state->m_s2636_0_ram[0xc0] & (spriteno*2))!=0) ? 2 : 1;
            int bx       = (state->m_s2636_0_ram[offs+10] * 4) - 22;
            int by       = (state->m_s2636_0_ram[offs+12] * 3) + 3;
            int x,y;

            /* Sprite->Background collision detection */
			drawgfx_transpen(bitmap,cliprect, machine.gfx[expand],
				    spriteno,
					1,
				    0,0,
				    bx,by, 0);

	        for (x = bx; x < bx + machine.gfx[expand]->width; x++)
	        {
		        for (y = by; y < by + machine.gfx[expand]->height; y++)
                {
			        if (visarea.contains(x, y))
	        	        if (bitmap.pix16(y, x) != state->m_bitmap.pix16(y, x))
	        	        {
	                    	state->m_CollisionBackground = 0x80;
					        break;
				        }
                }
	        }

			drawgfx_transpen(bitmap,cliprect, machine.gfx[expand],
				    spriteno,
					0,
				    0,0,
				    bx,by, 0);
        }
    }

    /* Sprite->Sprite collision detection */
    state->m_CollisionSprite = 0;
//  if(SpriteCollision(machine, 0,1)) state->m_CollisionSprite |= 0x20;   /* Not Used */
    if(SpriteCollision(machine, 0,2)) state->m_CollisionSprite |= 0x10;
    if(SpriteCollision(machine, 0,4)) state->m_CollisionSprite |= 0x08;
    if(SpriteCollision(machine, 1,2)) state->m_CollisionSprite |= 0x04;
    if(SpriteCollision(machine, 1,4)) state->m_CollisionSprite |= 0x02;
//  if(SpriteCollision(machine, 2,4)) state->m_CollisionSprite |= 0x01;   /* Not Used */
}

SCREEN_UPDATE_IND16( tinvader )
{
	zac2650_state *state = screen.machine().driver_data<zac2650_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
