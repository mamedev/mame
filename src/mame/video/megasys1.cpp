// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Jaleco Mega System 1 =-

                    driver by   Luca Elia (l.elia@tin.it)



**********  There are 3 scrolling layers, 1 word per tile:

* Note: MS1-Z has 2 layers only.

  A page is 256x256, approximately the visible screen size. Each layer is
  made up of 8 pages (8x8 tiles) or 32 pages (16x16 tiles). The number of
  horizontal  pages and the tiles size  is selectable, using the  layer's
  control register. I think that when tiles are 16x16 a layer can be made
  of 16x2, 8x4, 4x8 or 2x16 pages (see below). When tile size is 8x8 we
  have two examples to guide the choice:

  the copyright screen of p47j (0x12) should be 4x2 (unless it's been hacked :)
  the ending sequence of 64th street (0x13) should be 2x4.

  I don't see a relation.


MS1-A MS1-B MS1-C
-----------------

                    Scrolling layers:

90000 50000 e0000   Scroll 0
94000 54000 e8000   Scroll 1
98000 58000 f0000   Scroll 2                    * Note: missing on MS1-Z

Tile format:    fedc------------    Palette
                ----ba9876543210    Tile Number



84000 44000 c2208   Layers Enable               * Note: missing on MS1-Z?

    fedc ---- ---- ---- unused
    ---- ba98 ---- ---- Priority Code
    ---- ---- 7654 ---- unused
    ---- ---- ---- 3--- Enable Sprites
    ---- ---- ---- -210 Enable Layer 210

    (Note that the bottom layer can't be disabled)


84200 44200 c2000   Scroll 0 Control
84208 44208 c2008   Scroll 1 Control
84008 44008 c2100   Scroll 2 Control        * Note: missing on MS1-Z

Offset:     00                      Scroll X
            02                      Scroll Y
            04 fedc ba98 765- ----  ? (unused?)
               ---- ---- ---4 ----  0<->16x16 Tiles 1<->8x8 Tiles
               ---- ---- ---- 32--  ? (used, by p47!)
               ---- ---- ---- --10  N: Layer H pages = 16 / (2^N)



84300 44300 c2308   Screen Control

    fed- ---- ---- ----     ? (unused?)
    ---c ---- ---- ----     ? (on, troughout peekaboo)
    ---- ba9- ---- ----     ? (unused?)
    ---- ---8 ---- ----     Portrait F/F (?FullFill?)
    ---- ---- 765- ----     ? (unused?)
    ---- ---- ---4 ----     Reset Sound CPU (1->0 Transition)
    ---- ---- ---- 321-     ? (unused?)
    ---- ---- ---- ---0     Flip Screen



**********  There are 256*4 colors (256*3 for MS1-Z):

Colors      MS1-A/C         MS1-Z

000-0ff     Scroll 0        Scroll 0
100-1ff     Scroll 1        Sprites
200-2ff     Scroll 2        Scroll 1
300-3ff     Sprites         -

88000 48000 f8000   Palette

    fedc--------3---    Red
    ----ba98-----2--    Blue
    --------7654--1-    Green
    ---------------0    ? (used, not RGB! [not changed in fades])


**********  There are 256 sprites (128 for MS1-Z):

&RAM[8000]  Sprite Data (16 bytes/entry. 128? entries)

Offset:     0-6                     ? (used, but as normal RAM, I think)
            08  fed- ---- ---- ---- ?
                ---c ---- ---- ---- mosaic sol. (?)
                ---- ba98 ---- ---- mosaic      (?)
                ---- ---- 7--- ---- y flip
                ---- ---- -6-- ---- x flip
                ---- ---- --45 ---- ?
                ---- ---- ---- 3210 color code (* bit 3 = priority *)
            0A                      H position
            0C                      V position
            0E  fedc ---- ---- ---- ? (used by p47j, 0-8!)
                ---- ba98 7654 3210 Number



Object RAM tells the hw how to use Sprite Data (missing on MS1-Z).
This makes it possible to group multiple small sprite, up to 256,
into one big virtual sprite (up to a whole screen):

8e000 4e000 d2000   Object RAM (8 bytes/entry. 256*4 entries)

Offset:     00  Index into Sprite Data RAM
            02  H   Displacement
            04  V   Displacement
            06  Number  Displacement

Only one of these four 256 entries is used to see if the sprite is to be
displayed, according to this latter's flipx/y state:

Object RAM entries:     Used by sprites with:

000-0ff                 No Flip
100-1ff                 Flip X
200-2ff                 Flip Y
300-3ff                 Flip X & Y




No? No? c2108   Sprite Bank

    fedc ba98 7654 321- ? (unused?)
    ---- ---- ---- ---0 Sprite Bank



84100 44100 c2200 Sprite Control ( m_sprite_flag )

            fedc ba9- ---- ---- ? (unused?)
            ---- ---8 ---- ---- Enable Sprite Splitting In 2 Groups:
                                Some Sprite Appear Over, Some Below The Layers
            ---- ---- 765- ---- ? (unused?)
            ---- ---- ---4 ---- Enable Effect (don't clear sprite framebuffer)
            ---- ---- ---- 3210 Effect Number (?)

I think bit 4 enables some sort of color cycling for sprites having priority
bit set. See code of p7j at 6488,  affecting the rotating sprites before the
Jaleco logo is shown: values 11-1f, then 0. I fear the Effect Number is an
offset to be applied over the pens used by those sprites. As for bit 8, it's
not used during game, but it is turned on when sprite/foreground priority is
tested, along with Effect Number being 1, so ...


**********  Priorities (ouch!)

[ Sprite / Sprite order ]

    [MS1-A,B,C]     From first in Object RAM (frontmost) to last.
    [MS1-Z]         From last in Sprite RAM (frontmost) to first.

[ Layer / Layer & Sprite / Layer order ]

Controlled by:

    * bits 7-4 (16 values) of the Layer Control register
    * bit 4 of the Sprite Control register

        Layer Control   Sprite Control
MS1-Z   -
MS1-A   84000           84100
MS1-B   44000           44100
MS1-C   c2208           c2200

When bit 4 of the Sprite Contol register is set, sprites with color
code 0-7 and sprites with color 8-f form two groups. Each group can
appear over or below some layers.

The 16 values in the Layer Control register determine the order of
the layers, and of the groups of sprites.

There is a PROM that translates the values in the register to the
actual code sent to the hardware.


***************************************************************************/

#include "emu.h"
#include "includes/megasys1.h"



VIDEO_START_MEMBER(megasys1_state,megasys1)
{
	m_spriteram = &m_ram[0x8000/2];

	m_buffer_objectram = std::make_unique<u16[]>(0x2000);
	m_buffer_spriteram16 = std::make_unique<u16[]>(0x2000);
	m_buffer2_objectram = std::make_unique<u16[]>(0x2000);
	m_buffer2_spriteram16 = std::make_unique<u16[]>(0x2000);

	m_active_layers = m_sprite_bank = m_screen_flag = m_sprite_flag = 0;

	m_hardware_type_z = 0;

	m_screen->register_screen_bitmap(m_sprite_buffer_bitmap);

	save_pointer(NAME(m_buffer_objectram), 0x2000);
	save_pointer(NAME(m_buffer_spriteram16), 0x2000);
	save_pointer(NAME(m_buffer2_objectram), 0x2000);
	save_pointer(NAME(m_buffer2_spriteram16), 0x2000);
	save_item(NAME(m_screen_flag));
	save_item(NAME(m_active_layers));
	save_item(NAME(m_sprite_flag));
}

VIDEO_START_MEMBER(megasys1_state,megasys1_z)
{
	VIDEO_START_CALL_MEMBER(megasys1);
	m_hardware_type_z = 1;
}

/***************************************************************************

                            Video registers access

***************************************************************************/


WRITE16_MEMBER(megasys1_state::active_layers_w)
{
	COMBINE_DATA(&m_active_layers);
	m_screen->update_partial(m_screen->vpos());
}

WRITE16_MEMBER(megasys1_state::sprite_bank_w)
{
	COMBINE_DATA(&m_sprite_bank);
}

READ16_MEMBER(megasys1_state::sprite_flag_r)
{
	return m_sprite_flag;
}

WRITE16_MEMBER(megasys1_state::sprite_flag_w)
{
	COMBINE_DATA(&m_sprite_flag);
}

WRITE16_MEMBER(megasys1_state::screen_flag_w)
{
	COMBINE_DATA(&m_screen_flag);

	if (m_audiocpu.found())
	{
		if (m_screen_flag & 0x10)
			m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		else
			m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
}

WRITE16_MEMBER(megasys1_state::soundlatch_w)
{
	m_soundlatch[0]->write(data);
	m_audiocpu->set_input_line(4, HOLD_LINE);
}

WRITE16_MEMBER(megasys1_state::soundlatch_z_w)
{
	m_soundlatch[0]->write(data & 0xff);
	m_audiocpu->set_input_line(5, HOLD_LINE);
}

WRITE16_MEMBER(megasys1_state::soundlatch_c_w)
{
	// Cybattler reads sound latch on irq 2
	m_soundlatch[0]->write(data);
	m_audiocpu->set_input_line(2, HOLD_LINE);
}

WRITE16_MEMBER(megasys1_state::monkelf_scroll0_w)
{
	// code in routine $280 does this. protection?
	if (offset == 0)
		data = data - (((data & 0x0f) > 0x0d) ? 0x10 : 0);
	m_tmap[0]->scroll_w(space, offset, data, mem_mask);
}

WRITE16_MEMBER(megasys1_state::monkelf_scroll1_w)
{
	// code in routine $280 does this. protection?
	if (offset == 0)
		data = data - (((data & 0x0f) > 0x0b) ? 0x10 : 0);
	m_tmap[1]->scroll_w(space, offset, data, mem_mask);
}


/***************************************************************************

                            Sprites Drawing

***************************************************************************/


/*   Draw sprites in the given bitmap.

 Sprite Data:

    Offset      Data

    00-07                       ?
    08      fed- ---- ---- ---- ?
            ---c ---- ---- ---- mosaic sol.
            ---- ba98 ---- ---- mosaic
            ---- ---- 7--- ---- y flip
            ---- ---- -6-- ---- x flip
            ---- ---- --45 ---- ?
            ---- ---- ---- 3210 color code (bit 3 = priority)
    0A      X position
    0C      Y position
    0E      Code                                            */


void megasys1_state::mix_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *decodegfx = m_gfxdecode->gfx(0);
	u16 colorbase = decodegfx->colorbase();

	for (int y = cliprect.min_y;y <= cliprect.max_y;y++)
	{
		u16* srcline = &m_sprite_buffer_bitmap.pix16(y);
		u16* dstline = &bitmap.pix16(y);
		u8 *prio = &screen.priority().pix8(y);

		for (int x = cliprect.min_x;x <= cliprect.max_x;x++)
		{
			u16 pixel = srcline[x];

			if ((pixel & 0xf) != 0xf)
			{
				int priority = (pixel & 0x4000) >> 14;
				priority = (priority) ? 0x0c : 0x0a;

				if ((priority & (1 << (prio[x] & 0x1f))) == 0)
				{
					u8 coldat = pixel & 0x3fff;
					dstline[x] = coldat + colorbase;

				}
			}
		}
	}
}

void megasys1_state::partial_clear_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 param)
{
	for (int y = cliprect.min_y;y <= cliprect.max_y;y++)
	{
		u16* srcline = &m_sprite_buffer_bitmap.pix16(y);

		for (int x = cliprect.min_x;x <= cliprect.max_x;x++)
		{
			u16 pixel = srcline[x];
			srcline[x] = pixel & 0x7fff; // wipe our 'drawn here' marker otherwise trails will always have priority over new sprites, which is incorrect.

			// guess, very unclear from the video refernece we have, used when removing p47 trails
			if (((pixel & 0xf0) >> 4) < param)
				srcline[x] = 0x7fff;
		}
	}
}


inline void megasys1_state::draw_16x16_priority_sprite(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect, s32 code, s32 color, s32 sx, s32 sy, s32 flipx, s32 flipy, u8 mosaic, u8 mosaicsol, s32 priority)
{
//  if (sy >= nScreenHeight || sy < -15 || sx >= nScreenWidth || sx < -15) return;
	gfx_element *decodegfx = m_gfxdecode->gfx(0);
	sy = sy + 16;

	const u8* gfx = decodegfx->get_data(code);

	flipy = (flipy) ? 0x0f : 0;
	flipx = (flipx) ? 0x0f : 0;

	color = color * 16;


	for (s32 y = 0; y < 16; y++, sy++, sx-=16)
	{
	//  u16 *dest = &bitmap.pix16(sy)+ sx;
	//  u8 *prio = &screen.priority().pix8(sy) + sx;
		u16* dest = &m_sprite_buffer_bitmap.pix16(sy)+ sx;

		for (s32 x = 0; x < 16; x++, sx++)
		{
			if (sx < cliprect.min_x || sy < cliprect.min_y || sx > cliprect.max_x || sy > cliprect.max_y) continue;

			s32 pxl;

			if (mosaicsol)
				pxl = gfx[(((y ^ flipy) |  mosaic) * 16) + ((x ^ flipx) |  mosaic)];
			else
				pxl = gfx[(((y ^ flipy) & ~mosaic) * 16) + ((x ^ flipx) & ~mosaic)];

			if (pxl != 0x0f)
			{
				if (!(dest[x] & 0x8000))
				{
					dest[x] = (pxl+color) | (priority << 14);
					dest[x] |= 0x8000;
				}
			}
		}
	}

}

void megasys1_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int color,code,sx,sy,flipx,flipy,attr,sprite;



/* objram: 0x100*4 entries      spritedata: 0x80 entries */

	/* sprite order is from first in Sprite Data RAM (frontmost) to last */

	if (m_hardware_type_z == 0)  /* standard sprite hardware */
	{
		if (!(m_sprite_flag&0x10))
			m_sprite_buffer_bitmap.fill(0x7fff, cliprect);
		else
		{
			// P47 sprite trails effect.. not quite right tho
			// I think the low 4 bits are used to clear specific pens?
			// when the hardware wants to clear the trails from the screen
			// it increases the value from 0x00 to 0x0e
			//printf("m_sprite_flag %02x\n", m_sprite_flag);
			partial_clear_sprite_bitmap(screen, bitmap, cliprect, m_sprite_flag&0x0f);
		}

		s32 color_mask = (m_sprite_flag & 0x100) ? 0x07 : 0x0f;

		u16 *objectram = (u16*)m_buffer2_objectram.get();
		u16 *spriteram = (u16*)m_buffer2_spriteram16.get();

		for (s32 offs = (0x800-8)/2; offs >= 0; offs -= 4)
		{
			for (s32 sprite = 0; sprite < 4 ; sprite ++)
			{
				u16 *objectdata = &objectram[offs + (0x800/2) * sprite];
				u16 *spritedata = &spriteram[(objectdata[0] & 0x7f) * 8];

				s32 attr = spritedata[4];
				if (((attr & 0xc0) >> 6) != sprite) continue;

				s32 sx = (spritedata[5] + objectdata[1]) & 0x1ff;
				s32 sy = (spritedata[6] + objectdata[2]) & 0x1ff;

				if (sx > 255) sx -= 512;
				if (sy > 255) sy -= 512;

				s32 code  = spritedata[7] + objectdata[3];
				s32 color = attr & color_mask;

				s32 flipx = attr & 0x40;
				s32 flipy = attr & 0x80;
				//s32 pri  = (attr & 0x08) ? 0x0c : 0x0a;
				s32 pri  = (attr & 0x08)>>3;
				s32 mosaic = (attr & 0x0f00)>>8;
				s32 mossol = (attr & 0x1000)>>8;

				code = (code & 0xfff) + ((m_sprite_bank & 1) << 12);

				if (m_screen_flag & 1)
				{
					flipx = !flipx;
					flipy = !flipy;
					sx = 240 - sx;
					sy = 240 - sy;
				}

				draw_16x16_priority_sprite(screen,bitmap,cliprect,code, color, sx, sy - 16, flipx, flipy, mosaic, mossol, pri);
			}
		}
	}   /* non Z hw */
	else
	{
		u16 *spriteram16 = m_spriteram;

		/* MS1-Z just draws Sprite Data, and in reverse order */

		for (sprite = 0x80-1;sprite >= 0;sprite--)
		{
			u16 *spritedata = &spriteram16[ sprite * 0x10/2];

			attr = spritedata[ 8/2 ];

			sx = spritedata[0x0A/2] % 512;
			sy = spritedata[0x0C/2] % 512;

			if (sx > 256-1) sx -= 512;
			if (sy > 256-1) sy -= 512;

			code  = spritedata[0x0E/2];
			color = (attr & 0x0F);

			flipx = attr & 0x40;
			flipy = attr & 0x80;

			if (m_screen_flag & 1)
			{
				flipx = !flipx;     flipy = !flipy;
				sx = 240-sx;        sy = 240-sy;
			}

			m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
					code,
					color,
					flipx, flipy,
					sx, sy,
					screen.priority(),
					(attr & 0x08) ? 0x0c : 0x0a,15);
		}   /* sprite */
	}   /* Z hw */

}




