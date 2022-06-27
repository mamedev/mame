// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/* video hardware for Namco System II */

#include "emu.h"
#include "namcos2.h"

void namcos2_state::TilemapCB(uint16_t code, int *tile, int *mask)
{
	*mask = code;
	/* The order of bits needs to be corrected to index the right tile  14 15 11 12 13 */
	*tile = bitswap<16>(code, 13, 12, 11, 15, 14, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
}

void namcos2_state::TilemapCB_finalap2(uint16_t code, int *tile, int *mask)
{
	*mask = code;
	*tile = bitswap<15>(code, 13, 12, 11, 14, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
}

/**
 * m_gfx_ctrl selects a bank of 128 sprites within spriteram
 *
 * m_gfx_ctrl also supplies palette and priority information that is applied to the output of the
 *            Namco System 2 ROZ chip
 *
 * -xxx ---- ---- ---- roz priority
 * ---- xxxx ---- ---- roz palette
 * ---- ---- xxxx ---- always zero?
 * ---- ---- ---- xxxx sprite bank
 */
uint16_t namcos2_state::gfx_ctrl_r()
{
	return m_gfx_ctrl;
}

void namcos2_state::gfx_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_gfx_ctrl);
}

/**************************************************************************/

uint8_t namcos2_state::c116_r(offs_t offset)
{
	if ((offset & 0x1800) == 0x1800)
	{
		/* palette register */
		offset &= 0x180f;

		/* registers 6,7: unmapped? */
		if (offset > 0x180b) return 0xff; // fix for finallap boot
	}
	return m_c116->read(offset);
}

/**************************************************************************/

void namcos2_state::create_shadow_table()
{
	/* set table for sprite color == 0x0f */
	for (int i = 0; i < 16 * 256; i++)
	{
		m_c116->shadow_table()[i] = i + 0x2000;
	}
}

/**************************************************************************/

void namcos2_state::video_start()
{
	create_shadow_table();

	save_item(NAME(m_gfx_ctrl));
}

void namcos2_state::apply_clip(rectangle &clip, const rectangle &cliprect)
{
	clip.min_x = m_c116->get_reg(0) - 0x4a;
	clip.max_x = m_c116->get_reg(1) - 0x4a - 1;
	clip.min_y = m_c116->get_reg(2) - 0x21;
	clip.max_y = m_c116->get_reg(3) - 0x21 - 1;
	/* intersect with master clip rectangle */
	clip &= cliprect;
}

uint32_t namcos2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	bitmap.fill(m_c116->black_pen(), cliprect);
	apply_clip(clip, cliprect);

	/* HACK: enable ROZ layer only if it has priority > 0 */
	// Phelios contradicts with this so disabled
	// (level 0 ROZ is actually used by stages 2, 3 and 4 at very least)
	//bool roz_enable = ((m_gfx_ctrl & 0x7000) ? true : false);

	for (pri = 0; pri < 16; pri++)
	{
		if ((pri & 1) == 0)
		{
			m_c123tmap->draw(screen, bitmap, clip, pri / 2);

			//if (roz_enable)
			{
				if (((m_gfx_ctrl & 0x7000) >> 12) == pri / 2)
				{
					m_ns2roz->draw_roz(screen, bitmap, clip, m_gfx_ctrl);
				}
			}
			m_ns2sprite->draw_sprites(screen, bitmap, clip, pri / 2, m_gfx_ctrl);
		}
	}
	return 0;
}

/**************************************************************************/

uint32_t namcos2_state::screen_update_finallap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	bitmap.fill(m_c116->black_pen(), cliprect);
	apply_clip(clip, cliprect);

	for (pri = 0; pri < 16; pri++)
	{
		if ((pri & 1) == 0)
		{
			m_c123tmap->draw(screen, bitmap, clip, pri / 2);
		}
		m_c45_road->draw(bitmap, clip, pri);
		m_ns2sprite->draw_sprites(screen, bitmap, clip, pri, m_gfx_ctrl);
	}
	return 0;
}

/**************************************************************************/

void namcos2_state::RozCB_luckywld(uint16_t code, int *tile, int *mask, int which)
{
	*mask = code;

	uint16_t mangle = bitswap<11>(code & 0x31ff, 13, 12, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	switch ((code >> 9) & 7)
	{
	case 0x00: mangle += 0x1c00; break; // Plus, NOT OR
	case 0x01: mangle |= 0x0800; break;
	case 0x02: mangle |= 0x0000; break;
	default: break;
	}

	*tile = mangle;
}

void namcos2_state::video_start_luckywld()
{
}

uint32_t namcos2_state::screen_update_luckywld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	bitmap.fill(m_c116->black_pen(), cliprect);
	apply_clip(clip, cliprect);

	for (pri = 0; pri < 16; pri++)
	{
		if ((pri & 1) == 0)
		{
			m_c123tmap->draw(screen, bitmap, clip, pri / 2);
		}
		m_c45_road->draw(bitmap, clip, pri);

		if (m_c169roz)
			m_c169roz->draw(screen, bitmap, clip, pri);

		m_c355spr->draw(screen, bitmap, clip, pri);
	}
	return 0;
}

/**************************************************************************/

void namcos2_state::video_start_sgunner()
{
}

uint32_t namcos2_state::screen_update_sgunner(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	bitmap.fill(m_c116->black_pen(), cliprect);
	apply_clip(clip, cliprect);

	for (pri = 0; pri < 8; pri++)
	{
		m_c123tmap->draw(screen, bitmap, clip, pri);
		m_c355spr->draw(screen, bitmap, clip, pri);
	}
	return 0;
}


/**************************************************************************/

void namcos2_state::RozCB_metlhawk(uint16_t code, int *tile, int *mask, int which)
{
	*mask = code;
	*tile = bitswap<13>(code & 0x1fff, 11, 10, 9, 12, 8, 7, 6, 5, 4, 3, 2, 1, 0);
}

void namcos2_state::video_start_metlhawk()
{
}

uint32_t namcos2_state::screen_update_metlhawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	bitmap.fill(m_c116->black_pen(), cliprect);
	apply_clip(clip, cliprect);

	for (pri = 0; pri < 16; pri++)
	{
		if ((pri & 1) == 0)
		{
			m_c123tmap->draw(screen, bitmap, clip, pri / 2);
		}
		m_c169roz->draw(screen, bitmap, clip, pri);
		m_ns2sprite->draw_sprites(screen, bitmap, clip, pri, 0);
	}
	return 0;
}
