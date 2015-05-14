// license:BSD-3-Clause
// copyright-holders:R. Belmont, ElSemi
/* video/namcofl.c */

#include "emu.h"
#include "includes/namcoic.h"
#include "includes/namcofl.h"


/* nth_word32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of 16 bit words.
 */
#ifdef UNUSED_FUNCTION
INLINE UINT16
nth_word32( const UINT32 *source, int which )
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
INLINE UINT8
nth_byte32( const UINT32 *pSource, int which )
{
		UINT32 data = pSource[which/4];

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

static void TilemapCB(running_machine &machine, UINT16 code, int *tile, int *mask )
{
	*tile = code;
	*mask = code;
}


UINT32 namcofl_state::screen_update_namcofl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int pri;

	bitmap.fill(m_palette->black_pen(), cliprect );

	for( pri=0; pri<16; pri++ )
	{
		c169_roz_draw(screen, bitmap, cliprect, pri);
		if((pri&1)==0)
			namco_tilemap_draw( screen, bitmap, cliprect, pri>>1 );
		c355_obj_draw(screen, bitmap, cliprect, pri );
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

static int FLobjcode2tile( running_machine &machine, int code )
{
	namcofl_state *state = machine.driver_data<namcofl_state>();
	if ((code & 0x2000) && (state->m_sprbank & 2)) { code += 0x4000; }

	return code;
}

VIDEO_START_MEMBER(namcofl_state,namcofl)
{
	namco_tilemap_init(NAMCOFL_TILEGFX, memregion(NAMCOFL_TILEMASKREGION)->base(), TilemapCB );
	c355_obj_init(NAMCOFL_SPRITEGFX,0x0,namcos2_shared_state::c355_obj_code2tile_delegate(FUNC(FLobjcode2tile), &machine()));
	c169_roz_init(NAMCOFL_ROTGFX,NAMCOFL_ROTMASKREGION);
}
