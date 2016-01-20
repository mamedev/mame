// license:BSD-3-Clause
// copyright-holders:Mark McDougall

/*
 *    Yamaha YGV608 - PVDC2 Pattern mode Video Display Controller 2
 *    - Mark McDougall
 *
 *    Notes:
 *    ======
 *
 *    This implementation is far from complete.
 *    There's enough here to emulate Namco ND-1 games.
 *    Some functionality is missing, some is incomplete.
 *    Also missing for ND-1 is rotation and scaling (cosmetic only).
 *
 *    It could also do with some optimisation for speed!
 *
 *    (Still lots of debugging info/options in here!)
 *
 *    TODO: soon
 *    create tilemaps at vh_start time and switch between at runtime
 *    mark tiles dirty when VRAM is written to instead of dirtying entire
 *       screen each frame
 *
 *    T.B.D. (not critical to ND-1)
 *    ======
 *
 *    Rotation
 *    Scaling
 *    Split-screen scrolling by row (by column supported) (see test mode)
 *    Everything else! :)
 */

#include "emu.h"
#include "video/ygv608.h"

#define _ENABLE_SPRITES
#define _ENABLE_SCROLLX
#define _ENABLE_SCROLLY
//#define _ENABLE_SCREEN_RESIZE
//#define _ENABLE_ROTATE_ZOOM
//#define _SHOW_VIDEO_DEBUG

#define GFX_8X8_4BIT    0
#define GFX_16X16_4BIT  1
#define GFX_32X32_4BIT  2
#define GFX_64X64_4BIT  3
#define GFX_8X8_8BIT    4
#define GFX_16X16_8BIT  5


const device_type YGV608 = &device_creator<ygv608_device>;

ygv608_device::ygv608_device( const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock )
	: device_t(mconfig, YGV608, "YGV608 VDP", tag, owner, clock, "ygv608", __FILE__),
	m_gfxdecode(*this),
	m_palette(*this)
{
}

