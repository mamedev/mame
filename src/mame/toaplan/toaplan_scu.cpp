// license:BSD-3-Clause
// copyright-holders:Quench
/* Toaplan Sprite Controller 'SCU'
 used by video/twincobr.cpp (including wardner)
 and rallybik in toaplan1.cpp
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
	, device_video_interface(mconfig, *this)
	, m_pri_cb(*this)
{
}

void toaplan_scu_device::device_start()
{
	m_pri_cb.resolve();
}

void toaplan_scu_device::device_reset()
{
}

/***************************************************************************
    Sprite Handlers
***************************************************************************/

template<class BitmapClass>
void toaplan_scu_device::draw_sprites_common(BitmapClass &bitmap, const rectangle &cliprect, u16* spriteram, u32 bytes)
{
	for (int offs = (bytes / 2) - 4; offs >= 0; offs -= 4)
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
			u32 pri_mask     = 0; // priority mask
			if (!m_pri_cb.isnull())
				m_pri_cb(priority, pri_mask);

			int sx          = spriteram[offs + 2] >> 7;
			const int flipx = attribute & 0x100;
			if (flipx) sx  -= m_xoffs_flipped;

			const int flipy = attribute & 0x200;
			gfx(0)->prio_transpen(bitmap, cliprect,
				sprite,
				color,
				flipx, flipy,
				sx - m_xoffs, sy - 16, screen().priority(), pri_mask, 0);
		}
	}
}


void toaplan_scu_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, u16* spriteram, u32 bytes)
{
	draw_sprites_common(bitmap, cliprect, spriteram, bytes);
}


void toaplan_scu_device::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, u16* spriteram, u32 bytes)
{
	draw_sprites_common(bitmap, cliprect, spriteram, bytes);
}


