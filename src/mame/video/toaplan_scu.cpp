// license:BSD-3-Clause
// copyright-holders:Quench
/* Toaplan Sprite Controller 'SCU'
 used by video/twincobr.c (including wardner)
 and rallybik in toaplan1.c
*/



#include "emu.h"
#include "toaplan_scu.h"

const device_type TOAPLAN_SCU = &device_creator<toaplan_scu_device>;

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


toaplan_scu_device::toaplan_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TOAPLAN_SCU, "Toaplan SCU", tag, owner, clock, "toaplan_scu", __FILE__),
	device_gfx_interface(mconfig, *this, gfxinfo )
{
}

void toaplan_scu_device::static_set_xoffsets(device_t &device, int xoffs, int xoffs_flipped)
{
	toaplan_scu_device &dev = downcast<toaplan_scu_device &>(device);
	dev.m_xoffs = xoffs;
	dev.m_xoffs_flipped = xoffs_flipped;
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

void toaplan_scu_device::draw_sprites_to_tempbitmap(const rectangle &cliprect, UINT16* spriteram, UINT32 bytes )
{
	int offs;
	m_temp_spritebitmap.fill(0,cliprect);

	for (offs = 0;offs < bytes/2;offs += 4)
	{
		int attribute,sx,sy,flipx,flipy;
		int sprite, color;

		attribute = spriteram[offs + 1];
		int priority = (attribute & 0x0c00)>>10;

		// are 0 priority really skipped, or can they still mask?
		if (!priority) continue;

		sy = spriteram[offs + 3] >> 7;
		if (sy != 0x0100) {     /* sx = 0x01a0 or 0x0040*/
			sprite = spriteram[offs] & 0x7ff;
			color  = attribute & 0x3f;
			color |= priority << 6; // encode colour

			sx = spriteram[offs + 2] >> 7;
			flipx = attribute & 0x100;
			if (flipx) sx -= m_xoffs_flipped;

			flipy = attribute & 0x200;
			gfx(0)->transpen_raw(m_temp_spritebitmap,cliprect,
				sprite,
				color << 4 /* << 4 because using _raw */ ,
				flipx,flipy,
				sx-m_xoffs,sy-16,0);
		}
	}

}


/***************************************************************************
    Draw the game screen in the given bitmap_ind16.
***************************************************************************/

void toaplan_scu_device::copy_sprites_from_tempbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	int y, x;
	int colourbase = gfx(0)->colorbase();

	for (y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		UINT16* srcline = &m_temp_spritebitmap.pix16(y);
		UINT16* dstline = &bitmap.pix16(y);

		for (x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			UINT16 pix = srcline[x];

			if ( (pix>>(4+6)) == priority )
			{
				if (pix&0xf)
				{
					dstline[x] = (pix & 0x3ff)+colourbase;
				}
			}
		}

	}
}
