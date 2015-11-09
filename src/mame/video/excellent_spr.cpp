// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    Excellent Systems Sprite chip

  is this original hw or a clone of something?

  possible chips:
  ES 9207 & ES 9303 are near the sprite ROM (aquarium)
  ES 9208 is near the tilemap ROM so probably not the sprite chip


  todo: collapse to single draw function instead of one for each game

*/


#include "emu.h"
#include "excellent_spr.h"


const device_type EXCELLENT_SPRITE = &device_creator<excellent_spr_device>;

excellent_spr_device::excellent_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EXCELLENT_SPRITE, "Excellent 8-bit Sprite", tag, owner, clock, "excellent_spr", __FILE__),
		device_video_interface(mconfig, *this)
{
}


void excellent_spr_device::device_start()
{
	m_ram = auto_alloc_array_clear(this->machine(), UINT8, 0x1000);
	save_pointer(NAME(m_ram), 0x1000);
}


READ8_MEMBER(excellent_spr_device::read)
{
	return m_ram[offset];
}

WRITE8_MEMBER(excellent_spr_device::write)
{
	m_ram[offset] = data;
}

void excellent_spr_device::device_reset()
{
}


/****************************************************************
                     SPRITE DRAW ROUTINE
                     (8-bit)

    Word |     Bit(s)      | Use
    -----+-----------------+-----------------
      0  | xxxxxxxx| X lo
      1  | xxxxxxxx| X hi
      2  | xxxxxxxx| Y lo
      3  | xxxxxxxx| Y hi
      4  | x.......| Disable
      4  | ...x....| Flip Y
      4  | ....x...| 1 = Y chain, 0 = X chain
      4  | .....xxx| Chain size
      5  | ??xxxxxx| Tile (low)
      6  | xxxxxxxx| Tile (high)
      7  | ....xxxx| Color Bank

****************************************************************/


void excellent_spr_device::aquarium_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int y_offs )
{
	int offs, chain_pos;
	int x, y, curx, cury;
	UINT8 col, flipx, flipy, chain;
	UINT16 code;

	for (offs = 0; offs < 0x1000; offs += 8)
	{
		code = ((m_ram[offs + 5]) & 0xff) + (((m_ram[offs + 6]) & 0xff) << 8);
		code &= 0x3fff;

		if (!(m_ram[offs + 4] &0x80))  /* active sprite ? */
		{
			x = ((m_ram[offs + 0]) &0xff) + (((m_ram[offs + 1]) & 0xff) << 8);
			y = ((m_ram[offs + 2]) &0xff) + (((m_ram[offs + 3]) & 0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			col = ((m_ram[offs + 7]) & 0x0f);
			chain = (m_ram[offs + 4]) & 0x07;
			flipy = (m_ram[offs + 4]) & 0x10;
			flipx = (m_ram[offs + 4]) & 0x20;

			curx = x;
			cury = y;

			if (((m_ram[offs + 4]) & 0x08) && flipy)
				cury += (chain * 16);

			if (!(((m_ram[offs + 4]) & 0x08)) && flipx)
				curx += (chain * 16);


			for (chain_pos = chain; chain_pos >= 0; chain_pos--)
			{
				gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code,
						col,
						flipx, flipy,
						curx,cury,0);

				/* wrap around y */
				gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code,
						col,
						flipx, flipy,
						curx,cury + 256,0);

				code++;

				if ((m_ram[offs + 4]) &0x08)   /* Y chain */
				{
					if (flipy)
						cury -= 16;
					else
						cury += 16;
				}
				else    /* X chain */
				{
					if (flipx)
						curx -= 16;
					else
						curx += 16;
				}
			}
		}
	}

#if 0
	if (rotate)
	{
		char buf[80];
		sprintf(buf, "sprite rotate offs %04x ?", rotate);
		popmessage(buf);
	}
#endif
}

void excellent_spr_device::gcpinbal_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int y_offs, int priority )
{
	UINT8 *spriteram = m_ram;
	int offs, chain_pos;
	int x, y, curx, cury;
//  int priority = 0;
	UINT8 col, flipx, flipy, chain;
	UINT16 code;


	for (offs = 0x1000 - 8; offs >= 0; offs -= 8)
	{
		code = ((spriteram[offs + 5]) & 0xff) + (((spriteram[offs + 6]) & 0xff) << 8);
		code &= 0x3fff;

		if (!(spriteram[offs + 4] &0x80))   /* active sprite ? */
		{
			x = ((spriteram[offs + 0]) & 0xff) + (((spriteram[offs + 1]) & 0xff) << 8);
			y = ((spriteram[offs + 2]) & 0xff) + (((spriteram[offs + 3]) & 0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			col  = ((spriteram[offs + 7]) & 0x0f) | 0x60;
			chain = (spriteram[offs + 4]) & 0x07;
			flipy = (spriteram[offs + 4]) & 0x10;
			flipx = 0;

			curx = x;
			cury = y;

			if (((spriteram[offs + 4]) & 0x08) && flipy)
				cury += (chain * 16);

			for (chain_pos = chain; chain_pos >= 0; chain_pos--)
			{
				gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
						code,
						col,
						flipx, flipy,
						curx,cury,
						screen.priority(),
						priority ? 0xfc : 0xf0,0);

				code++;

				if ((spriteram[offs + 4]) & 0x08)   /* Y chain */
				{
					if (flipy)  cury -= 16;
					else cury += 16;
				}
				else    /* X chain */
				{
					curx += 16;
				}
			}
		}
	}
#if 0
	if (rotate)
	{
		char buf[80];
		sprintf(buf, "sprite rotate offs %04x ?", rotate);
		popmessage(buf);
	}
#endif
}
