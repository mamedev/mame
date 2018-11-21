// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Mike Coates, Nicola Salmoria, Miguel Angel Horna, Luca Elia, David Haywood

#include "emu.h"
#include "gaelco_wrally_sprites.h"

DEFINE_DEVICE_TYPE(GAELCO_WRALLY_SPRITES, gaelco_wrally_sprites_device, "gaelco_wrally_sprites", "Gaelco World Rally Sprites")
DEFINE_DEVICE_TYPE(BLMBYCAR_SPRITES, blmbycar_sprites_device, "blmbycar_sprites", "Blomby Car Sprites")

gaelco_wrally_sprites_device::gaelco_wrally_sprites_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
{
}

gaelco_wrally_sprites_device::gaelco_wrally_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gaelco_wrally_sprites_device(mconfig, GAELCO_WRALLY_SPRITES, tag, owner, clock)
{
}

void gaelco_wrally_sprites_device::device_start()
{
	m_screen->register_screen_bitmap(m_temp_bitmap_sprites);
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
	high_priority = number >= 0x3700; // HACK! this is almost certainly not how the priority is determined
	end = 0;
}

void gaelco_wrally_sprites_device::draw_sprites(const rectangle &cliprect, uint16_t* spriteram, int flip_screen)
{
	m_temp_bitmap_sprites.fill(0xffff, cliprect);

	int i;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (i = 6 / 2; i < (0x1000 - 6) / 2; i += 4)
	{
		int sx, sy, number, color, attr, end, color_effect, high_priority;

		get_sprites_info(spriteram + i, sx, sy, number, color, color_effect, attr, high_priority, end);

		if (end)
			break;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		color = color & 0x0f;

		if (flip_screen) {
			sy = sy + 248;
		}

		// store these bits with the written data so that we can use them in mixing later
		color |= (high_priority << 4);
		color |= (color_effect << 5);

		gfx->transpen(m_temp_bitmap_sprites, cliprect, number,
			color, xflip, yflip,
			sx - 0x0f, sy, 0);  // wrally adjusts sx by 0x0f, blmbycar implementation was 0x10
	}
}

void gaelco_wrally_sprites_device::mix_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	for (int y = cliprect.min_y; y < cliprect.max_y; y++)
	{
		const uint16_t* spriteptr = &m_temp_bitmap_sprites.pix16(y);
		uint16_t* dstptr = &bitmap.pix16(y);

		for (int x = cliprect.min_x; x < cliprect.max_x; x++)
		{
			if (spriteptr[x] != 0xffff)
			{
				const int pridat = (spriteptr[x] & 0x100) >> 8;

				if (pridat == priority)
				{
					const int shadow = (spriteptr[x] & 0x200) >> 9;
					
					if (!shadow)
					{
						const uint16_t pendat = (spriteptr[x] & 0xff);
						dstptr[x] = pendat + 0x200;
					}
					else
					{
						const uint16_t pendat = (spriteptr[x] & 0xff);

						if ((pendat & 0xf) >= 8)
						{
							uint16_t srcdat = (dstptr[x] & 0x3ff);	
							dstptr[x] = srcdat + ((pendat & 0xf)-8) * 0x400;
						}
					}
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
	sx = spriteram[3] & 0x03ff;
	sy = (spriteram[0] & 0x01ff);
	sy   = 0xf0 - ((sy & 0xff)  - (sy & 0x100)); // check if this logic works for wrally

	number = spriteram[1] & 0x3fff;
	color = (spriteram[2] & 0x000f) >> 0; // note moved
	color_effect = (spriteram[3] & 0x4000) >> 14;

	attr = (spriteram[2] & 0xfe00) >> 9;
	end = (spriteram[0] & 0x8000); // does wrally have this too?

	high_priority = (~(color >> 3))&1;
}
