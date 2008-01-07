/***************************************************************************

  video\cvs.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/s2636.h"
#include "cpu/s2650/s2650.h"

#define MAX_STARS        250
#define STARS_COLOR_BASE 16

#ifdef LSB_FIRST
#define BL0 0
#define BL1 1
#define BL2 2
#define BL3 3
#define WL0 0
#define WL1 1
#else
#define BL0 3
#define BL1 2
#define BL2 1
#define BL3 0
#define WL0 1
#define WL1 0
#endif

struct star
{
	int x,y,code;
};

static struct star stars[MAX_STARS];
static int    total_stars;
static int    scroll[8];
static int    stars_on=0;
static int 	  character_mode=0;
static int    character_page=0;
static int    scroll_reg = 0;
static int    stars_scroll=0;

static UINT8 *character_1_ram;
static UINT8 *character_2_ram;
static UINT8 *character_3_ram;
UINT8 *cvs_bullet_ram;

mame_bitmap *cvs_collision_bitmap;
mame_bitmap *cvs_collision_background;
static mame_bitmap *scrolled_background;

int cvs_collision_register=0;

static const int ModeOffset[4] = {223,191,255,127};

/******************************************************
 * Convert Colour prom to format for Mame Colour Map  *
 *                                                    *
 * There is a prom used for colour mapping and plane  *
 * priority. This is converted to a colour table here *
 *                                                    *
 * colours are taken from SRAM and are programmable   *
 ******************************************************/

PALETTE_INIT( cvs )
{
	int attr,col,map;

	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

    /* Colour Mapping Prom */

    for(attr = 0;attr < 256; attr++)
    {
    	for(col = 0; col < 8; col++)
        {
          	map = color_prom[(col * 256) + attr];

            /* bits 1 and 3 are swopped over */

            COLOR(0,attr*8 + col) = ((map & 1) << 2) + (map & 2) + ((map & 4) >> 2);
        }
    }

    /* Background Collision Map */

    for(map=0;map<8;map++)
    {
    	COLOR(0,2048+map) = (map & 4) >> 2;
        COLOR(0,2056+map) = (map & 2) >> 1;
        COLOR(0,2064+map) = ((map & 2) >> 1) || ((map & 4) >> 2);
    }

    /* Sprites */

    for(map=0;map<8;map++)
    {
    	COLOR(0,map*2 + 2072) = 0;
    	COLOR(0,map*2 + 2073) = 8 + map;
    }

    /* Initialise Dirty Character Array */
	character_1_ram = auto_malloc(0x2000);
	character_2_ram = character_1_ram + 0x800;
	character_3_ram = character_2_ram + 0x800;

    memset(character_1_ram, 0, 1024);
    memset(character_2_ram, 0, 1024);
    memset(character_3_ram, 0, 1024);

    /* Set Sprite chip offsets */

	s2636_x_offset = -26;
	s2636_y_offset = 3;

    /* Set Scroll fixed areas */

    scroll[0]=0;
    scroll[6]=0;
    scroll[7]=0;
}

WRITE8_HANDLER( cvs_video_fx_w )
{
	logerror("%4x : Data Port = %2x\n",activecpu_get_pc(),data);

    /* Unimplemented */

    if(data & 2)   logerror("       SHADE BRIGHTER TO RIGHT\n");
    if(data & 4)   logerror("       SCREEN ROTATE\n");
    if(data & 8)   logerror("       SHADE BRIGHTER TO LEFT\n");
    if(data & 64)  logerror("       SHADE BRIGHTER TO BOTTOM\n");
    if(data & 128) logerror("       SHADE BRIGHTER TO TOP\n");

    /* Implemented */

    stars_on = data & 1;
    set_led_status(1,data & 16);	/* Lamp 1 */
    set_led_status(2,data & 32);	/* Lamp 2 */
}