void ygv608_device::static_set_gfxdecode_tag(device_t &device, std::string tag)
{
	downcast<ygv608_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void ygv608_device::static_set_palette_tag(device_t &device, std::string tag)
{
	downcast<ygv608_device &>(device).m_palette.set_tag(tag);
}

void ygv608_device::set_gfxbank(UINT8 gfxbank)
{
	m_namcond1_gfxbank = gfxbank;
}

/* interrupt generated every 1ms second */
INTERRUPT_GEN_MEMBER(ygv608_device::timed_interrupt )
{
/*
    this is not quite generic, because we trigger a 68k interrupt
    - if this chip is ever used by another driver, we should make
      this more generic - or move it into the machine driver
*/

	static int timer = 0;

	if( ++timer == 1000 )
		timer = 0;

	/* once every 60Hz, set the vertical border interval start flag */
	if( ( timer % (1000/60) ) == 0 )
	{
		m_ports.s.p6 |= p6_fv;
		if (m_regs.s.r14 & r14_iev)
			device.execute().set_input_line(2, HOLD_LINE);
	}

	/* once every 60Hz, set the position detection flag (somewhere) */
	else if( ( timer % (1000/60) ) == 7 )
	{
		m_ports.s.p6 |= p6_fp;
		if (m_regs.s.r14 & r14_iep)
			device.execute().set_input_line(2, HOLD_LINE);
	}
}


TILEMAP_MAPPER_MEMBER( ygv608_device::get_tile_offset )
{
	// this optimisation is not much good to us,
	// since we really need row,col in the get_tile_info() routines
	// - so just pack them into a UINT32

	return( ( col << 6 ) | row );
}

#define layout_total(x) \
(m_gfxdecode->gfx(x)->elements())

TILE_GET_INFO_MEMBER( ygv608_device::get_tile_info_A_8 )
{
	// extract row,col packed into tile_index
	int             col = tile_index >> 6;
	int             row = tile_index & 0x3f;

	UINT8   attr = 0;
	int             pattern_name_base = 0;
	int             set = ((m_regs.s.r7 & r7_md) == MD_1PLANE_256COLOUR
						? GFX_8X8_8BIT : GFX_8X8_4BIT );
	int             base = row >> m_base_y_shift;

	if( col >= m_page_x )
	{
		SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	else if( row >= m_page_y )
	{
		SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	else
	{
		int sx, sy, page;
		int i = pattern_name_base + (((row << m_pny_shift) + col) << m_bits16);
		int j = m_pattern_name_table[i];
		int f = 0;

		if( m_bits16 )
		{
			j += ((int)(m_pattern_name_table[i+1] & m_na8_mask )) << 8;
			// attribute only valid in 16 color mode
			if (set == GFX_8X8_4BIT)
				attr = m_pattern_name_table[i+1] >> 4;

			if (m_regs.s.r7 & r7_flip)
			{
				if (m_pattern_name_table[i+1] & (1<<3)) f |= TILE_FLIPX;
				if (m_pattern_name_table[i+1] & (1<<2)) f |= TILE_FLIPY;
			}
		}

		/* calculate page according to scroll data */
		/* - assuming full-screen scroll only for now... */
		sy = (int)m_scroll_data_table[0][0x00] +
			(((int)m_scroll_data_table[0][0x01] & 0x0f ) << 8);
		sx = (int)m_scroll_data_table[0][0x80] +
			(((int)m_scroll_data_table[0][0x81] & 0x0f ) << 8);

		if ((m_regs.s.r7 & r7_md) == MD_2PLANE_16BIT)
		{
			page = ( ( sx + col * 8 ) % 1024 ) / 256;
			page += ( ( ( sy + row * 8 ) % 2048 ) / 256 ) * 4;
		}
		else if (m_regs.s.r8 & r8_pgs)
		{
			page = ( ( sx + col * 8 ) % 2048 ) / 512;
			page += ( ( ( sy + row * 8 ) % 2048 ) / 256 ) * 4;
		}
		else
		{
			page = ( ( sx + col * 8 ) % 2048 ) / 256;
			page += ( ( ( sy + row * 8 ) % 2048 ) / 512 ) * 8;
		}

		/* add page, base address to pattern name */
		j += ( (int)m_scroll_data_table[0][0xc0+page] << 10 );
		j += ( m_base_addr[0][base] << 8 );

		if( j >= layout_total(set) )
		{
			logerror( "A_8X8: tilemap=%d\n", j );
			j = 0;
		}
		if ((m_regs.s.r12 & r12_apf) != 0)
		{
			// attribute only valid in 16 color mode
			if( set == GFX_8X8_4BIT )
				attr = ( j >> ( ((m_regs.s.r12 & r12_apf) - 1 ) * 2 ) ) & 0x0f;
		}
		// banking
		if (set == GFX_8X8_4BIT)
		{
			j += m_namcond1_gfxbank * 0x10000;
		}
		else // 8x8x8
		{
			j += m_namcond1_gfxbank * 0x8000;
		}

		SET_TILE_INFO_MEMBER(set, j, attr & 0x0F, f );
	}
}

TILE_GET_INFO_MEMBER( ygv608_device::get_tile_info_B_8 )
{
	// extract row,col packed into tile_index
	int             col = tile_index >> 6;
	int             row = tile_index & 0x3f;

	UINT8   attr = 0;
	int             pattern_name_base = ( ( m_page_y << m_pny_shift )
						<< m_bits16 );
	int             set = GFX_8X8_4BIT;
	int             base = row >> m_base_y_shift;

	if ((m_regs.s.r7 & r7_md) & MD_1PLANE )
	{
		SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	else if (col >= m_page_x)
	{
		SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	else if (row >= m_page_y)
	{
		SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	else
	{
		int sx, sy, page;
		int i = pattern_name_base + (((row << m_pny_shift) + col) << m_bits16);
		int j = m_pattern_name_table[i];
		int f = 0;

		if (m_bits16)
		{
			j += ((int)(m_pattern_name_table[i+1] & m_na8_mask )) << 8;
			attr = m_pattern_name_table[i+1] >> 4; /*& 0x00; 0xf0;*/

			if (m_regs.s.r7 & r7_flip)
			{
				if (m_pattern_name_table[i+1] & (1<<3)) f |= TILE_FLIPX;
				if (m_pattern_name_table[i+1] & (1<<2)) f |= TILE_FLIPY;
			}
		}

		/* calculate page according to scroll data */
		/* - assuming full-screen scroll only for now... */
		sy = (int)m_scroll_data_table[1][0x00] +
			(((int)m_scroll_data_table[1][0x01] & 0x0f ) << 8);
		sx = (int)m_scroll_data_table[1][0x80] +
			(((int)m_scroll_data_table[1][0x81] & 0x0f ) << 8);

		if ((m_regs.s.r7 & r7_md) == MD_2PLANE_16BIT )
		{
			page = ( ( sx + col * 8 ) % 1024 ) / 256;
			page += ( ( ( sy + row * 8 ) % 2048 ) / 256 ) * 4;
		}
		else if (m_regs.s.r8 & r8_pgs)
		{
			page = ( ( sx + col * 8 ) % 2048 ) / 512;
			page += ( ( ( sy + row * 8 ) % 2048 ) / 256 ) * 4;
		}
		else
		{
			page = ( ( sx + col * 8 ) % 2048 ) / 256;
			page += ( ( ( sy + row * 8 ) % 2048 ) / 512 ) * 8;
		}

		/* add page, base address to pattern name */
		j += ( (int)m_scroll_data_table[1][0xc0+page] << 10 );
		j += ( m_base_addr[1][base] << 8 );

		if( j >= layout_total(set) )
		{
			logerror( "B_8X8: tilemap=%d\n", j );
			j = 0;
		}
		if ((m_regs.s.r12 & r12_bpf) != 0)
		{
			UINT8 color = (m_regs.s.r12 & r12_bpf) >> 3;

			/* assume 16 colour mode for now... */
			attr = ( j >> ( (color - 1 ) * 2 ) ) & 0x0f;
		}

		// banking
		if (set == GFX_8X8_4BIT)
		{
			j += m_namcond1_gfxbank * 0x10000;
		}
		else // 8x8x8
		{
			j += m_namcond1_gfxbank * 0x8000;
		}

		SET_TILE_INFO_MEMBER(set, j, attr, f );
	}
}

TILE_GET_INFO_MEMBER( ygv608_device::get_tile_info_A_16 )
{
	// extract row,col packed into tile_index
	int             col = tile_index >> 6;
	int             row = tile_index & 0x3f;

	UINT8   attr = 0;
	int             pattern_name_base = 0;
	int             set = ((m_regs.s.r7 & r7_md) == MD_1PLANE_256COLOUR
						? GFX_16X16_8BIT : GFX_16X16_4BIT );
	int             base = row >> m_base_y_shift;

	if( col >= m_page_x ) {
	SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	else if( row >= m_page_y ) {
	SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	else {
	int sx, sy, page;
	int j;
	int i = ( ( ( row << m_pny_shift ) + col ) << m_bits16 );
	int f = 0;
	i += pattern_name_base;

	j = m_pattern_name_table[i];
	if( m_bits16 ) {
		j += ((int)(m_pattern_name_table[i+1] & m_na8_mask )) << 8;
		// attribute only valid in 16 color mode
		if( set == GFX_16X16_4BIT )
		attr = m_pattern_name_table[i+1] >> 4;

		if (m_regs.s.r7 & r7_flip)
		{
		if (m_pattern_name_table[i+1] & (1<<3)) f |= TILE_FLIPX;
		if (m_pattern_name_table[i+1] & (1<<2)) f |= TILE_FLIPY;
		}
	}

	/* calculate page according to scroll data */
	/* - assuming full-screen scroll only for now... */
	sy = (int)m_scroll_data_table[0][0x00] +
			(((int)m_scroll_data_table[0][0x01] & 0x0f ) << 8);
	sx = (int)m_scroll_data_table[0][0x80] +
			(((int)m_scroll_data_table[0][0x81] & 0x0f ) << 8);
	if((m_regs.s.r7 & r7_md) == MD_2PLANE_16BIT ) {
		page = ( ( sx + col * 16 ) % 2048 ) / 512;
		page += ( ( sy + row * 16 ) / 512 ) * 4;
	}
	else if (m_regs.s.r8 & r8_pgs) {
		page = ( sx + col * 16 ) / 512;
		page += ( ( sy + row * 16 ) / 1024 ) * 8;
	}
	else {
		page = ( sx + col * 16 ) / 1024;
		page += ( ( sy + row * 16 ) / 512 ) * 4;
	}

	/* add page, base address to pattern name */
	j += ( (int)m_scroll_data_table[0][0xc0+page] << 8 );
	j += ( m_base_addr[0][base] << 8 );

	if( j >= layout_total(set) ) {
	logerror( "A_16X16: tilemap=%d\n", j );
		j = 0;
	}

	if ((m_regs.s.r12 & r12_apf) != 0)
	{
		// attribute only valid in 16 color mode
		if( set == GFX_16X16_4BIT )
		attr = ( j >> ( ((m_regs.s.r12 & r12_apf)) * 2 ) ) & 0x0f;
	}

	// banking
	if (set == GFX_16X16_4BIT)
	{
		j += m_namcond1_gfxbank * 0x4000;
	}
	else // 8x8x8
	{
		j += m_namcond1_gfxbank * 0x2000;
	}


	SET_TILE_INFO_MEMBER(set, j, attr, f );
	}
}

TILE_GET_INFO_MEMBER( ygv608_device::get_tile_info_B_16 )
{
	// extract row,col packed into tile_index
	int             col = tile_index >> 6;
	int             row = tile_index & 0x3f;

	UINT8   attr = 0;
	int             pattern_name_base = ( ( m_page_y << m_pny_shift )
					<< m_bits16 );
	int             set = GFX_16X16_4BIT;
	int             base = row >> m_base_y_shift;

	if((m_regs.s.r7 & r7_md) & MD_1PLANE ) {
	SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	if( col >= m_page_x ) {
	SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	else if( row >= m_page_y ) {
	SET_TILE_INFO_MEMBER(set, 0, 0, 0 );
	}
	else {
	int sx, sy, page;
	int j;
	int i = ( ( ( row << m_pny_shift ) + col ) << m_bits16 );
	int f = 0;
	i += pattern_name_base;

	j = m_pattern_name_table[i];
	if( m_bits16 ) {
		j += ((int)(m_pattern_name_table[i+1] & m_na8_mask )) << 8;
		attr = m_pattern_name_table[i+1] >> 4; /*& 0x00; 0xf0;*/

		if (m_regs.s.r7 & r7_flip)
		{
		if (m_pattern_name_table[i+1] & (1<<3)) f |= TILE_FLIPX;
		if (m_pattern_name_table[i+1] & (1<<2)) f |= TILE_FLIPY;
		}
	}

	/* calculate page according to scroll data */
	/* - assuming full-screen scroll only for now... */
	sy = (int)m_scroll_data_table[1][0x00] +
			(((int)m_scroll_data_table[1][0x01] & 0x0f ) << 8);
	sx = (int)m_scroll_data_table[1][0x80] +
			(((int)m_scroll_data_table[1][0x81] & 0x0f ) << 8);
	if((m_regs.s.r7 & r7_md) == MD_2PLANE_16BIT ) {
		page = ( ( sx + col * 16 ) % 2048 ) / 512;
		page += ( ( sy + row * 16 ) / 512 ) * 4;
	}
	else if (m_regs.s.r8 & r8_pgs) {
		page = ( sx + col * 16 ) / 512;
		page += ( ( sy + row * 16 ) / 1024 ) * 8;
	}
	else {
		page = ( sx + col * 16 ) / 1024;
		page += ( ( sy + row * 16 ) / 512 ) * 4;
	}

	/* add page, base address to pattern name */
	j += ( (int)m_scroll_data_table[1][0xc0+page] << 8 );
	j += ( m_base_addr[1][base] << 8 );

	if( j >= layout_total(set) ) {
	logerror( "B_16X16: tilemap=%d\n", j );
		j = 0;
	}

	if ((m_regs.s.r12 & r12_bpf) != 0)
	{
		UINT8 color = (m_regs.s.r12 & r12_bpf) >> 3;

		/* assume 16 colour mode for now... */
		attr = ( j >> (color * 2)) & 0x0f;
	}

	// banking
	if (set == GFX_16X16_4BIT)
	{
		j += m_namcond1_gfxbank * 0x4000;
	}
	else // 8x8x8
	{
		j += m_namcond1_gfxbank * 0x2000;
	}

	SET_TILE_INFO_MEMBER(set, j, attr, f );
	}
}

void ygv608_device::postload()
{
	int i;

	m_screen_resize = 1;
	m_tilemap_resize = 1;

	for(i = 0; i < 50; i++)
		SetPostShortcuts(i);
}

void ygv608_device::register_state_save()
{
	save_item(NAME(m_ports.b));
	save_item(NAME(m_regs.b));
	save_item(NAME(m_pattern_name_table));
	save_item(NAME(m_sprite_attribute_table.b));
	save_item(NAME(m_scroll_data_table));
	save_item(NAME(m_colour_palette));

	machine().save().register_postload(save_prepost_delegate(FUNC(ygv608_device::postload), this));
}

void ygv608_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	memset(&m_ports, 0, sizeof(m_ports));
	memset(&m_regs, 0, sizeof(m_regs));
	memset(&m_pattern_name_table, 0, sizeof(m_pattern_name_table));
	memset(&m_sprite_attribute_table, 0, sizeof(m_sprite_attribute_table));

	memset(&m_scroll_data_table, 0, sizeof(m_scroll_data_table));
	memset(&m_colour_palette, 0, sizeof(m_colour_palette));

	m_bits16 = 0;
	m_page_x = 0;
	m_page_y = 0;
	m_pny_shift = 0;
	m_na8_mask = 0;
	m_col_shift = 0;

	m_ax = 0; m_dx = 0; m_dxy = 0; m_ay = 0; m_dy = 0; m_dyx = 0;

	memset(&m_base_addr, 0, sizeof(m_base_addr));
	m_base_y_shift = 0;

	// flag rebuild of the tilemaps
	m_screen_resize = 1;
	m_tilemap_resize = 1;
	m_namcond1_gfxbank = 0;
	save_item(NAME(m_namcond1_gfxbank));

	/* create tilemaps of all sizes and combinations */
	m_tilemap_A_cache_8[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_A_8),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  8,8, 32,32);
	m_tilemap_A_cache_8[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_A_8),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  8,8, 64,32);
	m_tilemap_A_cache_8[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_A_8),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  8,8, 32,64);

	m_tilemap_A_cache_16[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_A_16),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  16,16, 32,32);
	m_tilemap_A_cache_16[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_A_16),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  16,16, 64,32);
	m_tilemap_A_cache_16[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_A_16),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  16,16, 32,64);

	m_tilemap_B_cache_8[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_B_8),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  8,8, 32,32);
	m_tilemap_B_cache_8[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_B_8),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  8,8, 64,32);
	m_tilemap_B_cache_8[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_B_8),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  8,8, 32,64);

	m_tilemap_B_cache_16[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_B_16),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  16,16, 32,32);
	m_tilemap_B_cache_16[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_B_16),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  16,16, 64,32);
	m_tilemap_B_cache_16[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ygv608_device::get_tile_info_B_16),this), tilemap_mapper_delegate(FUNC(ygv608_device::get_tile_offset),this),  16,16, 32,64);

	m_tilemap_A = nullptr;
	m_tilemap_B = nullptr;

	register_state_save();
}

