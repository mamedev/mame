// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/* video/namconb1.c */

#include "emu.h"
#include "includes/namconb1.h"
#include "includes/namcoic.h"


/* nth_word32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of 16 bit words.
 */
static inline UINT16
nth_word32( const UINT32 *source, int which )
{
	source += which/2;
	if( which&1 )
	{
		return (*source)&0xffff;
	}
	else
	{
		return (*source)>>16;
	}
}

/* nth_byte32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of bytes.
 */
static inline UINT8
nth_byte32( const UINT32 *pSource, int which )
{
	UINT32 data = pSource[which/4];
	switch( which&3 )
	{
	case 0: return data>>24;
	case 1: return (data>>16)&0xff;
	case 2: return (data>>8)&0xff;
	default: return data&0xff;
	}
} /* nth_byte32 */

static void
NB1TilemapCB(running_machine &machine, UINT16 code, int *tile, int *mask )
{
	*tile = code;
	*mask = code;
} /* NB1TilemapCB */

static void
NB2TilemapCB(running_machine &machine, UINT16 code, int *tile, int *mask )
{
	namconb1_state *state = machine.driver_data<namconb1_state>();
	int mangle;

	if( state->m_gametype == NAMCONB2_MACH_BREAKERS )
	{
		/*  00010203 04050607 00010203 04050607 (normal) */
		/*  00010718 191a1b07 00010708 090a0b07 (alt bank) */
		int bank = nth_byte32( state->m_tilebank32, (code>>13)+8 );
		mangle = (code&0x1fff) + bank*0x2000;
		*tile = mangle;
		*mask = mangle;
	}
	else
	{
		/* the pixmap index is mangled, the transparency bitmask index is not */
		mangle = code&~(0x140);
		if( code&0x100 ) mangle |= 0x040;
		if( code&0x040 ) mangle |= 0x100;
		*tile = mangle;
		*mask = code;
	}
} /* NB2TilemapCB */

static void
video_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bROZ )
{
	namconb1_state *state = screen.machine().driver_data<namconb1_state>();
	int pri;

	if( bROZ )
	{
		for( pri=0; pri<16; pri++ )
		{
			state->c169_roz_draw(screen, bitmap, cliprect, pri);
			if( (pri&1)==0 )
			{
				namco_tilemap_draw( screen, bitmap, cliprect, pri/2 );
			}
			state->c355_obj_draw(screen, bitmap, cliprect, pri );
		}
	}
	else
	{
		for( pri=0; pri<8; pri++ )
		{
			namco_tilemap_draw( screen, bitmap, cliprect, pri );
			state->c355_obj_draw(screen, bitmap, cliprect, pri );
		}
	}
} /* video_update_common */

/************************************************************************************************/

UINT32 namconb1_state::screen_update_namconb1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* compute window for custom screen blanking */
	rectangle clip;
	//004a 016a 0021 0101 0144 0020 (nebulas ray)
	clip.min_x = m_c116->get_reg(0) - 0x4a;
	clip.max_x = m_c116->get_reg(1) - 0x4a - 1;
	clip.min_y = m_c116->get_reg(2) - 0x21;
	clip.max_y = m_c116->get_reg(3) - 0x21 - 1;
	/* intersect with master clip rectangle */
	clip &= cliprect;

	bitmap.fill(m_palette->black_pen(), cliprect );

	video_update_common( screen, bitmap, clip, 0 );

	return 0;
}

static int
NB1objcode2tile( running_machine &machine, int code )
{
	namconb1_state *state = machine.driver_data<namconb1_state>();
	int bank = nth_word32( state->m_spritebank32, code>>11 );
	return (code&0x7ff) + bank*0x800;
}

VIDEO_START_MEMBER(namconb1_state,namconb1)
{
	namco_tilemap_init(NAMCONB1_TILEGFX, memregion(NAMCONB1_TILEMASKREGION)->base(), NB1TilemapCB );
	c355_obj_init(NAMCONB1_SPRITEGFX,0x0,namcos2_shared_state::c355_obj_code2tile_delegate(FUNC(NB1objcode2tile), &machine()));
} /* namconb1 */

/****************************************************************************************************/

UINT32 namconb1_state::screen_update_namconb2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

	bitmap.fill(m_palette->black_pen(), cliprect );

	if( memcmp(m_tilemap_tile_bank,m_tilebank32,sizeof(m_tilemap_tile_bank))!=0 )
	{
		namco_tilemap_invalidate();
		memcpy(m_tilemap_tile_bank,m_tilebank32,sizeof(m_tilemap_tile_bank));
	}
	video_update_common( screen, bitmap, clip, 1 );
	return 0;
}

static int
NB2objcode2tile( running_machine &machine, int code )
{
	namconb1_state *state = machine.driver_data<namconb1_state>();
	int bank = nth_byte32( state->m_spritebank32, (code>>11)&0xf );
	code &= 0x7ff;
	if( state->m_gametype == NAMCONB2_MACH_BREAKERS )
	{
		if( bank&0x01 ) code |= 0x01*0x800;
		if( bank&0x02 ) code |= 0x02*0x800;
		if( bank&0x04 ) code |= 0x04*0x800;
		if( bank&0x08 ) code |= 0x08*0x800;
		if( bank&0x10 ) code |= 0x10*0x800;
		if( bank&0x40 ) code |= 0x20*0x800;
	}
	else
	{
		if( bank&0x01 ) code |= 0x01*0x800;
		if( bank&0x02 ) code |= 0x04*0x800;
		if( bank&0x04 ) code |= 0x02*0x800;
		if( bank&0x08 ) code |= 0x08*0x800;
		if( bank&0x10 ) code |= 0x10*0x800;
		if( bank&0x40 ) code |= 0x20*0x800;
	}
	return code;
} /* NB2objcode2tile */

VIDEO_START_MEMBER(namconb1_state,namconb2)
{
	namco_tilemap_init(NAMCONB1_TILEGFX, memregion(NAMCONB1_TILEMASKREGION)->base(), NB2TilemapCB );
	c355_obj_init(NAMCONB1_SPRITEGFX,0x0,namcos2_shared_state::c355_obj_code2tile_delegate(FUNC(NB2objcode2tile), &machine()));
	c169_roz_init(NAMCONB1_ROTGFX,NAMCONB1_ROTMASKREGION);
} /* namconb2_vh_start */
