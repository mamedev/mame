// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Mike Coates, Nicola Salmoria, Miguel Angel Horna, Luca Elia, David Haywood

#include "emu.h"
#include "gaelco_wrally_sprites.h"

DEFINE_DEVICE_TYPE(GAELCO_WRALLY_SPRITES, gaelco_wrally_sprites_device, "gaelco_wrally_sprites", "Gaelco World Rally Sprites")
DEFINE_DEVICE_TYPE(BLMBYCAR_SPRITES, blmbycar_sprites_device, "blmbycar_sprites", "Blomby Car Sprites")

gaelco_wrally_sprites_device::gaelco_wrally_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GAELCO_WRALLY_SPRITES, tag, owner, clock)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}

gaelco_wrally_sprites_device::gaelco_wrally_sprites_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}


void gaelco_wrally_sprites_device::device_start()
{
}

void gaelco_wrally_sprites_device::device_reset()
{
}


/*
    Sprite Format
    -------------

    Offs | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy

      1  | xxxxxxxx xxxxxxxx | unknown
      
	  2  | ------xx xxxxxxxx | x position
      2  | --xxxx-- -------- | sprite color (low 4 bits)  
	  2  | -x------ -------- | shadows/highlights (see below)
      2  | x------- -------- | not used?
      
	  3  | --xxxxxx xxxxxxxx | sprite code
      3  | xx------ -------- | not used?

    For shadows/highlights, the tile color below the sprite will be set using a
    palette (from the 8 available) based on the gfx pen of the sprite. Only pens
    in the range 0x8-0xf are used.
*/

void gaelco_wrally_sprites_device::get_sprites_info(uint16_t* spriteram, int& sx, int& sy, int& number, int& color, int& color_effect, int& attr, int& high_priority, int &end)
{
	sx = spriteram[2] & 0x03ff;
	sy = (240 - (spriteram[0] & 0x00ff)) & 0x00ff;
	number = spriteram[3] & 0x3fff;
	color = (spriteram[2] & 0x7c00) >> 10;
	color_effect = (color & 0x10) >> 4;
	attr = (spriteram[0] & 0xfe00) >> 9;
	high_priority = number >= 0x3700; // HACK! see below
	end = 0;
}

void gaelco_wrally_sprites_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t* spriteram, int flip_screen, int priority)
{
	int i, px, py;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (i = 6/2; i < (0x1000 - 6)/2; i += 4) {
		
		int sx,sy,number,color,attr, end, color_effect, high_priority;

		get_sprites_info(spriteram+i,sx,sy,number,color,color_effect,attr,high_priority,end);

		if (end)
			break;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		color = color & 0x0f;

		if (high_priority != priority) continue;

		if (flip_screen) {
			sy = sy + 248;
		}

		if (!color_effect) {
			gfx->transpen(bitmap,cliprect,number,
					0x20 + color,xflip,yflip,
					sx - 0x0f,sy,0);
		} else {
			/* get a pointer to the current sprite's gfx data */
			const uint8_t *gfx_src = gfx->get_data(number % gfx->elements());

			for (py = 0; py < gfx->height(); py++){
				/* get a pointer to the current line in the screen bitmap */
				int ypos = ((sy + py) & 0x1ff);
				uint16_t *srcy = &bitmap.pix16(ypos);

				int gfx_py = yflip ? (gfx->height() - 1 - py) : py;

				if ((ypos < cliprect.min_y) || (ypos > cliprect.max_y)) continue;

				for (px = 0; px < gfx->width(); px++){
					/* get current pixel */
					int xpos = (((sx + px) & 0x3ff) - 0x0f) & 0x3ff;
					uint16_t *pixel = srcy + xpos;
					int src_color = *pixel;

					int gfx_px = xflip ? (gfx->width() - 1 - px) : px;

					/* get asociated pen for the current sprite pixel */
					int gfx_pen = gfx_src[gfx->rowbytes()*gfx_py + gfx_px];

					/* pens 8..15 are used to select a palette */
					if ((gfx_pen < 8) || (gfx_pen >= 16)) continue;

					if ((xpos < cliprect.min_x) || (xpos > cliprect.max_x)) continue;

					/* modify the color of the tile */
					*pixel = src_color + (gfx_pen-8)*1024;
				}
			}
		}
	}
}


/***************************************************************************


                                Sprites Drawing

    Offset:     Bits:                   Value:

        0       f--- ---- ---- ----     End Of Sprites
                -edc ba9- ---- ----
                ---- ---8 7654 3210     Y (Signed)

        1                               Code

        2       f--- ---- ---- ----     Flip Y
                -e-- ---- ---- ----     Flip X
                --dc ba98 7654 ----
                ---- ---- ---- 3210     Color (Bit 3 = Priority)

        3       f--- ---- ---- ----     ? Is this ever used ?
                -e-- ---- ---- ----     ? 1 = shadow sprite!
                --dc ba9- ---- ----
                ---- ---8 7654 3210     X (Signed)


***************************************************************************/

blmbycar_sprites_device::blmbycar_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gaelco_wrally_sprites_device(mconfig, BLMBYCAR_SPRITES, tag, owner, clock)
{
}

void blmbycar_sprites_device::get_sprites_info(uint16_t* spriteram, int& sx, int& sy, int& number, int& color, int& color_effect, int& attr, int& high_priority, int &end)
{
	// bytes are swapped around and color attribute is moved
	sx = spriteram[3] & 0x03ff; // wrally adjusts this by 0x0f, blmbycar implementation was 0x10
	sy = (spriteram[0] & 0x01ff);
	sy   = 0xf0 - ((sy & 0xff)  - (sy & 0x100)); // check if this logic works for wrally

	number = spriteram[1] & 0x3fff;
	color = (spriteram[2] & 0x000f) >> 0; // note moved
	color_effect = (spriteram[3] & 0x4000) >> 14;

	attr = (spriteram[2] & 0xfe00) >> 9;
	end = (spriteram[0] & 0x8000); // does wrally have this too?

	high_priority = (~(color >> 3))&1;
}

// this is the original Blomby Car implementation, note the priority bit, seems less hacky than the Gaelco wrally implementation, check if it is the correct for there instead of the hack
// world rally priority implementation does not work well for blomby car
#if 0

void blmbycar_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint16_t *source, *finish;

	source = m_spriteram + 0x6 / 2;              // !
	finish = m_spriteram + m_spriteram.bytes() / 2 - 8 / 2;

	/* Find "the end of sprites" marker */

	for ( ; source < finish; source += 8 / 2 )
		if (source[0] & 0x8000) break;

	/* Draw sprites in reverse order for pdrawfgfx */

	source -= 8 / 2;
	finish = m_spriteram;

	for ( ; source >= finish; source -= 8 / 2 )
	{
		int y       = source[0];
		int code        = source[1];
		int attr        = source[2];
		int x       = source[3];

		int flipx       = attr & 0x4000;
		int flipy       = attr & 0x8000;
		int pri     = (~attr >> 3) & 0x1;       // Priority (1 = Low)
		int pri_mask    = ~((1 << (pri+1)) - 1);    // Above the first "pri" levels

		if (x & 0x4000) continue;   // ? To get rid of the "shadow" blocks

		x   = (x & 0x1ff) - 0x10;
		y   = 0xf0 - ((y & 0xff)  - (y & 0x100));

		m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
					code,
					0x20 + (attr & 0xf),
					flipx, flipy,
					x, y,
					screen.priority(),
					pri_mask,0);
	}
}

#endif