void ygv608_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#ifdef _ENABLE_SPRITES

	// sprites are always clipped to 512x512
	// - regardless of the visible display dimensions
	rectangle spriteClip(0, 512, 0, 512);

	SPRITE_ATTR *sa;
	int flipx = 0, flipy = 0;
	int i;

	/* ensure that sprites are enabled */
	if( ((m_regs.s.r7 & r7_dspe) == 0 ) || (m_regs.s.r10 & r10_sprd))
	return;

	/* draw sprites */
	spriteClip &= cliprect;
	sa = &m_sprite_attribute_table.s[YGV608_MAX_SPRITES-1];
	for( i=0; i<YGV608_MAX_SPRITES; i++, sa-- )
	{
	int code, color, sx, sy, size, attr, g_attr, spf;

	color = (sa->attr >> 4) & 0x0f;
	sx = ( (int)(sa->attr & 0x02) << 7 ) | (int)sa->sx;
	sy = ( ( ( (int)(sa->attr & 0x01) << 8 ) | (int)sa->sy ) + 1 ) & 0x1ff;
	attr = (sa->attr & 0x0c) >> 2;
	g_attr = (m_regs.s.r10 & r10_spa) >> 6;
	spf = (m_regs.s.r12 & r12_spf) >> 6;

	if ((m_regs.s.r10 & r10_spas) == SPAS_SPRITESIZE )
	{
		size = g_attr;
		flipx = (attr & SZ_HORIZREVERSE) != 0;
		flipy = (attr & SZ_VERTREVERSE) != 0;
	}
	else
	{
		size = attr;
		flipx = (g_attr & SZ_HORIZREVERSE) != 0;
		flipy = (g_attr & SZ_VERTREVERSE) != 0;
	}

	switch( size ) {
	case SZ_8X8 :
		code = ( (int)m_regs.s.sba << 8 ) | (int)sa->sn;
		if (spf != 0)
		color = ( code >> ( (spf - 1) * 2 ) ) & 0x0f;
		if( code >= layout_total(GFX_8X8_4BIT) ) {
		logerror( "SZ_8X8: sprite=%d\n", code );
		code = 0;
		}
		m_gfxdecode->gfx(GFX_8X8_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x10000,
			color,
			flipx,flipy,
			sx,sy,0x00);
		// redraw with wrap-around
		if( sx > 512-8 )
		m_gfxdecode->gfx(GFX_8X8_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x10000,
			color,
			flipx,flipy,
			sx-512,sy,0x00);
		if( sy > 512-8 )
		m_gfxdecode->gfx(GFX_8X8_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x10000,
			color,
			flipx,flipy,
			sx,sy-512,0x00);
		// really should draw again for both wrapped!
		// - ignore until someone thinks it's required
		break;

	case SZ_16X16 :
		code = ( ( (int)m_regs.s.sba & 0xfc ) << 6 ) | (int)sa->sn;
		if (spf != 0)
		color = ( code >> (spf * 2) ) & 0x0f;
		if( code >= layout_total(GFX_16X16_4BIT) ) {
		logerror( "SZ_8X8: sprite=%d\n", code );
		code = 0;
		}
		m_gfxdecode->gfx(GFX_16X16_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x4000,
			color,
			flipx,flipy,
			sx,sy,0x00);
		// redraw with wrap-around
		if( sx > 512-16 )
		m_gfxdecode->gfx(GFX_16X16_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x4000,
			color,
			flipx,flipy,
			sx-512,sy,0x00);
		if( sy > 512-16 )
		m_gfxdecode->gfx(GFX_16X16_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x4000,
			color,
			flipx,flipy,
			sx,sy-512,0x00);
		// really should draw again for both wrapped!
		// - ignore until someone thinks it's required
		break;

	case SZ_32X32 :
		code = ( ( (int)m_regs.s.sba & 0xf0 ) << 4 ) | (int)sa->sn;
		if (spf != 0)
	color = ( code >> ( (spf + 1) * 2 ) ) & 0x0f;
		if( code >= layout_total(GFX_32X32_4BIT) ) {
		logerror( "SZ_32X32: sprite=%d\n", code );
	code = 0;
		}
		m_gfxdecode->gfx(GFX_32X32_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x1000,
			color,
			flipx,flipy,
			sx,sy,0x00);
		// redraw with wrap-around
		if( sx > 512-32 )
		m_gfxdecode->gfx(GFX_32X32_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x1000,
			color,
			flipx,flipy,
			sx-512,sy,0x00);
		if( sy > 512-32 )
		m_gfxdecode->gfx(GFX_32X32_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x1000,
			color,
			flipx,flipy,
			sx,sy-512,0x00);
		// really should draw again for both wrapped!
		// - ignore until someone thinks it's required
		break;

	case SZ_64X64 :
		code = ( ( (int)m_regs.s.sba & 0xc0 ) << 2 ) | (int)sa->sn;
		if (spf != 0)
		color = ( code >> ( (spf + 1) * 2 ) ) & 0x0f;
		if( code >= layout_total(GFX_64X64_4BIT) ) {
		logerror( "SZ_64X64: sprite=%d\n", code );
		code = 0;
		}
		m_gfxdecode->gfx(GFX_64X64_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x400,
			color,
			flipx,flipy,
			sx,sy,0x00);
		// redraw with wrap-around
		if( sx > 512-64 )
		m_gfxdecode->gfx(GFX_64X64_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x400,
			color,
			flipx,flipy,
			sx-512,sy,0x00);
		if( sy > 512-64 )
		m_gfxdecode->gfx(GFX_64X64_4BIT)->transpen(bitmap,spriteClip,
			code+m_namcond1_gfxbank*0x400,
			color,
			flipx,flipy,
			sx,sy-512,0x00);
		// really should draw again for both wrapped!
		// - ignore until someone thinks it's required
		break;

	default :
		break;
	}
	}