READ8_HANDLER( cvs_character_mode_r )
{
	/* Really a write - uses address info */

    int value   = offset + 0x10;
    int newmode = (value >> 4) & 3;

    character_mode = newmode;
    character_page = (value << 2) & 0x300;

    return 0;
}

READ8_HANDLER( cvs_collision_r )
{
	return cvs_collision_register;
}

READ8_HANDLER( cvs_collision_clear )
{
	cvs_collision_register=0;
	return 0;
}

WRITE8_HANDLER( cvs_scroll_w )
{
	scroll_reg = 255 - data;

    scroll[1]=scroll_reg;
    scroll[2]=scroll_reg;
    scroll[3]=scroll_reg;
    scroll[4]=scroll_reg;
    scroll[5]=scroll_reg;
}

WRITE8_HANDLER( cvs_videoram_w )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// Colour Map

        colorram_w(offset,data);
    }
    else
    {
    	// Data

        videoram_w(offset,data);
    }
}

READ8_HANDLER( cvs_videoram_r )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// Colour Map

        return colorram[offset];
    }
    else
    {
    	// Data

        return videoram[offset];
    }
}

WRITE8_HANDLER( cvs_bullet_w )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// Bullet Ram

        cvs_bullet_ram[offset] = data;
    }
    else
    {
    	// Pallette Ram - Inverted ?
		offset &= 0x0f;
		data ^= 0xff;

		paletteram[offset] = data;
		palette_set_color_rgb(Machine, offset, pal2bit(data >> 0), pal3bit(data >> 2), pal3bit(data >> 5));
    }
}

READ8_HANDLER( cvs_bullet_r )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// Bullet Ram

        return cvs_bullet_ram[offset];
    }
    else
    {
    	// Pallette Ram

        return (paletteram[offset] ^ 0xff);
    }
}

WRITE8_HANDLER( cvs_2636_1_w )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// First 2636

        s2636_1_ram[offset] = data;
    }
    else
    {
    	// Character Ram 1
       	character_1_ram[character_page + offset] = data;
	}
}

READ8_HANDLER( cvs_2636_1_r )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// First 2636

        return s2636_1_ram[offset];
    }
    else
    {
    	// Character Ram 1

        return character_1_ram[character_page + offset];
    }
}

WRITE8_HANDLER( cvs_2636_2_w )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// Second 2636

        s2636_2_ram[offset] = data;
    }
    else
    {
    	// Character Ram 2
       	character_2_ram[character_page + offset] = data;
    }
}

READ8_HANDLER( cvs_2636_2_r )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// Second 2636

        return s2636_2_ram[offset];
    }
    else
    {
    	// Character Ram 2

        return character_2_ram[character_page + offset];
    }
}

WRITE8_HANDLER( cvs_2636_3_w )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// Third 2636

        s2636_3_ram[offset] = data;
    }
    else
    {
    	// Character Ram 3
       	character_3_ram[character_page + offset] = data;
    }
}

READ8_HANDLER( cvs_2636_3_r )
{
	if(!activecpu_get_reg(S2650_FO))
    {
    	// Third 2636

        return s2636_3_ram[offset];
    }
    else
    {
    	// Character Ram 3

        return character_3_ram[character_page + offset];
    }
}

