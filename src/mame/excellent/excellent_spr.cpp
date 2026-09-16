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
#include "screen.h"

#include <algorithm>


// 16x16x4
static gfx_layout layout_16x16x4 =
{
	16,16,  // 16*16 sprites
	RGN_FRAC(1,1),
	4,  // 4 bits per pixel
//  { 16, 48, 0, 32 },
	{ 48, 16, 32, 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16*4) },
	16*16*4   // every sprite takes 128 consecutive bytes
};

GFXDECODE_MEMBER(excellent_spr_device::gfxinfo)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, layout_16x16x4, 0, 16)
GFXDECODE_END

DEFINE_DEVICE_TYPE(EXCELLENT_SPRITE, excellent_spr_device, "excellent_spr", "Excellent 8-bit Sprite")

excellent_spr_device::excellent_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, EXCELLENT_SPRITE, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_colpri_cb(*this)
	, m_colbase(0)
{
}


void excellent_spr_device::device_start()
{
	decode_gfx(gfxinfo);
	gfx(0)->set_colorbase(m_colbase);

	m_colpri_cb.resolve();
	m_ram = make_unique_clear<u8[]>(0x1000);
	m_ram_buffered = make_unique_clear<u8[]>(0x1000);

	save_pointer(NAME(m_ram), 0x1000);
	save_pointer(NAME(m_ram_buffered), 0x1000);
}


u8 excellent_spr_device::read(offs_t offset)
{
	return m_ram[offset];
}

void excellent_spr_device::write(offs_t offset, u8 data)
{
	m_ram[offset] = data;
}

void excellent_spr_device::device_reset()
{
}

void excellent_spr_device::vblank(int state)
{
	if (state)
		std::copy_n(&m_ram[0], 0x1000, &m_ram_buffered[0]);
}

/****************************************************************
                     SPRITE DRAW ROUTINE
                     (8-bit)

    Word |  Bit(s)  | Use
    -----+----------+---------------------------
      0  | xxxxxxxx | X lo
      1  | xxxxxxxx | X hi
      2  | xxxxxxxx | Y lo
      3  | xxxxxxxx | Y hi
      4  | x....... | Disable
      4  | ..x..... | Flip X (aquarium)
      4  | ...x.... | Flip Y
      4  | ....x... | 1 = Y chain, 0 = X chain
      4  | .....xxx | Chain size
      5  | ??xxxxxx | Tile (low)
      6  | xxxxxxxx | Tile (high)
      7  | ....xxxx | Color Bank

****************************************************************/


void excellent_spr_device::aquarium_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const bool priority = !m_colpri_cb.isnull();

	int start, end, inc;
	if (priority) { start = 0x1000 - 8; end =     -8; inc = -8; }
	else          { start =          0; end = 0x1000; inc = +8; }

	for (int offs = start; offs != end; offs += inc)
	{
		if (BIT(~m_ram_buffered[offs + 4], 7))  /* active sprite ? */
		{
			u32 code = ((m_ram_buffered[offs + 5]) & 0xff) + (((m_ram_buffered[offs + 6]) & 0xff) << 8);
			code &= 0x3fff;

			int x = ((m_ram_buffered[offs + 0]) & 0xff) + (((m_ram_buffered[offs + 1]) & 0xff) << 8);
			int y = ((m_ram_buffered[offs + 2]) & 0xff) + (((m_ram_buffered[offs + 3]) & 0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			u32 pri_mask = 0;
			u32 colour = ((m_ram_buffered[offs + 7]) & 0x0f);
			const u8 chain = (m_ram_buffered[offs + 4]) & 0x07;
			const bool chaindir = BIT(m_ram_buffered[offs + 4], 3);
			const bool flipy = BIT(m_ram_buffered[offs + 4], 4);
			const bool flipx = BIT(m_ram_buffered[offs + 4], 5);
			if (priority)
				m_colpri_cb(colour, pri_mask);

			int curx = x;
			int cury = y;

			if (chaindir && flipy)
				cury += (chain * 16);

			if ((!chaindir) && flipx)
				curx += (chain * 16);

			for (int chain_pos = chain; chain_pos >= 0; chain_pos--)
			{
				if (priority)
				{
					gfx(0)->prio_transpen(bitmap,cliprect,
							code,
							colour,
							flipx, flipy,
							curx,cury,
							screen.priority(),pri_mask,0);

					/* wrap around y */
					gfx(0)->prio_transpen(bitmap,cliprect,
							code,
							colour,
							flipx, flipy,
							curx,cury + 256,
							screen.priority(),pri_mask,0);
				}
				else
				{
					gfx(0)->transpen(bitmap,cliprect,
							code,
							colour,
							flipx, flipy,
							curx,cury,
							0);

					/* wrap around y */
					gfx(0)->transpen(bitmap,cliprect,
							code,
							colour,
							flipx, flipy,
							curx,cury + 256,
							0);
				}

				code++;

				if (chaindir)   /* Y chain */
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
}

void excellent_spr_device::gcpinbal_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const bool priority = !m_colpri_cb.isnull();

	int start, end, inc;
	if (priority) { start = 0x1000 - 8; end =     -8; inc = -8; }
	else          { start =          0; end = 0x1000; inc = +8; }

	for (int offs = start; offs != end; offs += inc)
	{
		if (BIT(~m_ram_buffered[offs + 4], 7))  /* active sprite ? */
		{
			u32 code = ((m_ram_buffered[offs + 5]) & 0xff) + (((m_ram_buffered[offs + 6]) & 0xff) << 8);
			code &= 0x3fff;

			int x = ((m_ram_buffered[offs + 0]) & 0xff) + (((m_ram_buffered[offs + 1]) & 0xff) << 8);
			int y = ((m_ram_buffered[offs + 2]) & 0xff) + (((m_ram_buffered[offs + 3]) & 0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			u32 pri_mask = 0;
			u32 colour = ((m_ram_buffered[offs + 7]) & 0x0f);
			const u8 chain = (m_ram_buffered[offs + 4]) & 0x07;
			const bool chaindir = BIT(m_ram_buffered[offs + 4], 3);
			const bool flipy = BIT(m_ram_buffered[offs + 4], 4);
			const bool flipx = 0;
			if (priority)
				m_colpri_cb(colour, pri_mask);

			int curx = x;
			int cury = y;

			if (chaindir && flipy)
				cury += (chain * 16);

			for (int chain_pos = chain; chain_pos >= 0; chain_pos--)
			{
				if (priority)
				{
					gfx(0)->prio_transpen(bitmap,cliprect,
							code,
							colour,
							flipx, flipy,
							curx,cury,
							screen.priority(),pri_mask,0);
				}
				else
				{
					gfx(0)->transpen(bitmap,cliprect,
							code,
							colour,
							flipx, flipy,
							curx,cury,
							0);
				}

				code++;

				if (chaindir)   /* Y chain */
				{
					if (flipy)
						cury -= 16;
					else
						cury += 16;
				}
				else    /* X chain */
				{
					curx += 16;
				}
			}
		}
	}
}