#endif
}

#ifdef _SHOW_VIDEO_DEBUG
static const char *const mode[] = {
	"2PLANE_8BIT",
	"2PLANE_16BIT",
	"1PLANE_16COLORS",
	"1PLANE_256COLORS"
};

static const char *const psize[] = { "8x8", "16x16", "32x32", "64x64" };
#endif

UINT32 ygv608_device::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#ifdef _SHOW_VIDEO_DEBUG
	char buffer[64];
#endif
#ifdef _ENABLE_SCROLLY
	int col;
#endif
#ifdef _ENABLE_ROTATE_ZOOM
	int xc, yc;
	double r, alpha, sin_theta, cos_theta;
#endif
	rectangle finalclip;
	const rectangle &visarea = screen.visible_area();

	// clip to the current bitmap
	finalclip.set(0, screen.width() - 1, 0, screen.height() - 1);
	finalclip &= cliprect;

	// punt if not initialized
	if (m_page_x == 0 || m_page_y == 0)
	{
		bitmap.fill(0, finalclip);
		return 0;
	}

	if( m_screen_resize )
	{
#ifdef _ENABLE_SCREEN_RESIZE
		// hdw should be scaled by 16, not 8
		// - is it something to do with double dot-clocks???
		screen.set_visible_area(0, ((int)(m_regs.s.hdw)<<3/*4*/)-1,
							0, ((int)(m_regs.s.vdw)<<3)-1 );
#endif

		m_work_bitmap.resize(screen.width(), screen.height());

		// reset resize flag
		m_screen_resize = 0;
	}

	if( m_tilemap_resize )
	{
		int index;

		/* based on the page sizes, pick an index */
		if (m_page_x == 64)
			index = 1;
		else if (m_page_y == 64)
			index = 2;
		else
			index = 0;

		if ((m_regs.s.r9 & r9_pts) == PTS_8X8 )
			m_tilemap_A = m_tilemap_A_cache_8[index];
		else
			m_tilemap_A = m_tilemap_A_cache_16[index];
		m_tilemap_A->mark_all_dirty();

		m_tilemap_A->set_transparent_pen(0 );
		// for NCV1 it's sufficient to scroll only columns
		m_tilemap_A->set_scroll_cols(m_page_x );

		if ((m_regs.s.r9 & r9_pts) == PTS_8X8 )
			m_tilemap_B = m_tilemap_B_cache_8[index];
		else
			m_tilemap_B = m_tilemap_B_cache_16[index];
		m_tilemap_B->mark_all_dirty();

		// for NCV1 it's sufficient to scroll only columns
		m_tilemap_B->set_scroll_cols(m_page_x );

		// now clear the screen in case we change to 1-plane mode
		m_work_bitmap.fill(0, finalclip );

		// reset resize flag
		m_tilemap_resize = 0;
	}

#ifdef _ENABLE_SCROLLY

	for( col=0; col<m_page_x; col++ )
	{
	m_tilemap_B->set_scrolly(col,
				( (int)m_scroll_data_table[1][(col>>m_col_shift)<<1] +
					( (int)m_scroll_data_table[1][((col>>m_col_shift)<<1)+1] << 8 ) ) );

	m_tilemap_A->set_scrolly(col,
				( (int)m_scroll_data_table[0][(col>>m_col_shift)<<1] +
				( (int)m_scroll_data_table[0][((col>>m_col_shift)<<1)+1] << 8 ) ) );
	}

#endif

#ifdef _ENABLE_SCROLLX

	m_tilemap_B->set_scrollx(0,
				( (int)m_scroll_data_table[1][0x80] +
					( (int)m_scroll_data_table[1][0x81] << 8 ) ) );

	m_tilemap_A->set_scrollx(0,
				( (int)m_scroll_data_table[0][0x80] +
				( (int)m_scroll_data_table[0][0x81] << 8 ) ) );

#endif

	m_tilemap_A->enable(m_regs.s.r7 & r7_dspe);
	if((m_regs.s.r7 & r7_md) & MD_1PLANE )
		m_tilemap_B->enable(0 );
	else
		m_tilemap_B->enable(m_regs.s.r7 & r7_dspe);

	m_tilemap_A ->mark_all_dirty();
	m_tilemap_B ->mark_all_dirty();


	/*
	 *    now we can render the screen
	 */

#if 1
	// LBO - need to implement proper pen marking for sprites as well as set aside a non-transparent
	// pen to be used for background fills when plane B is disabled.
	if ((m_regs.s.r7 & r7_md) & MD_1PLANE)
	{
		// If the background tilemap is disabled, we need to clear the bitmap to black
		m_work_bitmap.fill(0, finalclip);
//      m_work_bitmap.fill(1, *visarea);
	}
	else
#endif
		m_tilemap_B->draw(screen, m_work_bitmap, finalclip, 0, 0 );

