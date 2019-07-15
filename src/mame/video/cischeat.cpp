// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Jaleco Driving Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z or X with:

        Q,W,E       shows scroll 0,1,2
        R,T         shows road 0,1
        A,S,D,F     shows sprites with priority 0,1,2,3
        X           shows some info on each sprite
        M           disables sprites zooming
        U           toggles the display of some hardware registers'
                    values ($80000/2/4/6)

        Keys can be used together!
        Additionally, layers can be disabled, writing to $82400
        (fake video register):

            ---- ---- --54 ----     Enable Road 1,0
            ---- ---- ---- 3---     Enable Sprites
            ---- ---- ---- -210     Enable Scroll 2,1,0

        0 is the same as 0x3f

[ 3 Scrolling Layers ]

    see ms1_tmap.cpp

[ 2 Road Layers ]

    Each of the 256 (not all visible) lines of the screen
    can display any of the lines of gfx in ROM, which are
    larger than the screen and can therefore be scrolled

                                    Cisco Heat              F1 GP Star
                Line Width          1024                    1024
                Zoom                No                      Yes

[ 256 Sprites ]

    Sprites are made of several 16x16 tiles (up to 256x256 pixels)
    and can be zoomed in and out. See below for sprite RAM format.

***************************************************************************/

#include "emu.h"
#include "includes/cischeat.h"


#define cischeat_tmap_DRAW(_n_) \
	if ( (m_tmap[_n_]).found() && (active_layers1 & (1 << _n_) ) ) \
	{ \
		m_tmap[_n_]->draw(screen, bitmap, cliprect, flag, 0 ); \
		flag = 0; \
	}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void cischeat_state::prepare_shadows()
{
	int i;
	for (i = 0;i < 16;i++)
		m_drawmode_table[i] = DRAWMODE_SOURCE;

	m_drawmode_table[ 0] = DRAWMODE_SHADOW;
	m_drawmode_table[15] = DRAWMODE_NONE;
}

void cischeat_state::video_start()
{
	m_spriteram = &m_ram[0x8000/2];

	m_active_layers = 0;

	prepare_shadows();

	m_motor_value = 0;
	m_io_value = 0;
}

void wildplt_state::video_start()
{
	cischeat_state::video_start();
	m_buffer_spriteram = &m_ram[0x8000/2];
	m_spriteram = auto_alloc_array(machine(),uint16_t, 0x1000/2);
}

WRITE16_MEMBER(wildplt_state::sprite_dma_w)
{
	// bit 13: 0 -> 1 transition triggers a sprite DMA
	if(data & 0x2000 && (m_sprite_dma_reg & 0x2000) == 0)
	{
		for(int i=0;i<0x1000/2;i++)
			m_spriteram[i] = m_buffer_spriteram[i];
	}

	// other bits unknown
	COMBINE_DATA(&m_sprite_dma_reg);
}

/***************************************************************************


                        Hardware registers access


***************************************************************************/

/*
    F1 GP Star has a real pedal, while Cisco Heat's is connected to
    a switch. The Former game stores, during boot, the value that
    corresponds to the pedal not pressed, and compares against it:

    The value returned must decrease when the pedal is pressed.
*/

/**************************************************************************
                                Big Run
**************************************************************************/

READ16_MEMBER(cischeat_state::bigrun_ip_select_r)
{
	switch (m_ip_select & 0x3)
	{
		case 0 : return ioport("IN6")->read();      // Driving Wheel
		case 1 : return 0xffff;                 // Cockpit: Up / Down Position
		case 2 : return 0xffff;                 // Cockpit: Left / Right Position?
		case 3 : return ioport("PEDAL")->read();    // Accelerator (Pedal)
		default: return 0xffff;
	}
}


WRITE16_MEMBER(cischeat_state::leds_out_w)
{
	// leds
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);
		m_leds[0] = BIT(data, 4);   // start button
		m_leds[1] = BIT(data, 5);   // ?
	}
}


WRITE16_MEMBER(cischeat_state::unknown_out_w)
{
	// ?? 91/1/91/1 ...
}


