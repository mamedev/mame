// license:BSD-3-Clause
// copyright-holders:R. Belmont, ElSemi
/* video/namcofl.cpp */

#include "emu.h"
#include "includes/namcofl.h"

/* nth_word32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of 16 bit words.
 */
#ifdef UNUSED_FUNCTION
static inline uint16_t
nth_word32( const uint32_t *source, int which )
{
	source += which/2;
	which ^= 1; /* i960 is little-endian */
	if( which&1 )
	{
		return (*source)&0xffff;
	}
	else
	{
		return (*source)>>16;
	}
}
#endif

/* nth_byte32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of bytes.
 */
#ifdef UNUSED_FUNCTION
static inline uint8_t
nth_byte32( const uint32_t *pSource, int which )
{
		uint32_t data = pSource[which/4];

		which ^= 3; /* i960 is little-endian */
		switch( which&3 )
		{
		case 0: return data>>24;
		case 1: return (data>>16)&0xff;
		case 2: return (data>>8)&0xff;
		default: return data&0xff;
		}
} /* nth_byte32 */
#endif

void namcofl_state::TilemapCB(uint16_t code, int *tile, int *mask)
{
	*tile = code;
	*mask = code;
}

void namcofl_state::RozCB(uint16_t code, int *tile, int *mask, int which)
{
	*tile = code;
	*mask = code;
}


uint32_t namcofl_state::screen_update_namcofl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int pri;

	bitmap.fill(m_palette->black_pen(), cliprect );

	for( pri=0; pri<16; pri++ )
	{
		m_c169roz->draw(screen, bitmap, cliprect, pri);
		if ((pri & 1) == 0)
		{
			m_c123tmap->draw(screen, bitmap, cliprect, pri >> 1);
		}
		
		m_c355spr->draw(screen, bitmap, cliprect, pri );
	}

	return 0;
}

// NOTE : The two low bits toggle banks (code + 0x4000) for two
//        groups of sprites.  I am unsure how to differentiate those groups
//        at this time however.

WRITE32_MEMBER(namcofl_state::namcofl_spritebank_w)
{
	COMBINE_DATA(&m_sprbank);
}

int namcofl_state::FLobjcode2tile(int code)
{
	if ((code & 0x2000) && (m_sprbank & 2)) { code += 0x4000; }

	return code;
}

VIDEO_START_MEMBER(namcofl_state,namcofl)
{
	m_c355spr->init(NAMCOFL_SPRITEGFX,0x0,namco_c355spr_device::c355_obj_code2tile_delegate(&namcofl_state::FLobjcode2tile, this));
}