VIDEO_START( cvs )
{
	int generator = 0;
    int x,y;

	colorram = auto_malloc(0x400);
	paletteram = auto_malloc(0x100);

	video_start_generic(machine);

	/* precalculate the star background */

	total_stars = 0;

	for (y = 255;y >= 0;y--)
	{
		for (x = 511;x >= 0;x--)
		{
			int bit1,bit2;

			generator <<= 1;
			bit1 = (~generator >> 17) & 1;
			bit2 = (generator >> 5) & 1;

			if (bit1 ^ bit2) generator |= 1;

			if (((~generator >> 16) & 1) && (generator & 0xfe) == 0xfe)
			{
            	if(((~(generator >> 12)) & 0x01) && ((~(generator >> 13)) & 0x01))
                {
				    if (total_stars < MAX_STARS)
				    {
					    stars[total_stars].x = x;
					    stars[total_stars].y = y;
					    stars[total_stars].code = 1;

					    total_stars++;
				    }
                }
			}
		}
	}

	/* Need 3 bitmaps for 2636 chips */

	s2636_1_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,BITMAP_FORMAT_INDEXED8);
	s2636_2_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,BITMAP_FORMAT_INDEXED8);
	s2636_3_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,BITMAP_FORMAT_INDEXED8);

	/* 3 bitmaps for collision detection */

	cvs_collision_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,BITMAP_FORMAT_INDEXED8);
	cvs_collision_background = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,BITMAP_FORMAT_INDEXED8);
	scrolled_background = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,BITMAP_FORMAT_INDEXED8);
}

INTERRUPT_GEN( cvs_interrupt )
{
	stars_scroll++;

	cpunum_set_input_line_vector(0,0,0x03);
	cpunum_set_input_line(0,0,PULSE_LINE);
}

INLINE void plot_star(running_machine* machine, mame_bitmap *bitmap, int x, int y)
{
	if (flip_screen_x)
	{
		x = 255 - x;
	}
	if (flip_screen_y)
	{
		y = 255 - y;
	}

	if (*BITMAP_ADDR16(bitmap, y, x) == machine->pens[0])
	{
		*BITMAP_ADDR16(bitmap, y, x) = machine->pens[7];
	}
}