WRITE16_MEMBER(cischeat_state::motor_out_w)
{
	// motor (seat?)
	if (ACCESSING_BITS_0_7)
		m_leds[2] = (data & 0xff) != m_motor_value ? 1 : 0;
	m_motor_value = data & 0xff;
}


WRITE16_MEMBER(cischeat_state::wheel_out_w)
{
	// motor (wheel?)
}


WRITE16_MEMBER(cischeat_state::ip_select_w)
{
	m_ip_select = data;
}


WRITE16_MEMBER(cischeat_state::ip_select_plus1_w)
{
	// value above + 1
	m_ip_select = data + 1;
}


WRITE16_MEMBER(cischeat_state::bigrun_comms_w)
{
	/* Not sure about this one.. */
	m_cpu2->set_input_line(INPUT_LINE_RESET, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_cpu3->set_input_line(INPUT_LINE_RESET, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? ASSERT_LINE : CLEAR_LINE);
}

// TODO: fake port, never written to my knowledge!
WRITE16_MEMBER(cischeat_state::active_layers_w)
{
	COMBINE_DATA(&m_active_layers);
}


/**************************************************************************
                                Cisco Heat
**************************************************************************/

READ16_MEMBER(cischeat_state::cischeat_ip_select_r)
{
	switch (m_ip_select & 0x3)
	{
		case 0 : return ioport("IN6")->read();  // Driving Wheel
		case 1 : return ~0;                 // Cockpit: Up / Down Position?
		case 2 : return ~0;                 // Cockpit: Left / Right Position?
		default: return ~0;
	}
}


WRITE16_MEMBER(cischeat_state::cischeat_soundlatch_w)
{
	/* Sound CPU: reads latch during int 4, and stores command */
	m_soundlatch->write(data);
	m_soundcpu->set_input_line(4, HOLD_LINE);
}


WRITE16_MEMBER(cischeat_state::cischeat_comms_w)
{
	/* Not sure about this one.. */
	m_cpu2->set_input_line(INPUT_LINE_RESET, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_cpu3->set_input_line(INPUT_LINE_RESET, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? ASSERT_LINE : CLEAR_LINE);
}



/**************************************************************************
                            F1 GrandPrix Star
**************************************************************************/

READ16_MEMBER(cischeat_state::f1gpstar_wheel_r)
{
	return (ioport("PEDAL")->read() & 0xff) + ((ioport("IN5")->read() & 0xff)<<8);
}


READ16_MEMBER(cischeat_state::f1gpstr2_ioready_r)
{
	return (m_f1gpstr2_ioready[0]&1) ? 0xff : 0xf0;
}


/**************************************************************************
                            Wild Pilot
**************************************************************************/


WRITE16_MEMBER(cischeat_state::f1gpstar_motor_w)
{
	// "shudder" motors, leds
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);
		m_leds[0] = BIT(data, 2);   // start button
		m_leds[1] = BIT(data, 5);   // ?
		// wheel | seat motor
		m_leds[2] = BIT(data, 3) | BIT(data, 4);
	}
}


WRITE16_MEMBER(cischeat_state::f1gpstar_soundint_w)
{
	/* $80008 and $80018 usually written in sequence, but not always */
	m_soundcpu->set_input_line(4, HOLD_LINE);
}


WRITE16_MEMBER(cischeat_state::f1gpstar_comms_w)
{
	/* Not sure about this one. Values: $10 then 0, $7 then 0 */
	m_cpu2->set_input_line(INPUT_LINE_RESET, (data & 1) ? ASSERT_LINE : CLEAR_LINE);
	m_cpu3->set_input_line(INPUT_LINE_RESET, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 4) ? ASSERT_LINE : CLEAR_LINE);
}


WRITE16_MEMBER(cischeat_state::f1gpstr2_io_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if ((m_io_value & 4) && ((data & 4) == 0))
			m_cpu5->set_input_line(4, HOLD_LINE);
		if ((m_io_value & 2) && ((data & 2) == 0))
			m_cpu5->set_input_line(2, HOLD_LINE);
		m_io_value = data & 0xff;
	}
}