#ifdef _ENABLE_ROTATE_ZOOM

	/*
	*    fudge - translate ax,ay to startx, starty each time
	*/

	xc = m_ax >> 16;
	yc = m_ay >> 16;
	r = sqrt( (double)( xc * xc + yc * yc ) );
	alpha = atan( (double)xc / (double)yc );
	sin_theta = (double)m_dyx / (double)0x10000;
	cos_theta = (double)m_dx / (double)0x10000;

	if( m_regs.s.zron )
	copyrozbitmap( bitmap, finalclip, &m_work_bitmap,
					( visarea.min_x << 16 ) +
					m_ax + 0x10000 * r *
					( -sin( alpha ) * cos_theta + cos( alpha ) * sin_theta ),
					( visarea.min_y << 16 ) +
					m_ay + 0x10000 * r *
					( cos( alpha ) * cos_theta + sin( alpha ) * sin_theta ),
					m_dx, m_dxy, m_dyx, m_dy, 0);
	else
#endif
	copybitmap( bitmap, m_work_bitmap, 0, 0, 0, 0, finalclip);

	// for some reason we can't use an opaque m_tilemap_A
	// so use a transparent but clear the work bitmap first
	// - look at why this is the case?!?
	m_work_bitmap.fill(0, visarea );

	if ((m_regs.s.r11 & r11_prm) == PRM_ASBDEX ||
		(m_regs.s.r11 & r11_prm) == PRM_ASEBDX )
		draw_sprites(bitmap, finalclip);

	m_tilemap_A->draw(screen, m_work_bitmap, finalclip, 0, 0 );

#ifdef _ENABLE_ROTATE_ZOOM
	if( m_regs.s.zron )
	copyrozbitmap_trans( bitmap, finalclip, &m_work_bitmap,
					m_ax, // + ( visarea.min_x << 16 ),
					m_ay, // + ( visarea.min_y << 16 ),
					m_dx, m_dxy, m_dyx, m_dy, 0,
					0 );
	else
#endif
	copybitmap_trans( bitmap, m_work_bitmap, 0, 0, 0, 0, finalclip, 0 );

	if ((m_regs.s.r11 & r11_prm) == PRM_SABDEX ||
		(m_regs.s.r11 & r11_prm) == PRM_SEABDX)
		draw_sprites(bitmap,finalclip );


#ifdef _SHOW_VIDEO_DEBUG
	/* show screen control information */
	ui_draw_text( mode[(m_regs.s.r7 & r7_md) >> 1], 0, 0 );
	sprintf( buffer, "%02ux%02u", m_page_x, m_page_y );
	ui_draw_text( buffer, 0, 16 );
	ui_draw_text( psize[(m_regs.s.r9 & r9_pts) >> 6], 0, 32 );
	sprintf( buffer, "A: SX:%d SY:%d",
		(int)m_scroll_data_table[0][0x80] +
		( ( (int)m_scroll_data_table[0][0x81] & 0x0f ) << 8 ),
		(int)m_scroll_data_table[0][0x00] +
		( ( (int)m_scroll_data_table[0][0x01] & 0x0f ) << 8 ) );
	ui_draw_text( buffer, 0, 48 );
	sprintf( buffer, "B: SX:%d SY:%d",
		(int)m_scroll_data_table[1][0x80] +
		( ( (int)m_scroll_data_table[1][0x81] & 0x0f ) << 8 ),
		(int)m_scroll_data_table[1][0x00] +
		( ( (int)m_scroll_data_table[1][0x01] & 0x0f ) << 8 ) );
	ui_draw_text( buffer, 0, 64 );
#endif
	return 0;
}

READ16_MEMBER( ygv608_device::read )
{
	static int p0_state = 0;
	static int p3_state = 0;
	static int pattern_name_base = 0;  /* pattern name table base address */
	int pn=0;
	UINT16  data = 0;

	switch (offset)
	{
		case 0x00: /* P#0 - pattern name table data port */
		{
			UINT8 xTile = m_regs.s.r1 & r1_pnx;
			UINT8 yTile = m_regs.s.r0 & r0_pny;

			switch (p0_state)
			{
				case 0:
					/* Are we reading from plane B? */
					if (!((m_regs.s.r7 & r7_md) & MD_1PLANE) && (m_regs.s.r0 & r0_b_a))
						pattern_name_base = ((m_page_y << m_pny_shift) << m_bits16);

					/* read character from ram */
					pn = pattern_name_base + (((yTile << m_pny_shift) + xTile) << m_bits16);
					break;

				case 1:
					/* read character from ram */
					pn = pattern_name_base + (((yTile << m_pny_shift) + xTile) << m_bits16) + 1;
					break;
			}

			if (pn > 4095)
			{
				logerror( "attempt (%d) to read pattern name %d\n"
					"mode = %d, pgs = %d (%dx%d)\n"
					"pattern_name_base = %d\n"
					"pnx = %d, pny = %d, pny_shift = %d, bits16 = %d\n",
					p0_state,
					pn, m_regs.s.r7 & r7_md, m_regs.s.r8 & r8_pgs,
					m_page_x, m_page_y,
					pattern_name_base,
					xTile, yTile, m_pny_shift,
					m_bits16 );
				pn = 0;
			}
			data = m_pattern_name_table[pn];

			p0_state++;
			if ((m_regs.s.r7 & r7_md) == MD_2PLANE_8BIT )
				p0_state++;

			if (p0_state == 2)
			{
				if (m_regs.s.r0 & r0_pnya)
				{
					if (yTile++ == (m_page_y - 1))
					{
						yTile = 0;
						if (xTile++ == (m_page_x - 1))
						{
							xTile = 0;
							// we're now off this tile plane, toggle planes
							m_regs.s.r0 ^= r0_b_a;
						}
					}
					m_regs.s.r0 &= ~r0_pny;
					m_regs.s.r0 |= yTile;
					m_regs.s.r1 &= ~r1_pnx;
					m_regs.s.r1 |= xTile;
				}
				else if (m_regs.s.r1 & r1_pnxa)
				{
					if (xTile++ == (m_page_x - 1))
					{
						xTile = 0;
						if (yTile++ == (m_page_y - 1))
						{
							yTile = 0;
							// we're now off this tile plane, toggle planes
							m_regs.s.r0 ^= r0_b_a;
						}
					}
					m_regs.s.r0 &= ~r0_pny;
					m_regs.s.r0 |= yTile;
					m_regs.s.r1 &= ~r1_pnx;
					m_regs.s.r1 |= xTile;
				}
				p0_state = 0;
				pattern_name_base = 0;
			}
			return (data << 8);
		}

		case 0x01: /* P#1 - sprite data port */
			data = m_sprite_attribute_table.b[m_regs.s.saa];
			if (m_regs.s.r2 & r2_saar)
				m_regs.s.saa++;
			return (data << 8);

		case 0x02: /* P#2 - scroll data port */
			data = m_scroll_data_table[(m_regs.s.r2 & r2_b_a) >> 4][m_regs.s.sca];
			if (m_regs.s.r2 & r2_scar)
			{
				m_regs.s.sca++;
				/* handle wrap to next plane */
				if (m_regs.s.sca == 0)
					m_regs.s.r2 ^= r2_b_a;
			}
			return( data << 8 );

		case 0x03: /* P#3 - color palette data port */
			data = m_colour_palette[m_regs.s.cc][p3_state];
			if( ++p3_state == 3 )
			{
				p3_state = 0;
				if( m_regs.s.r2 & r2_cpar)
					m_regs.s.cc++;
			}
			return( data << 8 );

		case 0x04: /* P#4 - register data port */
		{
			UINT8 regNum = (m_ports.s.p5) & p5_rn;
			data = m_regs.b[regNum];
			if (m_ports.s.p5 & p5_rrai)
			{
				regNum ++;
				if (regNum == 50)
				{
					regNum = 0;
					logerror( "warning: rn=50 after read increment\n" );
				}
				m_ports.s.p5 &= ~p5_rn;
				m_ports.s.p5 |= regNum;
			}
			return (data << 8);
		}

		case 0x05:
			break;

		case 0x06:
		case 0x07:
			return( (UINT16)(m_ports.b[offset]) << 8 );

		default :
			logerror( "unknown ygv608 register (%d)\n", offset );
			break;
	}

	return( 0 );
}