/***************************************************************************
                        Convert the Priority Prom
***************************************************************************/

struct priority
{
	const char *driver;
	int priorities[16];
};


/*
    Layers order encoded as an int like: 0x01234, where

    0:  Scroll 0
    1:  Scroll 1
    2:  Scroll 2
    3:  Sprites with color 0-7
        (*every sprite*, if sprite splitting is not active)
    4:  Sprites with color 8-f
    f:  empty placeholder (we can't use 0!)

    and the bottom layer is on the left (e.g. 0).

    The special value 0xfffff means that the order is either unknown
    or no simple stack of layers can account for the values in the prom.
    (the default value, 0x04132, will be used in those cases)

*/



/*
    Convert the 512 bytes in the Priority Prom into 16 ints, encoding
    the layers order for 16 selectable priority schemes.

    INPUT (to the video chip):

        4 pixels: 3 layers(012) + 1 sprite (3)
        (there are low and high priority sprites which
        are split when the "split sprites" bit is set)

    addr =  ( (low pri sprite & split sprites ) << 0 ) +
            ( (pixel 0 is enabled and opaque )  << 1 ) +
            ( (pixel 1 is enabled and opaque )  << 2 ) +
            ( (pixel 2 is enabled and opaque )  << 3 ) +
            ( (pixel 3 is enabled and opaque )  << 4 ) +
            ( (layers_enable bits 11-8  )       << 5 )

    OUTPUT (to video):
        1 pixel, the one from layer: PROM[addr] (layer can be 0-3)

    This scheme can generate a wealth of funky priority schemes
    while we can account for just a simple stack of transparent
    layers like: 01324. That is: bottom layer is 0, then 1, then
    sprites (low priority sprites if sprite splitting is active,
    every sprite if not) then layer 2 and high priority sprites
    (only if sprite splitting is active).

    Hence, during the conversion process we make sure each of the
    16 priority scheme in the prom is a "simple" one like the above
    and log a warning otherwise. The feasibility criterion is such:

    the opaque pens of the top layer must be above any other layer.
    The transparent pens of the top layer must be either totally
    opaque or totally transparent with respects to the other layers:
    when the bit relative to the top layer is not set, the top layer's
    code must be either always absent (transparent case) or always
    present (opaque case) in the prom.

    NOTE: This can't account for orders starting like: 030..
    as found in peekaboo's prom. That's where sprites go below
    the bottom layer's opaque pens, but above its transparent
    pens.
*/
void megasys1_state::priority_create()
{
	const u8 *color_prom = memregion("proms")->base();
	int pri_code, offset, i, order;

	/* convert PROM to something we can use */

	for (pri_code = 0; pri_code < 0x10 ; pri_code++)    // 16 priority codes
	{
		int layers_order[2];    // 2 layers orders (split sprites on/off)

		for (offset = 0; offset < 2; offset ++)
		{
			int enable_mask = 0xf;  // start with every layer enabled

			layers_order[offset] = 0xfffff;

			do
			{
				int top = color_prom[pri_code * 0x20 + offset + enable_mask * 2] & 3;   // this must be the top layer
				int top_mask = 1 << top;

				int result = 0;     // result of the feasibility check for this layer

				for (i = 0; i < 0x10 ; i++) // every combination of opaque and transparent pens
				{
					int opacity =   i & enable_mask;    // only consider active layers
					int layer   =   color_prom[pri_code * 0x20 + offset + opacity * 2];

					if (opacity)
					{
						if (opacity & top_mask)
						{
							if (layer != top )  result |= 1;    // error: opaque pens aren't always opaque!
						}
						else
						{
							if (layer == top)   result |= 2;    // transparent pen is opaque
							else                result |= 4;    // transparent pen is transparent
						}
					}
				}

				/*  note: 3210 means that layer 0 is the bottom layer
				    (the order is reversed in the hand-crafted data) */

				layers_order[offset] = ( (layers_order[offset] << 4) | top ) & 0xfffff;
				enable_mask &= ~top_mask;

				if (result & 1)
				{
					logerror("WARNING, pri $%X split %d - layer %d's opaque pens not totally opaque\n",pri_code,offset,top);

					layers_order[offset] = 0xfffff;
					break;
				}

				if  ((result & 6) == 6)
				{
					logerror("WARNING, pri $%X split %d - layer %d's transparent pens aren't always transparent nor always opaque\n",pri_code,offset,top);

					layers_order[offset] = 0xfffff;
					break;
				}

				if (result == 2)    enable_mask = 0; // totally opaque top layer

			}   while (enable_mask);

		}   // offset

		/* merge the two layers orders */

		order = 0xfffff;

		for (i = 5; i > 0 ; )   // 5 layers to write
		{
			int layer;
			int layer0 = layers_order[0] & 0x0f;
			int layer1 = layers_order[1] & 0x0f;

			if (layer0 != 3)    // 0,1,2 or f
			{
				if (layer1 == 3)
				{
					layer = 4;
					layers_order[0] <<= 4;  // layer1 won't change next loop
				}
				else
				{
					layer = layer0;
					if (layer0 != layer1)
					{
						logerror("WARNING, pri $%X - 'sprite splitting' does not simply split sprites\n",pri_code);

						order = 0xfffff;
						break;
					}

				}
			}
			else    // layer0 = 3;
			{
				if (layer1 == 3)
				{
					layer = 0x43;           // 4 must always be present
					order <<= 4;
					i --;                   // 2 layers written at once
				}
				else
				{
					layer = 3;
					layers_order[1] <<= 4;  // layer1 won't change next loop
				}
			}

			/* reverse the order now */
			order = (order << 4 ) | layer;

			i --;       // layer written

			layers_order[0] >>= 4;
			layers_order[1] >>= 4;

		}   // merging

		m_layers_order[pri_code] = order & 0xfffff; // at last!

	}   // pri_code



#if 0
	/* log the priority schemes */
	for (i = 0; i < 16; i++)
		logerror("PROM %X] %05x\n", i, m_layers_order[i]);
#endif


}