/***************************************************************************


                                Road Drawing


***************************************************************************/

/* horizontal size of 1 line and 1 tile of the road */
#define X_SIZE (1024)
#define TILE_SIZE (64)


/**************************************************************************

                        Cisco Heat road format


    Offset:     Bits:                   Value:

    00.w                                Code

    02.w                                X Scroll

    04.w        fedc ---- ---- ----     unused?
                ---- ba98 ---- ----     Priority
                ---- ---- 76-- ----     unused?
                ---- ---- --54 3210     Color

    06.w                                Unused

**************************************************************************/

/*  Draw the road in the given bitmap. The priority1 and priority2 parameters
    specify the range of lines to draw  */

void cischeat_state::cischeat_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect, int road_num, int priority1, int priority2, int transparency)
{
	int curr_code,sx,sy;
	int min_priority, max_priority;

	rectangle rect      =   cliprect;
	gfx_element *gfx        =   m_gfxdecode->gfx((road_num & 1) ? 2 : 1);

	uint16_t *roadram         =   m_roadram[road_num & 1];

	int min_y = rect.min_y;
	int max_y = rect.max_y;

	int max_x = rect.max_x;

	if (priority1 < priority2)  {   min_priority = priority1;   max_priority = priority2; }
	else                        {   min_priority = priority2;   max_priority = priority1; }

	/* Move the priority values in place */
	min_priority = (min_priority & 7) * 0x100;
	max_priority = (max_priority & 7) * 0x100;

	/* Let's draw from the top to the bottom of the visible screen */
	for (sy = min_y ; sy <= max_y ; sy ++)
	{
		int code    = roadram[ sy * 4 + 0 ];
		int xscroll = roadram[ sy * 4 + 1 ];
		int attr    = roadram[ sy * 4 + 2 ];

		/* high byte is a priority information */
		if ( ((attr & 0x700) < min_priority) || ((attr & 0x700) > max_priority) )
			continue;

		/* line number converted to tile number (each tile is TILE_SIZE x 1) */
		code = code * (X_SIZE/TILE_SIZE);

		xscroll %= X_SIZE;
		curr_code = code + xscroll/TILE_SIZE;

		for (sx = -(xscroll%TILE_SIZE) ; sx <= max_x ; sx +=TILE_SIZE)
		{
			gfx->transpen(bitmap,rect,
					curr_code++,
					attr,
					0,0,
					sx,sy,
					transparency ? 15 : -1);

			/* wrap around */
			if (curr_code%(X_SIZE/TILE_SIZE)==0)    curr_code = code;
		}
	}

}



/**************************************************************************

                        F1 GrandPrix Star road format

    Offset:     Bits:                   Value:

    00.w        fedc ---- ---- ----     Priority
                ---- ba98 7654 3210     X Scroll (After Zoom)

    02.w        fedc ba-- ---- ----     unused?
                ---- --98 7654 3210     X Zoom

    04.w        fe-- ---- ---- ----     unused?
                --dc ba98 ---- ----     Color
                ---- ---- 7654 3210     ?

    06.w                                Code


    Imagine an "empty" line, 2 * X_SIZE wide, with the gfx from
    the ROM - whose original size is X_SIZE - in the middle.

    Zooming acts on this latter and can shrink it to 1 pixel or
    widen it to 2 * X_SIZE, while *keeping it centered* in the
    empty line. Scrolling acts on the resulting line.


**************************************************************************/

/*  Draw the road in the given bitmap. The priority1 and priority2 parameters
    specify the range of lines to draw  */

