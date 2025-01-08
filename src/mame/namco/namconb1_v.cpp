// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/* video/namconb1.c */

#include "emu.h"
#include "namconb1.h"

#include <algorithm>

/* nth_word32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of 16 bit words.
 */
static inline u16
nth_word32(const u32 *source, int which)
{
	source += which / 2;
	if (BIT(which, 0))
	{
		return (*source) & 0xffff;
	}
	else
	{
		return (*source) >> 16;
	}
}

/* nth_byte32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of bytes.
 */
static inline u8
nth_byte32(const u32 *source, int which)
{
	const u32 data = source[which / 4];
	switch (which & 3)
	{
	case 0: return data >> 24;
	case 1: return (data >> 16) & 0xff;
	case 2: return (data >> 8) & 0xff;
	default: return data & 0xff;
	}
} /* nth_byte32 */

void namconb1_state::NB1TilemapCB(u16 code, int &tile, int &mask)
{
	tile = code;
	mask = code;
} /* NB1TilemapCB */

void namconb2_state::NB2TilemapCB_machbrkr(u16 code, int &tile, int &mask)
{
	/*  00010203 04050607 00010203 04050607 (normal) */
	/*  00010718 191a1b07 00010708 090a0b07 (alt bank) */
	const int bank = nth_byte32(m_tilebank32, (code >> 13) + 8);
	const int mangle = (code & 0x1fff) | (bank << 13);
	tile = mangle;
	mask = mangle;
} /* NB2TilemapCB_machbrkr */

void namconb2_state::NB2TilemapCB_outfxies(u16 code, int &tile, int &mask)
{
	/* the pixmap index is mangled, the transparency bitmask index is not */
	tile = bitswap<16>(code, 15, 14, 13, 12, 11, 10, 9, 6, 7, 8, 5, 4, 3, 2, 1, 0);
	mask = code;
} /* NB2TilemapCB_outfxies */

void namconb2_state::NB2RozCB_machbrkr(u16 code, int &tile, int &mask, int which)
{
	const int bank = nth_byte32(&m_rozbank32[which * 8 / 4], (code >> 11) & 0x7);
	const int mangle = (code & 0x7ff) | (bank << 11);
	tile = mangle;
	mask = mangle;
} /* NB2RozCB_machbrkr */

void namconb2_state::NB2RozCB_outfxies(u16 code, int &tile, int &mask, int which)
{
	/* the pixmap index is mangled, the transparency bitmask index is not */
	const int bank = nth_byte32(&m_rozbank32[which * 8 / 4], (code >> 11) & 0x7);
	int mangle = (code & 0x7ff) | (bank << 11);
	mask = mangle;
	mangle = bitswap<19>(mangle & 0x7ffff, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 4, 5, 6, 3, 2, 1, 0);
	tile = mangle;
} /* NB2RozCB_outfxies */

void namconb1_state::screen_vblank(int state)
{
	m_c355spr->vblank(state);
	if (state)
	{
		const u32 size = m_spritebank32.bytes() / 4;
		std::copy_n(&m_spritebank32[0], size, &m_spritebank32_delayed[0]);
	}
}

/************************************************************************************************/

u32 namconb1_state::screen_update_namconb1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* compute window for custom screen blanking */
	rectangle clip;
	//004a 016a 0021 0101 0144 0020 (nebulas ray)
	clip.min_x = s16(m_c116->get_reg(0)) - 0x4a;
	clip.max_x = s16(m_c116->get_reg(1)) - 0x4a - 1;
	clip.min_y = s16(m_c116->get_reg(2)) - 0x21;
	clip.max_y = s16(m_c116->get_reg(3)) - 0x21 - 1;

	/* intersect with master clip rectangle */
	clip &= cliprect;

	bitmap.fill(m_c116->black_pen(), cliprect);

	for (int pri = 0; pri < 8; pri++)
	{
		m_c123tmap->draw(screen, bitmap, clip, pri);
		m_c355spr->draw(screen, bitmap, clip, pri);
	}

	return 0;
}

int namconb1_state::NB1objcode2tile(int code)
{
	const int bank = nth_word32(m_spritebank32_delayed.get(), code >> 11);
	return (code & 0x7ff) | (bank << 11);
}

void namconb1_state::video_start()
{
	const u32 size = m_spritebank32.bytes() / 4;
	m_spritebank32_delayed = make_unique_clear<u32[]>(size);
	save_pointer(NAME(m_spritebank32_delayed), size);
} /* namconb1 */

/****************************************************************************************************/

void namconb2_state::rozbank32_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 old_data = m_rozbank32[offset];
	COMBINE_DATA(&m_rozbank32[offset]);
	if (m_rozbank32[offset] != old_data)
		m_c169roz->mark_all_dirty();
}

u32 namconb2_state::screen_update_namconb2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* compute window for custom screen blanking */
	rectangle clip;
	//004a016a 00210101 01440020
	clip.min_x = m_c116->get_reg(0) - 0x4b;
	clip.max_x = m_c116->get_reg(1) - 0x4b - 1;
	clip.min_y = m_c116->get_reg(2) - 0x21;
	clip.max_y = m_c116->get_reg(3) - 0x21 - 1;
	/* intersect with master clip rectangle */
	clip &= cliprect;

	bitmap.fill(m_c116->black_pen(), cliprect);

	if (memcmp(m_tilemap_tile_bank, m_tilebank32, sizeof(m_tilemap_tile_bank)) != 0)
	{
		m_c123tmap->mark_all_dirty();
		memcpy(m_tilemap_tile_bank, m_tilebank32, sizeof(m_tilemap_tile_bank));
	}
	for (int pri = 0; pri < 16; pri++)
	{
		m_c169roz->draw(screen, bitmap, clip, pri);
		if ((pri & 1) == 0)
		{
			m_c123tmap->draw(screen, bitmap, clip, pri >> 1);
		}
		m_c355spr->draw(screen, bitmap, clip, pri);
	}
	return 0;
}

int namconb2_state::NB2objcode2tile_machbrkr(int code)
{
	const int bank = nth_byte32(m_spritebank32_delayed.get(), (code >> 11) & 0xf);
	code &= 0x7ff;
	code |= bitswap<6>(bank & 0x5f, 6, 4, 3, 2, 1, 0) << 11;
	return code;
} /* NB2objcode2tile_machbrkr */

int namconb2_state::NB2objcode2tile_outfxies(int code)
{
	const int bank = nth_byte32(m_spritebank32_delayed.get(), (code >> 11) & 0xf);
	code &= 0x7ff;
	code |= bitswap<6>(bank & 0x5f, 6, 4, 3, 1, 2, 0) << 11;
	return code;
} /* NB2objcode2tile_outfxies */

void namconb2_state::video_start()
{
	namconb1_state::video_start();
	save_item(NAME(m_tilemap_tile_bank));
} /* namconb2 */