WRITE16_MEMBER( ygv608_device::write )
{
	static int p0_state = 0;
	static int p3_state = 0;
	static int pattern_name_base = 0;  /* pattern name table base address */
	int pn=0;

	data = ( data >> 8 ) & 0xff;

	switch (offset)
	{
		case 0x00: /* P#0 - pattern name table data port */
		{
			UINT8 xTile = m_regs.s.r1 & r1_pnx;
			UINT8 yTile = m_regs.s.r0 & r0_pny;

			switch (p0_state)
			{
				case 0:
					/* Are we reading from plane B? */
					if (!((m_regs.s.r7 & r7_md) & MD_1PLANE) && (m_regs.s.r0 & r0_b_a))
						pattern_name_base = ((m_page_y << m_pny_shift) << m_bits16);

					/* read character from ram */
					pn = pattern_name_base + (((yTile << m_pny_shift) + xTile) << m_bits16);
					break;

				case 1:
					/* read character from ram */
					pn = pattern_name_base + (((yTile << m_pny_shift) + xTile) << m_bits16) + 1;
					break;
			}

			if (pn > 4095)
			{
				logerror( "attempt (%d) to read pattern name %d\n"
					"mode = %d, pgs = %d (%dx%d)\n"
					"pattern_name_base = %d\n"
					"pnx = %d, pny = %d, pny_shift = %d, bits16 = %d\n",
					p0_state,
					pn, m_regs.s.r7 & r7_md, m_regs.s.r8 & r8_pgs,
					m_page_x, m_page_y,
					pattern_name_base,
					xTile, yTile, m_pny_shift,
					m_bits16 );
				pn = 0;
			}
			m_pattern_name_table[pn] = data;

			p0_state++;
			if ((m_regs.s.r7 & r7_md) == MD_2PLANE_8BIT )
				p0_state++;

			if (p0_state == 2)
			{
				if (m_regs.s.r0 & r0_pnya)
				{
					if (yTile++ == (m_page_y - 1))
					{
						yTile = 0;
						if (xTile++ == (m_page_x - 1))
						{
							xTile = 0;
							m_regs.s.r0 ^= r0_b_a;
						}
					}
					m_regs.s.r0 &= ~r0_pny;
					m_regs.s.r0 |= yTile;
					m_regs.s.r1 &= ~r1_pnx;
					m_regs.s.r1 |= xTile;
				}
				else if (m_regs.s.r1 & r1_pnxa)
				{
					if (xTile++ == (m_page_x - 1))
					{
						xTile = 0;
						if (yTile++ == (m_page_y - 1))
						{
							yTile = 0;
							m_regs.s.r0 ^= r0_b_a;
						}
					}
					m_regs.s.r0 &= ~r0_pny;
					m_regs.s.r0 |= yTile;
					m_regs.s.r1 &= ~r1_pnx;
					m_regs.s.r1 |= xTile;
				}
				p0_state = 0;
				pattern_name_base = 0;
			}
		}
		break;

		case 0x01: /* P#1 - sprite data port */
			m_sprite_attribute_table.b[m_regs.s.saa] = data;
			if( m_regs.s.r2 & r2_saaw)
				m_regs.s.saa++;
			break;

		case 0x02: /* P#2 - scroll data port */
			m_scroll_data_table[(m_regs.s.r2 & r2_b_a) >> 4][m_regs.s.sca] = data;
			if (m_regs.s.r2 & r2_scaw)
			{
				m_regs.s.sca++;
				/* handle wrap to next plane */
				if (m_regs.s.sca == 0)
					m_regs.s.r2 ^= r2_b_a;
			}
			break;

		case 0x03: /* P#3 - colour palette data port */
			m_colour_palette[m_regs.s.cc][p3_state] = data;
			if (++p3_state == 3)
			{
				p3_state = 0;
				m_palette->set_pen_color(m_regs.s.cc,
					pal6bit(m_colour_palette[m_regs.s.cc][0]),
					pal6bit(m_colour_palette[m_regs.s.cc][1]),
					pal6bit(m_colour_palette[m_regs.s.cc][2]) );
				if (m_regs.s.r2 & r2_cpaw)
					m_regs.s.cc++;
			}
			break;

		case 0x04: /* P#4 - register data port */
		{
			UINT8 regNum = (m_ports.s.p5) & p5_rn;
#if 0
			logerror( "R#%d = $%02X\n", regNum, data );
#endif
			SetPreShortcuts (regNum, data);
			m_regs.b[regNum] = data;
			SetPostShortcuts (regNum);
			if (m_ports.s.p5 & p5_rwai)
			{
				regNum ++;
				if (regNum == 50)
				{
					regNum = 0;
					logerror( "warning: rn=50 after write increment\n" );
				}
				m_ports.s.p5 &= ~p5_rn;
				m_ports.s.p5 |= regNum;
			}
		}
		break;

		case 0x05: /* P#5 - register select port */
			m_ports.b[5] = data;
			break;

		case 0x06: /* P#6 - status port */
			/* writing a '1' resets that bit */
			m_ports.b[6] &= ~data;
			break;

		case 0x07: /* P#7 - system control port */
			m_ports.b[7] = data;
			if (m_ports.b[7] & 0x3e)
				HandleRomTransfers();
			if (m_ports.b[7] & 0x01)
				HandleYGV608Reset();
			break;

		default:
			logerror( "unknown ygv608 register (%d)\n", offset );
			break;
	}
}

void ygv608_device::HandleYGV608Reset()
{
	int i;

	/* Clear ports #0-7 */
	memset( &m_ports.b[0], 0, 8 );

	/* Clear registers #0-38, #47-49 */
	memset( &m_regs.b[0], 0, 39 );
	memset( &m_regs.b[47], 0, 3 );

	/* Clear internal ram */
	memset( m_pattern_name_table, 0, 4096 );
	memset( m_sprite_attribute_table.b, 0,
			YGV608_SPRITE_ATTR_TABLE_SIZE );
	memset( m_scroll_data_table, 0, 2*256 );
	memset( m_colour_palette, 0, 256*3 );

	/* should set shortcuts here too */
	for( i=0; i<50; i++ ) {
		//SetPreShortcuts( i );
		SetPostShortcuts( i );
	}
}

/*
    The YGV608 has a function to block-move data from the rom into
    internal tables. This function is not used in NCV1, but I used
    it for testing trojan ROM software.
    - So leave it in!
 */

void ygv608_device::HandleRomTransfers()
{
#if 0
	static UINT8 *sdt = (UINT8 *)m_scroll_data_table;
	static UINT8 *sat = (UINT8 *)m_sprite_attribute_table.b;

	/* fudge copy from sprite data for now... */
	UINT8 *RAM = machine.memory_region[0];
	int i;

	int src = ( ( (int)m_regs.s.tb13 << 8 ) +
			(int)m_regs.s.tb5 ) << 5;
	int bytes = (int)m_regs.s.tn4 << 4;

	logerror( "Transferring data from rom...\n" );

	/* pattern name table */
	if( m_ports.s.tn ) {
	}

	/* scroll table */
	if( m_ports.s.tl ) {
	int dest = (int)m_regs.s.sca;
	if( m_regs.s.p2_b_a )
		dest += 0x100;

	/* fudge a transfer for now... */
	for( i=0; i<bytes; i++ ) {
		sdt[(dest+i)%512] = RAM[src+(i^0x01)];

	}

	/* flag as finished */
	m_ports.s.tl = 0;
	}

	/* sprite attribute table */
	if( m_ports.s.ts ) {
	int dest = (int)m_regs.s.saa;

	/* fudge a transfer for now... */
	for( i=0; i<bytes; i++ ) {
		sat[(dest+i)%256] = RAM[src+(i^0x01)];

	}

	/* flag as finished */
	m_ports.s.ts = 0;
	}
#endif
}