void cischeat_state::f1gpstar_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect, int road_num, int priority1, int priority2, int transparency)
{
	int sx,sy;
	int xstart;
	int min_priority, max_priority;

	rectangle rect      =   cliprect;
	gfx_element *gfx        =   m_gfxdecode->gfx((road_num & 1) ? 2 : 1);

	uint16_t *roadram         =   m_roadram[road_num & 1];

	int min_y = rect.min_y;
	int max_y = rect.max_y;

	int max_x = rect.max_x << 16;   // use fixed point values (16.16), for accuracy

	if (priority1 < priority2)  {   min_priority = priority1;   max_priority = priority2; }
	else                        {   min_priority = priority2;   max_priority = priority1; }

	/* Move the priority values in place */
	min_priority = (min_priority & 7) * 0x1000;
	max_priority = (max_priority & 7) * 0x1000;

	/* Let's draw from the top to the bottom of the visible screen */
	for (sy = min_y ; sy <= max_y ; sy ++)
	{
		int xscale, xdim;

		int xscroll = roadram[ sy * 4 + 0 ];
		int xzoom   = roadram[ sy * 4 + 1 ];
		int attr    = roadram[ sy * 4 + 2 ];
		int code    = roadram[ sy * 4 + 3 ];

		/* highest nibble is a priority information */
		if ( ((xscroll & 0x7000) < min_priority) || ((xscroll & 0x7000) > max_priority) )
			continue;

		/* zoom code range: 000-3ff     scale range: 0.0-2.0 */
		xscale = ( ((xzoom & 0x3ff)+1) << (16+1) ) / 0x400;

		/* line number converted to tile number (each tile is TILE_SIZE x 1) */
		code    = code * (X_SIZE/TILE_SIZE);

		/* dimension of a tile after zoom */
		xdim = TILE_SIZE * xscale;

		xscroll %= 2 * X_SIZE;

		xstart  = (X_SIZE - xscroll) * 0x10000;
		xstart -= (X_SIZE * xscale) / 2;

		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		xscale += (1<<16)/TILE_SIZE;

		/* Draw the line */
		for (sx = xstart ; sx <= max_x ; sx += xdim)
		{
			gfx->zoom_transpen(bitmap,rect,
						code++,
						attr >> 8,
						0,0,
						sx / 0x10000, sy,
						xscale, 1 << 16,
						transparency ? 15 : -1);

			/* stop when the end of the line of gfx is reached */
			if ((code % (X_SIZE/TILE_SIZE)) == 0)   break;
		}

	}
}


/***************************************************************************

                Cisco Heat & F1 GP Star Sprites Drawing

    Offset: Bits:                   Value:

    00      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Don't display this sprite
            ---- ba98 ---- ----     unused?
            ---- ---- 7654 ----     Number of tiles along Y, minus 1 (1-16)
            ---- ---- ---- 3210     Number of tiles along X, minus 1 (1-16)

    02/04   fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Flip X/Y
            ---- ba9- ---- ----     ? X/Y zoom ?
            ---- ---8 7654 3210     X/Y zoom

    06/08   fedc ba-- ---- ----     ? X/Y position ?
            ---- --98 7654 3210     X/Y position

    0A                              0 ?

    0C                              Code

    0E      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Use pen 0 as shadow
            ---- ba98 ---- ----     Priority
            ---- ---- 7--- ----     unused?
            ---- ---- -654 3210     Color

***************************************************************************/

#define SHRINK(_org_,_fact_) ( ( ( (_org_) << 16 ) * (_fact_ & 0x01ff) ) / 0x80 )

/*  Draw sprites, in the given priority range, to a bitmap.

    Priorities between 0 and 15 cover sprites whose priority nibble
    is between 0 and 15. Priorities between 0+16 and 15+16 cover
    sprites whose priority nibble is between 0 and 15 and whose
    colour code's high bit is set.  */