VIDEO_UPDATE( cvs )
{
	int offs;

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */

	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int character_bank;
		int forecolor;

        int character = videoram[offs];

		int sx = (offs % 32) * 8;
		int sy = (offs / 32) * 8;

		/* Decide if RAM or ROM based character */

		if(character > ModeOffset[character_mode])
		{
			decodechar(machine->gfx[1],character,character_1_ram-1024,machine->drv->gfxdecodeinfo[1].gfxlayout);

			character_bank=1;
		}
		else
		{
			character_bank=0;
		}

		/* Main Screen */

		drawgfx(tmpbitmap,machine->gfx[character_bank],
				character,
				colorram[offs],
				0,0,
				sx,sy,
				0,TRANSPARENCY_NONE,0);


		/* Foreground for Collision Detection */

		forecolor = 0;
		if(colorram[offs] & 0x80)
		{
			forecolor=258;
		}
		else
		{
			if((colorram[offs] & 0x03) == 3) forecolor=256;
			else if((colorram[offs] & 0x01) == 0) forecolor=257;
		}

		if(forecolor)
			drawgfx(cvs_collision_background,machine->gfx[character_bank],
					character,
					forecolor,
					0,0,
					sx,sy,
					0,TRANSPARENCY_NONE,0);
	}


    /* Update screen - 8 regions, fixed scrolling area */

	copyscrollbitmap(bitmap,tmpbitmap,0,0,8,scroll,&machine->screen[0].visarea,TRANSPARENCY_NONE,0);
	copyscrollbitmap(scrolled_background,cvs_collision_background,0,0,8,scroll,&machine->screen[0].visarea,TRANSPARENCY_NONE,0);

    /* 2636's */

	fillbitmap(s2636_1_bitmap,0,0);
	s2636_update_bitmap(machine,s2636_1_bitmap,s2636_1_ram,2,cvs_collision_bitmap);

	fillbitmap(s2636_2_bitmap,0,0);
	s2636_update_bitmap(machine,s2636_2_bitmap,s2636_2_ram,3,cvs_collision_bitmap);

	fillbitmap(s2636_3_bitmap,0,0);
	s2636_update_bitmap(machine,s2636_3_bitmap,s2636_3_ram,4,cvs_collision_bitmap);

    /* Bullet Hardware */

    for (offs = 8; offs < 256; offs++ )
    {
        if(cvs_bullet_ram[offs] != 0)
        {
        	int ct;
            for(ct=0;ct<4;ct++)
            {
            	int bx=255-7-cvs_bullet_ram[offs]-ct;

            	/* Bullet/Object Collision */

                if((cvs_collision_register & 8) == 0)
                {
                    if ((*BITMAP_ADDR8(s2636_1_bitmap, offs, bx) != 0) ||
					    (*BITMAP_ADDR8(s2636_2_bitmap, offs, bx) != 0) ||
					    (*BITMAP_ADDR8(s2636_3_bitmap, offs, bx) != 0))
                        cvs_collision_register |= 8;
                }

            	/* Bullet/Background Collision */

                if((cvs_collision_register & 0x80) == 0)
                {
					if (*BITMAP_ADDR8(scrolled_background, offs, bx) != machine->pens[0])
                    	cvs_collision_register |= 0x80;
                }

				*BITMAP_ADDR16(bitmap, offs, bx) = machine->pens[7];
            }
        }
    }

    /* Update 2636 images */

    {
		int sx;
        UINT32 S1,S2,S3,SB,pen;

        for(sx=255;sx>7;sx--)
        {
        	UINT32 *sp1 = (UINT32 *)BITMAP_ADDR8(s2636_1_bitmap, sx, 0);
	    	UINT32 *sp2 = (UINT32 *)BITMAP_ADDR8(s2636_2_bitmap, sx, 0);
		    UINT32 *sp3 = (UINT32 *)BITMAP_ADDR8(s2636_3_bitmap, sx, 0);
	        UINT64 *dst = (UINT64 *)BITMAP_ADDR16(bitmap, sx, 0);
		    UINT8  *spb = (UINT8  *)BITMAP_ADDR8(scrolled_background, sx, 0);

            for(offs=0;offs<62;offs++)
            {
        	     S1 = (*sp1++);
                 S2 = (*sp2++);
                 S3 = (*sp3++);

        	     pen = S1 | S2 | S3;

                 if(pen)
                 {
             	    UINT16 *address = (UINT16 *)dst;
				    if (pen & 0xff000000) address[BL3] = machine->pens[(pen >> 24) & 15];
				    if (pen & 0x00ff0000) address[BL2] = machine->pens[(pen >> 16) & 15];
				    if (pen & 0x0000ff00) address[BL1] = machine->pens[(pen >>  8) & 15];
				    if (pen & 0x000000ff) address[BL0] = machine->pens[(pen & 15)];

                    /* Collision Detection */

                    SB = 0;
				    if (spb[BL3] != machine->pens[0]) SB =  0x08000000;
				    if (spb[BL2] != machine->pens[0]) SB |= 0x00080000;
				    if (spb[BL1] != machine->pens[0]) SB |= 0x00000800;
				    if (spb[BL0] != machine->pens[0]) SB |= 0x00000008;

       	            if (S1 & S2) cvs_collision_register |= 1;
       	            if (S2 & S3) cvs_collision_register |= 2;
    			    if (S1 & S3) cvs_collision_register |= 4;

                    if (SB)
                    {
    			        if (S1 & SB) cvs_collision_register |= 16;
   			            if (S2 & SB) cvs_collision_register |= 32;
       	                if (S3 & SB) cvs_collision_register |= 64;
                    }
                 }

           	     dst++;
                 spb+=4;
            }
        }
    }

    /* Stars */

    if(stars_on)
    {
		for (offs = 0;offs < total_stars;offs++)
		{
			int x,y;


			x = ((stars[offs].x + stars_scroll) % 512) / 2;
			y = (stars[offs].y + (stars_scroll + stars[offs].x) / 512) % 256;

			if (y >= machine->screen[0].visarea.min_y &&
				y <= machine->screen[0].visarea.max_y)
			{
				if ((y & 1) ^ ((x >> 4) & 1))
				{
					plot_star(machine, bitmap, x, y);
				}
			}
		}

    }
	return 0;
}