#if 0
void nvsram( offs_t offset, UINT16 data )
{
	static int i = 0;

	data = ( data >> 8 ) & 0xff;

	if( 1 ) {
	static char ascii[16];
	if( i%16 == 0 )
		logerror( "%04X: ", offset );
	logerror( "%02X ", data );
	ascii[i%16] = (data > 0x20) ? data : '.';
	if( i%16 == 15 )
		logerror( "| %-16.16s\n", ascii );
	}

	i++;
}
#endif

// Set any "short-cut" variables before we update the YGV608 registers
// - these are used only in optimisation of the emulation

void ygv608_device::SetPreShortcuts( int reg, int data )
{
	switch( reg ) {
		case 7 :
			if( ( ( data >> MD_SHIFT ) & MD_MASK ) != (m_regs.s.r7 & r7_md))
				m_tilemap_resize = 1;
			break;

		case 8 :
			if( ( ( data >> PGS_SHIFT ) & PGS_MASK ) != (m_regs.s.r8 & r8_pgs))
				m_tilemap_resize = 1;
			break;

		case 9 :
			if( ( ( data >> PTS_SHIFT ) & PTS_MASK ) != (m_regs.s.r9 & r9_pts))
				m_tilemap_resize= 1;
			break;

		case 40 :
			if( ( ( data >> HDW_SHIFT ) & HDW_MASK ) != (m_regs.s.r40 & r40_hdw))
				m_screen_resize = 1;
			break;

		case 44 :
			if( ( ( data >> VDW_SHIFT ) & VDW_MASK ) != (m_regs.s.r44 & r44_vdw))
				m_screen_resize = 1;
			break;
	}
}

// Set any "short-cut" variables after we have updated the YGV608 registers
// - these are used only in optimisation of the emulation

void ygv608_device::SetPostShortcuts(int reg )
{
	int plane, addr;

	switch (reg)
	{
	case 0:
	{
		UINT8 yTile = m_regs.s.r0 & r0_pny;

		if (yTile >= m_page_y)
		logerror ("%s:setting pny(%d) >= page_y(%d)\n", machine().describe_context(),
			yTile, m_page_y );
		yTile &= (m_page_y - 1);
		m_regs.s.r0 &= ~r0_pny;
		m_regs.s.r0 |= yTile;
	}
	break;

	case 1:
	{
		UINT8 xTile = m_regs.s.r1 & r1_pnx;

		if (xTile >= m_page_x)
		logerror ("%s:setting pnx(%d) >= page_x(%d)\n", machine().describe_context(),
			xTile, m_page_x );
		xTile &= (m_page_x - 1);
		m_regs.s.r1 &= ~r1_pnx;
		m_regs.s.r1 |= xTile;
	}
	break;

	case 6:
#if 0
		logerror( "SBA = $%08X\n", (int)m_regs.s.sba << 13 );
#endif
		break;

	case 7:
		m_na8_mask = ((m_regs.s.r7 & r7_flip) ? 0x03 : 0x0f );
		/* fall through */

	case 8 :
	m_bits16 = ((m_regs.s.r7 & r7_md) == MD_2PLANE_8BIT ? 0 : 1 );
	if((m_regs.s.r7 & r7_md) == MD_2PLANE_16BIT )
		m_page_x = m_page_y = 32;
	else {
		if ((m_regs.s.r8 & r8_pgs) == 0 ) {
		m_page_x = 64;
		m_page_y = 32;
		}
		else {
		m_page_x = 32;
		m_page_y = 64;
		}
	}
	m_pny_shift = ( m_page_x == 32 ? 5 : 6 );

	/* bits to shift pattern y coordinate to extract base */
	m_base_y_shift = ( m_page_y == 32 ? 2 : 3 );

	break;

	case 9 :
	switch (m_regs.s.r9 & r9_slv)
	{
		case SLV_SCREEN :
			// always use scoll table entry #1
			m_col_shift = 8;
			break;
		default :
			if ((m_regs.s.r9 & r9_pts) == PTS_8X8 )
				m_col_shift = (m_regs.s.r9 & r9_slv) - 4;
			else
				m_col_shift = (m_regs.s.r9 & r9_slv) - 5;
			if( m_col_shift < 0 )
			{
				// we can't handle certain conditions
				logerror( "Unhandled slv condition (pts=$%X,slv=$%X)\n",
							m_regs.s.r9 & r9_pts, m_regs.s.r9 & r9_slv);
				m_col_shift = 8;
			}
			break;
	}

	//if( m_regs.s.slh != SLH_SCREEN )
	//    logerror( "SLH = %1X\n", m_regs.s.slh );
	break;

	case 11 :
	//ShowYGV608Registers();
	break;

	case 17 : case 18 : case 19 : case 20 :
	case 21 : case 22 : case 23 : case 24 :
	plane = (reg-17) >> 2;
	addr = ( (reg-17) << 1 ) & 0x07;
	m_base_addr[plane][addr] = m_regs.b[reg] & 0x0f;
	m_base_addr[plane][addr+1] = m_regs.b[reg] >> 4;
	break;

	case 25 : case 26 : case 27 :
	m_ax = (int)(m_regs.s.ax16 & 0x1f) << 16 |
				(int)m_regs.s.ax8 << 8 |
				(int)m_regs.s.ax0;
	m_ax <<= 7;
	if( m_ax & 0x08000000 ) m_ax |= 0xf8000000;   // 2s complement
	//logerror( "m_ax = $%X\n", m_ax );
	break;

	case 28 : case 29 :
	m_dx = (int)(m_regs.s.dx8 & 0x1f) << 8 | (int)m_regs.s.dx0;
	m_dx <<= 7;
	if( m_dx & 0x00080000 ) m_dx |= 0xfff80000;   // 2s complement
	break;

	case 30 : case 31 :
	m_dxy = (int)(m_regs.s.dxy8 & 0x1f) << 8 | (int)m_regs.s.dxy0;
	m_dxy <<= 7;
	if( m_dxy & 0x00080000 ) m_dxy |= 0xfff80000; // 2s complement
	break;

	case 32 : case 33 : case 34 :
	m_ay = (int)(m_regs.s.ay16 & 0x1f) << 16 |
				(int)m_regs.s.ay8 << 8 |
				(int)m_regs.s.ay0;
	m_ay <<= 7;
	if( m_ay & 0x08000000 ) m_ay |= 0xf8000000;   // 2s complement
	//logerror( "m_ay = $%X\n", m_ay );
	break;

	case 35 : case 36 :
	m_dy = (int)(m_regs.s.dy8 & 0x1f) << 8 | (int)m_regs.s.dy0;
	m_dy <<= 7;
	if( m_dy & 0x00080000 ) m_dy |= 0xfff80000;   // 2s complement
	break;

	case 37 : case 38 :
	m_dyx = (int)(m_regs.s.dyx8 & 0x1f) << 8 | (int)m_regs.s.dyx0;
	m_dyx <<= 7;
	if( m_dyx & 0x00080000 ) m_dyx |= 0xfff80000; // 2s complement
	break;

	case 40 : case 41 : case 42 :
	//ShowYGV608Registers();
	break;

	}

}