void cischeat_state::cischeat_draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2)
{
	int x, sx, flipx, xzoom, xscale, xdim, xnum, xstart, xend, xinc;
	int y, sy, flipy, yzoom, yscale, ydim, ynum, ystart, yend, yinc;
	int code, attr, color, size, shadow;

	int min_priority, max_priority, high_sprites;

	uint16_t      *source =   m_spriteram;
	const uint16_t    *finish =   source + 0x1000/2;


	/* Move the priority values in place */
	high_sprites = (priority1 >= 16) | (priority2 >= 16);
	priority1 = (priority1 & 0x0f) * 0x100;
	priority2 = (priority2 & 0x0f) * 0x100;

	if (priority1 < priority2)  {   min_priority = priority1;   max_priority = priority2; }
	else                        {   min_priority = priority2;   max_priority = priority1; }

	for (; source < finish; source += 0x10/2 )
	{
		size    =   source[ 0 ];
		if (size & 0x1000)  continue;

		/* number of tiles */
		xnum    =   ( (size & 0x0f) >> 0 ) + 1;
		ynum    =   ( (size & 0xf0) >> 4 ) + 1;

		xzoom   =   source[ 1 ];
		yzoom   =   source[ 2 ];
		flipx   =   xzoom & 0x1000;
		flipy   =   yzoom & 0x1000;

		sx      =   source[ 3 ];
		sy      =   source[ 4 ];
		// TODO: was & 0x1ff with 0x200 as sprite wrap sign, looks incorrect with Grand Prix Star
		//       during big car on side view in attract mode (a tyre gets stuck on the right of the screen)
		//       this arrangement works with both games (otherwise Part 2 gets misaligned bleachers sprites)
		sx      =   (sx & 0x7ff);
		sy      =   (sy & 0x7ff);
		if(sx & 0x400)
			sx -= 0x800;
		if(sy & 0x400)
			sy -= 0x800;

		/* use fixed point values (16.16), for accuracy */
		sx <<= 16;
		sy <<= 16;

		/* dimension of a tile after zoom */
#ifdef MAME_DEBUG
		if ( machine().input().code_pressed(KEYCODE_Z) && machine().input().code_pressed(KEYCODE_M) )
		{
			xdim    =   16 << 16;
			ydim    =   16 << 16;
		}
		else
#endif
		{
			xdim    =   SHRINK(16,xzoom);
			ydim    =   SHRINK(16,yzoom);
		}

		if ( ( (xdim / 0x10000) == 0 ) || ( (ydim / 0x10000) == 0) )    continue;

		/* the y pos passed to the hardware is the that of the last line,
		   we need the y pos of the first line  */
		sy -= (ydim * ynum);

		code    =   source[ 6 ];
		attr    =   source[ 7 ];
		color   =   attr & 0x007f;
		shadow  =   attr & 0x1000;

		/* high byte is a priority information */
		if ( ((attr & 0x700) < min_priority) || ((attr & 0x700) > max_priority) )
			continue;

		if ( high_sprites && !(color & 0x80) )
			continue;

#ifdef MAME_DEBUG
if ( (m_debugsprites) && ( ((attr & 0x0300)>>8) != (m_debugsprites-1) ) ) { continue; };
#endif

		xscale = xdim / 16;
		yscale = ydim / 16;


		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		if (xscale & 0xffff)    xscale += (1<<16)/16;
		if (yscale & 0xffff)    yscale += (1<<16)/16;


		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }

		m_drawmode_table[ 0] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				m_gfxdecode->gfx(0)->zoom_transtable(bitmap,cliprect,code++,color,flipx,flipy,
							(sx + x * xdim) / 0x10000, (sy + y * ydim) / 0x10000,
							xscale, yscale, m_drawmode_table);
			}
		}
#ifdef MAME_DEBUG
#if 0
if (machine().input().code_pressed(KEYCODE_X))
{   /* Display some info on each sprite */
	sprintf(buf, "%04x",attr);
	ui_draw_text(buf, sx>>16, sy>>16);
}
#endif
#endif
	}   /* end sprite loop */
}


/***************************************************************************

                            Big Run Sprites Drawing

    Offset: Bits:                   Value:

    00      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Don't display this sprite
            ---- ba98 ---- ----     unused?
            ---- ---- 7654 ----     Number of tiles along Y, minus 1 (1-16)
            ---- ---- ---- 3210     Number of tiles along X, minus 1 (1-16)

    02      fedc ba98 ---- ----     Y zoom
            ---- ---- 7654 3210     X zoom

    04/06   fed- ---- ---- ----
            ---c ---- ---- ----     X/Y flip
            ---- ba9- ---- ----
            ---- ---8 7654 3210     X/Y position (signed)

    08                              ?
    0A                              ?

    0C                              Code

    0E      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Use pen 0 as shadow
            ---- ba98 ---- ----     Priority
            ---- ---- 76-- ----     unused?
            ---- ---- --54 3210     Color

***************************************************************************/

