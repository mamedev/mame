/* video/namconb1.c */

#include "driver.h"
#include "namconb1.h"
#include "namcoic.h"
#include "namcos2.h"

static UINT32 tilemap_tile_bank[4];

/* nth_word32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of 16 bit words.
 */
INLINE UINT16
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
INLINE UINT8
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
NB1TilemapCB(UINT16 code, int *tile, int *mask )
{
	*tile = code;
	*mask = code;
} /* NB1TilemapCB */

static void
NB2TilemapCB(UINT16 code, int *tile, int *mask )
{
	int mangle;

	if( namcos2_gametype == NAMCONB2_MACH_BREAKERS )
	{
		/*  00010203 04050607 00010203 04050607 (normal) */
		/*  00010718 191a1b07 00010708 090a0b07 (alt bank) */
		int bank = nth_byte32( namconb1_tilebank32, (code>>13)+8 );
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

static void namconb1_install_palette(running_machine *machine)
{
	int pen, page, dword_offset, byte_offset;
	UINT32 r,g,b;
	UINT32 *pSource;

	/**
     * This is unnecessarily expensive.  Better would be to mark palette entries dirty as
     * they are modified, and only process those that have changed.
     */
	pen = 0;
	for( page=0; page<4; page++ )
	{
		pSource = &paletteram32[page*0x2000/4];
		for( dword_offset=0; dword_offset<0x800/4; dword_offset++ )
		{
			r = pSource[dword_offset+0x0000/4];
			g = pSource[dword_offset+0x0800/4];
			b = pSource[dword_offset+0x1000/4];

			for( byte_offset=0; byte_offset<4; byte_offset++ )
			{
				palette_set_color_rgb( machine, pen++, r>>24, g>>24, b>>24 );
				r<<=8; g<<=8; b<<=8;
			}
		}
	}
} /* namconb1_install_palette */

/**
 * MCU simulation.  It manages coinage, input ports, and presumably
 * communication with the sound CPU.
 */
static void
handle_mcu( void )
{
	static int toggle;
	static UINT16 credits;
	static int old_coin_state;
	static int old_p1;
	static int old_p2;
	static int old_p3;
	static int old_p4;
	int new_coin_state = readinputport(0)&0x3; /* coin1,2 */
	unsigned dsw = readinputport(1)<<16;
	unsigned p1 = readinputport(2);
	unsigned p2 = readinputport(3);
	unsigned p3;
	unsigned p4;
	toggle = !toggle;
	if( toggle ) dsw &= ~(0x80<<16);
	if( namcos2_gametype == NAMCONB2_MACH_BREAKERS )
	{
		p3 = readinputport(4);
		p4 = readinputport(5);
	}
	else
	{
		p3 = 0;
		p4 = 0;
	}

	p1 = (p1&(~old_p1))|(p1<<8);
	p2 = (p2&(~old_p2))|(p2<<8);
	p3 = (p3&(~old_p3))|(p3<<8);
	p4 = (p4&(~old_p4))|(p4<<8);

	old_p1 = p1;
	old_p2 = p2;
	old_p3 = p3;
	old_p4 = p4;

	namconb1_workram32[0x6000/4] = dsw|p1;
	namconb1_workram32[0x6004/4] = (p2<<16)|p3;
	namconb1_workram32[0x6008/4] = p4<<16;

	if( new_coin_state && !old_coin_state )
	{
		credits++;
	}
	old_coin_state = new_coin_state;
	namconb1_workram32[0x601e/4] &= 0xffff0000;
	namconb1_workram32[0x601e/4] |= credits;
} /* handle_mcu */

static void
video_update_common(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int bROZ )
{
	int pri;
	handle_mcu();
	namconb1_install_palette(machine);

	if( bROZ )
	{
		for( pri=0; pri<16; pri++ )
		{
			namco_roz_draw( bitmap,cliprect,pri );
			if( (pri&1)==0 )
			{
				namco_tilemap_draw( bitmap, cliprect, pri/2 );
			}
			namco_obj_draw(machine, bitmap, cliprect, pri );
		}
	}
	else
	{
		for( pri=0; pri<8; pri++ )
		{
			namco_tilemap_draw( bitmap, cliprect, pri );
			namco_obj_draw(machine, bitmap, cliprect, pri );
		}
	}
} /* video_update_common */

/************************************************************************************************/

VIDEO_UPDATE( namconb1 )
{
	/* compute window for custom screen blanking */
	rectangle clip;
	//004a 016a 0021 0101 0144 0020 (nebulas ray)
	UINT32 xclip = paletteram32[0x1800/4];
	UINT32 yclip = paletteram32[0x1804/4];
	clip.min_x = (xclip>>16)    - 0x4a;
	clip.max_x = (xclip&0xffff) - 0x4a - 1;
	clip.min_y = (yclip>>16)    - 0x21;
	clip.max_y = (yclip&0xffff) - 0x21 - 1;
	/* intersect with master clip rectangle */
	if( clip.min_x < cliprect->min_x ){ clip.min_x = cliprect->min_x; }
	if( clip.min_y < cliprect->min_y ){ clip.min_y = cliprect->min_y; }
	if( clip.max_x > cliprect->max_x ){ clip.max_x = cliprect->max_x; }
	if( clip.max_y > cliprect->max_y ){ clip.max_y = cliprect->max_y; }

	fillbitmap( bitmap, get_black_pen(machine), cliprect );

	video_update_common( machine, bitmap, &clip, 0 );

	return 0;
}

static int
NB1objcode2tile( int code )
{
	int bank = nth_word32( namconb1_spritebank32, code>>11 );
	return (code&0x7ff) + bank*0x800;
}

VIDEO_START( namconb1 )
{
	namco_tilemap_init( NAMCONB1_TILEGFX, memory_region(NAMCONB1_TILEMASKREGION), NB1TilemapCB );
	namco_obj_init(NAMCONB1_SPRITEGFX,0x0,NB1objcode2tile);
} /* namconb1 */

/****************************************************************************************************/

VIDEO_UPDATE( namconb2 )
{
	/* compute window for custom screen blanking */
	rectangle clip;
	//004a016a 00210101 01440020
	UINT32 xclip = paletteram32[0x1800/4];
	UINT32 yclip = paletteram32[0x1804/4];
	clip.min_x = (xclip>>16)    - 0x4b;
	clip.max_x = (xclip&0xffff) - 0x4b - 1;
	clip.min_y = (yclip>>16)    - 0x21;
	clip.max_y = (yclip&0xffff) - 0x21 - 1;
	/* intersect with master clip rectangle */
	if( clip.min_x < cliprect->min_x ){ clip.min_x = cliprect->min_x; }
	if( clip.min_y < cliprect->min_y ){ clip.min_y = cliprect->min_y; }
	if( clip.max_x > cliprect->max_x ){ clip.max_x = cliprect->max_x; }
	if( clip.max_y > cliprect->max_y ){ clip.max_y = cliprect->max_y; }

	fillbitmap( bitmap, get_black_pen(machine), cliprect );

	if( memcmp(tilemap_tile_bank,namconb1_tilebank32,sizeof(tilemap_tile_bank))!=0 )
	{
		namco_tilemap_invalidate();
		memcpy(tilemap_tile_bank,namconb1_tilebank32,sizeof(tilemap_tile_bank));
	}
	video_update_common( machine, bitmap, &clip, 1 );
	return 0;
}

static int
NB2objcode2tile( int code )
{
	int bank = nth_byte32( namconb1_spritebank32, (code>>11)&0xf );
	code &= 0x7ff;
	if( namcos2_gametype == NAMCONB2_MACH_BREAKERS )
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

VIDEO_START( namconb2 )
{
	namco_tilemap_init(NAMCONB1_TILEGFX, memory_region(NAMCONB1_TILEMASKREGION), NB2TilemapCB );
	namco_obj_init(NAMCONB1_SPRITEGFX,0x0,NB2objcode2tile);
	namco_roz_init(NAMCONB1_ROTGFX,NAMCONB1_ROTMASKREGION);
} /* namconb2_vh_start */