/*
 *      The rest of this stuff is for debugging only!
 */


//#define SHOW_SOURCE_MODE

#if 0
void dump_block( char *name, UINT8 *block, int len )
{
	int i;

	logerror( "UINT8 %s[] = {\n", name );
	for( i=0; i<len; i++ ) {
	if( i%8 == 0 )
		logerror( " " );
	logerror( "0x%02X, ", block[i] );
	if( i%8 == 7 )
		logerror( "\n" );
	}
	logerror( "};\n" );
}
#endif
READ16_MEMBER( ygv608_device::debug_trigger_r )
{
	static int oneshot = 0;

#ifndef SHOW_SOURCE_MODE
	int i;
#endif
	char ascii[16];

	if( oneshot )
	return( 0 );
	oneshot = 1;

	ShowYGV608Registers();

#ifdef SHOW_SOURCE_MODE
#if 0
	dump_block( "ygv608_regs",
			(UINT8 *)m_regs.b,
			64 );
	dump_block( "ygv608_pnt",
			(UINT8 *)m_pattern_name_table,
			4096 );
	dump_block( "ygv608_sat",
			(UINT8 *)m_sprite_attribute_table.b,
			256 );
	dump_block( "ygv608_sdt",
			(UINT8 *)m_scroll_data_table,
			512 );
	dump_block( "ygv608_cp",
			(UINT8 *)m_colour_palette,
			768 );
#endif

#else

	/*
	*  Dump pattern name table ram
	*/
#if 1
	logerror( "Pattern Name Table\n" );
	for( i=0; i<4096; i++ ) {
	if( i % 16 == 0 )
		logerror( "$%04X : ", i );
	logerror( "%02X ", m_pattern_name_table[i] );
	if( m_pattern_name_table[i] >= 0x20)
		ascii[i%16] = m_pattern_name_table[i];
	else
		ascii[i%16] = '.';
	if( i % 16 == 15 )
		logerror( " | %-16.16s\n", ascii );
	}
	logerror( "\n" );
#endif

	/*
	*  Dump scroll table ram
	*/

	logerror( "Scroll Table\n" );
	for( i=0; i<256; i++ ) {
	if( i % 16 == 0 )
		logerror( "$%04X : ", i );
	logerror( "%02X ", m_scroll_data_table[0][i] );
	if( m_scroll_data_table[0][i] >= 0x20 )
		ascii[i%16] = m_scroll_data_table[0][i];
	else
		ascii[i%16] = '.';
	if( i % 16 == 15 )
		logerror( " | %-16.16s\n", ascii );
	}
	logerror( "\n" );

#endif

	return( 0 );
}

void ygv608_device::ShowYGV608Registers()
{
	int p, b;

	logerror( "YGV608 Registers\n" );
	logerror(
		"\tR#00: $%02X : PNYA(%d),B/A(%c),PNY(%d)\n",
		m_regs.b[0],
		m_regs.s.r0 & r0_pnya,
		((m_regs.s.r0 & r0_b_a) ? 'B' : 'A' ),
		m_regs.s.r0 & r0_pny);

	logerror(
		"\tR#01: $%02X : PNXA(%d),PNX(%d)\n",
		m_regs.b[1],
		m_regs.s.r1 & r1_pnxa,
		m_regs.s.r1 & r1_pnx);

	logerror(
		"\tR#02: $%02X : CPAW(%d),CPAR(%d),B/A(%d),SCAW(%d),SCAR(%d),SAAW(%d),SAAR(%d)\n",
		m_regs.b[2],
		m_regs.s.r2 & r2_cpaw,
		m_regs.s.r2 & r2_cpar,
		m_regs.s.r2 & r2_b_a,
		m_regs.s.r2 & r2_scaw,
		m_regs.s.r2 & r2_scar,
		m_regs.s.r2 & r2_saaw,
		m_regs.s.r2 & r2_saar);

	logerror(
		"\tR#03: $%02X : SAA($%02X)\n",
		m_regs.b[3],
		m_regs.s.saa );

	logerror(
		"\tR#04: $%02X : SCA($%02X)\n",
		m_regs.b[4],
		m_regs.s.sca );

	logerror(
		"\tR#05: $%02X : CC($%02X)\n",
		m_regs.b[5],
		m_regs.s.cc );

	logerror(
		"\tR#06: $%02X : SBA($%02X)\n",
		m_regs.b[6],
		m_regs.s.sba );

	logerror(
		"\tR#07: $%02X : DSPE(%d),MD(%d),ZRON(%d),FLIP(%d),DCKM(%d)\n",
		m_regs.b[7],
		m_regs.s.r7 & r7_dspe,
		m_regs.s.r7 & r7_md,
		m_regs.s.r7 & r7_zron,
		m_regs.s.r7 & r7_flip,
		m_regs.s.r7 & r7_dckm);

	logerror(
		"\tR#08: $%02X : HDS(%d),VDS(%d),RLRT(%d),RLSC(%d),PGS(%d)\n",
		m_regs.b[8],
		m_regs.s.r8 & r8_hds,
		m_regs.s.r8 & r8_vds,
		m_regs.s.r8 & r8_rlrt,
		m_regs.s.r8 & r8_rlsc,
		m_regs.s.r8 & r8_pgs);

	logerror(
		"\tR#11: $%02X : CTPA(%d),CTPB(%d),PRM(%d),CBDR(%d),YSE(%d),SCM(%d)\n",
		m_regs.b[11],
		m_regs.s.r11 & r11_ctpa,
		m_regs.s.r11 & r11_ctpb,
		m_regs.s.r11 & r11_prm,
		m_regs.s.r11 & r11_cbdr,
		m_regs.s.r11 & r11_yse,
		m_regs.s.r11 & r11_scm);

	logerror(
		"\tR#40: $%02X : HTL9:8($%02X)=$%06X,HDW(%d)\n",
		m_regs.b[40],
		m_regs.s.r40 & r40_htl89, (int)(m_regs.s.r40 & r40_htl89) << 8,
		m_regs.s.r40 & r40_hdw);

	logerror(
		"\tR#41: $%02X : HDSP($%02X)\n",
		m_regs.b[41],
		m_regs.s.hdsp );

	logerror(
		"\tR#42: $%02X : HTL7:0($%02X)\n",
		m_regs.b[42],
		m_regs.s.htl );

	logerror(
		"\t              HTL=$%03X\n",
		( (int)(m_regs.s.r40 & r40_htl89) << 8 ) |
		( (int)m_regs.s.htl ) );

	logerror(
		"\tR#47: $%02X : TB12:5($%02X) = $%06X\n",
		m_regs.b[47],
		m_regs.s.tb5, (int)m_regs.s.tb5 << 5 );

	logerror(
		"\tR#48: $%02X : TB20:13($%02X) = $%06X\n",
		m_regs.b[48],
		m_regs.s.tb13, (int)m_regs.s.tb13 << 13 );

	logerror(
		"\t              TB=$%06X\n",
		( (int)m_regs.s.tb13 << 13 ) |
		( (int)m_regs.s.tb5 << 5 ) );

	logerror(
		"\tR#49: $%02X : TN11:4($%02X) = $%04X\n",
		m_regs.b[49],
		m_regs.s.tn4, (int)m_regs.s.tn4 << 4 );

	logerror(
		"ShortCuts:\n" );

	for( p=0; p<2; p++ ) {
	logerror( "\t" );
	for( b=0; b<8; b++ ) {
		logerror( "%02X ", m_base_addr[p][b] );
	}
	logerror( "\n" );
	}
}