void megasys1_state::megasys1_palette(palette_device &palette)
{
	priority_create();
}





/***************************************************************************
              Draw the game screen in the given bitmap_ind16.
***************************************************************************/


u32 megasys1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, flag, pri, primask;
	int active_layers;

	if (m_hardware_type_z)
	{
		/* no layer 2 and fixed layers order? */
		active_layers = 0x000b;
		pri = 0x0314f;
	}
	else
	{
		int reallyactive = 0;

		/* get layers order */
		pri = m_layers_order[(m_active_layers & 0x0f0f) >> 8];

#ifdef MAME_DEBUG
		if (pri == 0xfffff)
		{
			popmessage("Pri: %04X - Flag: %04X", m_active_layers, m_sprite_flag);
		}
#endif

		if (pri == 0xfffff) pri = 0x04132;

		/* see what layers are really active (layers 4 & f will do no harm) */
		for (i = 0;i < 5;i++)
			reallyactive |= 1 << ((pri >> (4 * i)) & 0x0f);

		active_layers = m_active_layers & reallyactive;
		active_layers |= 1 << ((pri & 0xf0000) >> 16);  // bottom layer can't be disabled
	}

	for (i = 0; i < 3; i++)
	{
		if (m_tmap[i].found())
		{
			m_tmap[i]->set_flip((m_screen_flag & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_tmap[i]->enable(active_layers & (1 << i));
		}
	}

	screen.priority().fill(0, cliprect);

	flag = TILEMAP_DRAW_OPAQUE;
	primask = 0;

	for (i = 0;i < 5;i++)
	{
		int layer = (pri & 0xf0000) >> 16;
		pri <<= 4;

		switch (layer)
		{
		case 0:
		case 1:
		case 2:
			if (m_tmap[layer].found() && (active_layers & (1 << layer)))
			{
				m_tmap[layer]->draw(screen, bitmap, cliprect, flag, primask);
				flag = 0;
			}
			break;
		case 3:
		case 4:
			if (flag != 0)
			{
				flag = 0;
				bitmap.fill(0, cliprect);
			}

			if (m_sprite_flag & 0x100)  /* sprites are split */
			{
				/* following tilemaps will obscure this sprites layer */
				primask |= 1 << (layer - 3);
			}
			else
				/* following tilemaps will obscure all sprites */
				if (layer == 3) primask |= 3;

			break;
		}
	}

	if (active_layers & 0x08)
	{
		draw_sprites(screen, bitmap, cliprect);

		if (m_hardware_type_z == 0)
			mix_sprite_bitmap(screen, bitmap, cliprect);

	}

	return 0;
}

WRITE_LINE_MEMBER(megasys1_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		/* Sprite are TWO frames ahead, like NMK16 HW. */
	//m_objectram
		memcpy(m_buffer2_objectram.get(),m_buffer_objectram.get(), 0x2000);
		memcpy(m_buffer_objectram.get(), m_objectram, 0x2000);
	//spriteram16
		memcpy(m_buffer2_spriteram16.get(), m_buffer_spriteram16.get(), 0x2000);
		memcpy(m_buffer_spriteram16.get(), m_spriteram, 0x2000);
	}

}