void cischeat_state::bigrun_draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2)
{
	int x, sx, flipx, xzoom, xscale, xdim, xnum, xstart, xend, xinc;
	int y, sy, flipy, yzoom, yscale, ydim, ynum, ystart, yend, yinc;
	int code, attr, color, size, shadow;

	int min_priority, max_priority, high_sprites;

	uint16_t      *source =   m_spriteram;
	const uint16_t    *finish =   source + 0x1000/2;

	/* Move the priority values in place */
	high_sprites = (priority1 >= 16) | (priority2 >= 16);
	priority1 = (priority1 & 0x0f) * 0x100;
	priority2 = (priority2 & 0x0f) * 0x100;

	if (priority1 < priority2)  {   min_priority = priority1;   max_priority = priority2; }
	else                        {   min_priority = priority2;   max_priority = priority1; }

	for (; source < finish; source += 0x10/2 )
	{
		size    =   source[ 0 ];
		if (size & 0x1000)  continue;

		/* number of tiles */
		xnum    =   ( (size & 0x0f) >> 0 ) + 1;
		ynum    =   ( (size & 0xf0) >> 4 ) + 1;

		yzoom   =   (source[ 1 ] >> 8) & 0xff;
		xzoom   =   (source[ 1 ] >> 0) & 0xff;

		sx      =   source[ 2 ];
		sy      =   source[ 3 ];
		flipx   =   sx & 0x1000;
		flipy   =   sy & 0x1000;
//      sx      =   (sx & 0x1ff) - (sx & 0x200);
//      sy      =   (sy & 0x1ff) - (sy & 0x200);
		sx      =   (sx & 0x0ff) - (sx & 0x100);
		sy      =   (sy & 0x0ff) - (sy & 0x100);

		/* use fixed point values (16.16), for accuracy */
		sx <<= 16;
		sy <<= 16;

		/* dimension of a tile after zoom */
#ifdef MAME_DEBUG
		if ( machine().input().code_pressed(KEYCODE_Z) && machine().input().code_pressed(KEYCODE_M) )
		{
			xdim    =   16 << 16;
			ydim    =   16 << 16;
		}
		else
#endif
		{
			xdim    =   SHRINK(16,xzoom);
			ydim    =   SHRINK(16,yzoom);
		}

		if ( ( (xdim / 0x10000) == 0 ) || ( (ydim / 0x10000) == 0) )    continue;

//      sy -= (ydim * ynum);

		code    =   source[ 6 ];
		attr    =   source[ 7 ];
		color   =   attr & 0x007f;
		shadow  =   attr & 0x1000;

		/* high byte is a priority information */
		if ( ((attr & 0x700) < min_priority) || ((attr & 0x700) > max_priority) )
			continue;

		if ( high_sprites && !(color & 0x80) )
			continue;

#ifdef MAME_DEBUG
if ( (m_debugsprites) && ( ((attr & 0x0300)>>8) != (m_debugsprites-1) ) ) { continue; };
#endif

		xscale = xdim / 16;
		yscale = ydim / 16;


		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		if (xscale & 0xffff)    xscale += (1<<16)/16;
		if (yscale & 0xffff)    yscale += (1<<16)/16;


		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }

		m_drawmode_table[ 0] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				m_gfxdecode->gfx(0)->zoom_transtable(bitmap,cliprect,code++,color,flipx,flipy,
							(sx + x * xdim) / 0x10000, (sy + y * ydim) / 0x10000,
							xscale, yscale, m_drawmode_table);
			}
		}
#ifdef MAME_DEBUG
#if 0
if (machine().input().code_pressed(KEYCODE_X))
{   /* Display some info on each sprite */
	sprintf(buf, "%04x",attr);
	ui_draw_text(buf, sx>>16, sy>>16);
}
#endif
#endif
	}   /* end sprite loop */
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

