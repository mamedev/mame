// license:BSD-3-Clause
// copyright-holders:Quench
/* Toaplan Sprite Controller 'SCU'
 used by video/twincobr.c (including wardner)
 and rallybik in toaplan1.c
*/


#include "emu.h"
#include "toaplan_scu.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(TOAPLAN_SCU, toaplan_scu_device, "toaplan_scu", "Toaplan SCU")

const gfx_layout toaplan_scu_device::spritelayout =
{
	16,16,          /* 16*16 sprites */
	RGN_FRAC(1,4),
	4,              /* 4 bits per pixel */
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ STEP16(0, 1) },
	{ STEP16(0, 16) },
	16*16
};

GFXDECODE_MEMBER( toaplan_scu_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, spritelayout, 0, 64 )
GFXDECODE_END


toaplan_scu_device::toaplan_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TOAPLAN_SCU, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo)
{
}

void toaplan_scu_device::device_start()
{
}

void toaplan_scu_device::device_reset()
{
}

void toaplan_scu_device::alloc_sprite_bitmap(screen_device &screen)
{
	screen.register_screen_bitmap(m_temp_spritebitmap);
}

/***************************************************************************
    Sprite Handlers
***************************************************************************/

void toaplan_scu_device::draw_sprites_to_tempbitmap(const rectangle &cliprect, u16* spriteram, u32 bytes)
{
	m_temp_spritebitmap.fill(0, cliprect);

	for (int offs = 0; offs < bytes / 2; offs += 4)
	{
		const u16 attribute = spriteram[offs + 1];
		const int priority  = (attribute & 0x0c00) >> 10;

		// are 0 priority really skipped, or can they still mask?
		if (!priority) continue;

		const int sy = spriteram[offs + 3] >> 7;
		if (sy != 0x0100)     /* sx = 0x01a0 or 0x0040*/
		{
			const u32 sprite = spriteram[offs] & 0x7ff;
			u32 color        = attribute & 0x3f;
			color |= priority << 6; // encode colour

			int sx          = spriteram[offs + 2] >> 7;
			const int flipx = attribute & 0x100;
			if (flipx) sx  -= m_xoffs_flipped;

			const int flipy = attribute & 0x200;
			gfx(0)->transpen_raw(m_temp_spritebitmap, cliprect,
				sprite,
				color << 4 /* << 4 because using _raw */ ,
				flipx, flipy,
				sx - m_xoffs, sy - 16, 0);
		}
	}

}


/***************************************************************************
    Draw the game screen in the given bitmap.
***************************************************************************/

void toaplan_scu_device::copy_sprites_from_tempbitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, int priority)
{
	const pen_t *pens = &palette().pen(gfx(0)->colorbase());

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 *srcline = &m_temp_spritebitmap.pix16(y);
		u32 *dstline = &bitmap.pix32(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const u16 pix = srcline[x];

			if ((pix >> (4 + 6)) == priority)
			{
				if (pix & 0xf)
				{
					dstline[x] = pens[pix & 0x3ff];
				}
			}
		}

	}
}
