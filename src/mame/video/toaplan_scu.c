/* Toaplan Sprite Controller 'SCU'
 used by video/twincobr.c (including wardner)
 and rallybik in toaplan1.c
*/



#include "emu.h"
#include "toaplan_scu.h"

const device_type TOAPLAN_SCU = &device_creator<toaplan_scu_device>;

toaplan_scu_device::toaplan_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TOAPLAN_SCU, "toaplan_scu_device", tag, owner, clock, "toaplan_scu", __FILE__),
	m_gfxregion(0),
	m_gfxdecode(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void toaplan_scu_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<toaplan_scu_device &>(device).m_gfxdecode.set_tag(tag);
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

void toaplan_scu_device::set_gfx_region(int region)
{
	m_gfxregion = region;
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
			if (flipx) sx -= 14;        /* should really be 15 */
			flipy = attribute & 0x200;
			m_gfxdecode->gfx(m_gfxregion)->transpen_raw(m_temp_spritebitmap,cliprect,
				sprite,
				color << 4 /* << 4 because using _raw */ ,
				flipx,flipy,
				sx-32,sy-16,0);

		}
	}

}


/***************************************************************************
    Draw the game screen in the given bitmap_ind16.
***************************************************************************/

void toaplan_scu_device::copy_sprites_from_tempbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	int y, x;
	int colourbase = m_gfxdecode->gfx(m_gfxregion)->colorbase();

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