#ifdef MAME_DEBUG
#define CISCHEAT_LAYERSCTRL \
m_debugsprites = 0; \
if ( machine().input().code_pressed(KEYCODE_Z) || machine().input().code_pressed(KEYCODE_X) ) \
{ \
	int msk = 0; \
	if (machine().input().code_pressed(KEYCODE_Q))  { msk |= 0x01;} \
	if (machine().input().code_pressed(KEYCODE_W))  { msk |= 0x02;} \
	if (machine().input().code_pressed(KEYCODE_E))  { msk |= 0x04;} \
	if (machine().input().code_pressed(KEYCODE_A))  { msk |= 0x08; m_debugsprites = 1;} \
	if (machine().input().code_pressed(KEYCODE_S))  { msk |= 0x08; m_debugsprites = 2;} \
	if (machine().input().code_pressed(KEYCODE_D))  { msk |= 0x08; m_debugsprites = 3;} \
	if (machine().input().code_pressed(KEYCODE_F))  { msk |= 0x08; m_debugsprites = 4;} \
	if (machine().input().code_pressed(KEYCODE_R))  { msk |= 0x10;} \
	if (machine().input().code_pressed(KEYCODE_T))  { msk |= 0x20;} \
	\
	if (msk != 0) active_layers1 &= msk; \
} \
\
{ \
	if ( machine().input().code_pressed(KEYCODE_Z) && machine().input().code_pressed_once(KEYCODE_U) ) \
		m_show_unknown ^= 1; \
}
#else
#define CISCHEAT_LAYERSCTL
#endif

/**************************************************************************
                                Big Run
**************************************************************************/

uint32_t cischeat_state::screen_update_bigrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	bitmap.fill(0x1000, cliprect);

	for (i = 7; i >= 4; i--)
	{
		/* bitmap, cliprect, road, min_priority, max_priority, transparency */
		cischeat_draw_road(bitmap,cliprect,0,i,i,(i != 7));
		cischeat_draw_road(bitmap,cliprect,1,i,i,true);
		bigrun_draw_sprites(bitmap,cliprect,i+1,i);
	}

	m_tmap[0]->draw(screen, bitmap, cliprect, 0, 0 );
	m_tmap[1]->draw(screen, bitmap, cliprect, 0, 0 );

	for (i = 3; i >= 0; i--)
	{
		/* bitmap, cliprect, road, min_priority, max_priority, transparency */
		cischeat_draw_road(bitmap,cliprect,0,i,i,true);
		cischeat_draw_road(bitmap,cliprect,1,i,i,true);
		bigrun_draw_sprites(bitmap,cliprect,i+1,i);
	}

	m_tmap[2]->draw(screen, bitmap, cliprect, 0, 0 );

	return 0;
}


/**************************************************************************
                                Cisco Heat
**************************************************************************/

uint32_t cischeat_state::screen_update_cischeat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int active_layers1, flag;

#ifdef MAME_DEBUG
	/* FAKE Videoreg */
	active_layers1 = m_active_layers;
	if (active_layers1 == 0)   active_layers1 = 0x3f;
#else
	active_layers1 = 0x3f;
#endif

#ifdef MAME_DEBUG
	CISCHEAT_LAYERSCTRL
#endif

	bitmap.fill(0, cliprect);

										/* bitmap, road, priority, transparency */
	if (active_layers1 & 0x10) cischeat_draw_road(bitmap,cliprect,0,7,5,false);
	if (active_layers1 & 0x20) cischeat_draw_road(bitmap,cliprect,1,7,5,true);

	flag = 0;
	cischeat_tmap_DRAW(0)
//  else bitmap.fill(0, cliprect);
	cischeat_tmap_DRAW(1)

	if (active_layers1 & 0x08) cischeat_draw_sprites(bitmap,cliprect,15,3);
	if (active_layers1 & 0x10) cischeat_draw_road(bitmap,cliprect,0,4,1,true);
	if (active_layers1 & 0x20) cischeat_draw_road(bitmap,cliprect,1,4,1,true);
	if (active_layers1 & 0x08) cischeat_draw_sprites(bitmap,cliprect,2,2);
	if (active_layers1 & 0x10) cischeat_draw_road(bitmap,cliprect,0,0,0,true);
	if (active_layers1 & 0x20) cischeat_draw_road(bitmap,cliprect,1,0,0,true);
	if (active_layers1 & 0x08) cischeat_draw_sprites(bitmap,cliprect,1,0);
	cischeat_tmap_DRAW(2)

	/* for the map screen */
	if (active_layers1 & 0x08) cischeat_draw_sprites(bitmap,cliprect,0+16,0+16);

	return 0;
}



/**************************************************************************
                            F1 GrandPrix Star
**************************************************************************/

uint32_t cischeat_state::screen_update_f1gpstar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int active_layers1, flag;

#ifdef MAME_DEBUG
	/* FAKE Videoreg */
	active_layers1 = m_active_layers;
	if (active_layers1 == 0)   active_layers1 = 0x3f;
#else
	active_layers1 = 0x3f;
#endif

#ifdef MAME_DEBUG
	CISCHEAT_LAYERSCTRL
#endif

	bitmap.fill(0, cliprect);

/*  1: clouds 5, grad 7, road 0     2: clouds 5, grad 7, road 0, tunnel roof 0 */

	/* road 1!! 0!! */                  /* bitmap, road, min_priority, max_priority, transparency */
	if (active_layers1 & 0x20) f1gpstar_draw_road(bitmap,cliprect,1,6,7,false);
	if (active_layers1 & 0x10) f1gpstar_draw_road(bitmap,cliprect,0,6,7,true);

	flag = 0;
	cischeat_tmap_DRAW(0)
//  else bitmap.fill(0, cliprect);
	cischeat_tmap_DRAW(1)

	/* road 1!! 0!! */                  /* bitmap, road, min_priority, max_priority, transparency */
	if (active_layers1 & 0x20) f1gpstar_draw_road(bitmap,cliprect,1,1,5,true);
	if (active_layers1 & 0x10) f1gpstar_draw_road(bitmap,cliprect,0,1,5,true);

	if (active_layers1 & 0x08) cischeat_draw_sprites(bitmap,cliprect,15,2);

	/* road 1!! 0!! */                  /* bitmap, road, min_priority, max_priority, transparency */
	if (active_layers1 & 0x20) f1gpstar_draw_road(bitmap,cliprect,1,0,0,true);
	if (active_layers1 & 0x10) f1gpstar_draw_road(bitmap,cliprect,0,0,0,true);

	if (active_layers1 & 0x08) cischeat_draw_sprites(bitmap,cliprect,1,1);
	cischeat_tmap_DRAW(2)
	if (active_layers1 & 0x08) cischeat_draw_sprites(bitmap,cliprect,0,0);

	return 0;
}



/**************************************************************************
                                Scud Hammer
**************************************************************************/

uint32_t cischeat_state::screen_update_scudhamm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int active_layers1 = 0x0d;

#ifdef MAME_DEBUG
m_debugsprites = 0;
if ( machine().input().code_pressed(KEYCODE_Z) || machine().input().code_pressed(KEYCODE_X) )
{
#if 1
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		popmessage("Cmd: %04X Pos:%04X Lim:%04X Inp:%04X",
							m_scudhamm_motor_command,
							scudhamm_motor_pos_r(space,0,0xffff),
							scudhamm_motor_status_r(space,0,0xffff),
							scudhamm_analog_r(space,0,0xffff) );

#if 0
	// captflag
	{
		popmessage( "[%04x] [%04x]\nLEDS %04x", m_captflag_motor_command[LEFT], m_captflag_motor_command[RIGHT], m_captflag_leds );
		m_captflag_motor_command[LEFT] = m_captflag_motor_command[RIGHT] = 0;
	}
#endif

	}
#endif

}
#endif

	bitmap.fill(0, cliprect);

	if (active_layers1 & 0x01) m_tmap[0]->draw(screen, bitmap, cliprect, 0, 0);
	// no layer 1
	if (active_layers1 & 0x08) cischeat_draw_sprites(bitmap,cliprect,0,15);
	if (active_layers1 & 0x04) m_tmap[2]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
