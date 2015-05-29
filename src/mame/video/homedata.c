// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria
#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "includes/homedata.h"

/*
    video control registers:
    0 xxxxxxxx  unknown
    1 x-------  flip screen
      -xxxxxxx  unknown
    2 xxxxxxxx  unknown
    3 xxxxxxxx  related to visible area?
    4 ----xxxx  fine horiz scroll (sub-character, 8 or 12 pixels)
      xxxx----  unknown
    5 xxxxxxxx  related to visible area?
    6 xxxxxxxx  related to visible area?
    7 xxxxxxxx  unknown
    8 xxxxxxxx  unknown
    9 xxxxxxxx  unknown
    a xxxxxxxx  unknown
    b xxxxxx--  horiz scroll (character resolution)
    c xxxxxxxx  might be bg layer horiz scroll (always 0 here but used by the older games)
    d xxxxxxxx  might be fg layer horiz scroll (always 0 here but always 4 in the older games)
 */


/***************************************************************************

  Blitter

  This is probably work done by the GX61A01 custom chip found on all boards,
  however the way it works is not exactly the same in all games.

***************************************************************************/

void homedata_state::mrokumei_handleblit( address_space &space, int rom_base )
{
	int i;
	int dest_param;
	int source_addr;
	int dest_addr;
	int base_addr;
	int opcode, data, num_tiles;
	UINT8 *pBlitData = memregion("user1")->base() + rom_base;

	dest_param = m_blitter_param[(m_blitter_param_count - 4) & 3] * 256 +
		m_blitter_param[(m_blitter_param_count - 3) & 3];

	source_addr = m_blitter_param[(m_blitter_param_count - 2) & 3] * 256 +
		m_blitter_param[(m_blitter_param_count - 1) & 3];

	/*  xxx-.----.----.---- not used?
	 *  ---x.----.----.---- layer
	 *  ----.xxxx.xxxx.xxxx addr
	 */
	base_addr= (dest_param & 0x1000);
	dest_addr= (dest_param & 0x0fff);

//  logerror( "[blit bank %02x src %04x dst %04x]\n", blitter_bank, source_addr, dest_param);

	if( m_visible_page == 0 )
	{
		base_addr += 0x2000;
	}

	for(;;)
	{
		opcode = pBlitData[source_addr++];
		/* 00xxxxxx RLE incrementing
		 * 01xxxxxx Raw Run
		 * 1xxxxxxx RLE constant data
		 */
		if( opcode == 0x00 )
		{
			/* end-of-graphic */
			goto finish;
		}
		data  = pBlitData[source_addr++];

		if (opcode & 0x80)
			num_tiles = 0x80 - (opcode & 0x7f);
		else
			num_tiles = 0x40 - (opcode & 0x3f);

		for (i = 0; i < num_tiles; i++)
		{
			if (i != 0)
			{
				switch (opcode & 0xc0)
				{
				case 0x40: // Raw Run
					data  = pBlitData[source_addr++];
					break;

				case 0x00: // RLE incrementing
					data++;
					break;
				}
			} /* i!=0 */

			if (data)   /* 00 is a nop */
				mrokumei_videoram_w(space, base_addr + dest_addr, data);

			if (m_vreg[1] & 0x80)    /* flip screen */
			{
				dest_addr -= 2;
				if (dest_addr < 0)
					goto finish;
			}
			else
			{
				dest_addr+=2;
				if (dest_addr >= 0x1000) goto finish;
			}
		} /* for( i=0; i<num_tiles; i++ ) */
	} /* for(;;) */

finish:
	m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

void homedata_state::reikaids_handleblit( address_space &space, int rom_base )
{
	int i;
	UINT16 dest_param;
	int flipx;
	int source_addr, base_addr;
	int dest_addr;
	UINT8 *pBlitData = memregion("user1")->base() + rom_base;

	int opcode, data, num_tiles;

	dest_param = m_blitter_param[(m_blitter_param_count - 4) & 3] * 256 +
		m_blitter_param[(m_blitter_param_count - 3) & 3];

	source_addr = m_blitter_param[(m_blitter_param_count - 2) & 3] * 256 +
		m_blitter_param[(m_blitter_param_count - 1) & 3];

	/*  x---.----.----.---- flipx
	 *  -x--.----.----.---- select: attr/tile
	 *  --*x.xxxx.*xxx.xxxx addr
	 */
	base_addr = (dest_param & 0x4000);
	dest_addr = (dest_param & 0x3fff);
	flipx   = (dest_param & 0x8000);

//  logerror( "[blit %02x %04x %04x]\n",blitter_bank,source_addr,dest_param);

	if (m_visible_page == 0)
		base_addr += 0x2000 << 2;

	for(;;)
	{
		opcode = pBlitData[source_addr++];
		/* 00xxxxxx Raw Run
		 * 01xxxxxx RLE incrementing
		 * 1xxxxxxx RLE constant data
		 */
		if (opcode == 0x00)
		{
			/* end-of-graphic */
			goto finish;
		}

		data  = pBlitData[source_addr++];

		if ((opcode & 0xc0) == 0x80)
			num_tiles = 0x80 - (opcode & 0x7f);
		else
			num_tiles = 0x40 - (opcode & 0x3f);

		for (i = 0; i < num_tiles; i++)
		{
			if (i != 0)
			{
				switch (opcode & 0xc0)
				{
				case 0x00: // Raw Run
					data  = pBlitData[source_addr++];
					break;

				case 0x40: // RLE incrementing
					data++;
					break;
				}
			} /* i!=0 */

			if (data)   /* 00 is a nop */
			{
				int addr = base_addr + (dest_addr & 0x3fff);
				int dat = data;

				if ((addr & 0x2080) == 0)
				{
					addr = ((addr & 0xc000) >> 2) | ((addr & 0x1f00) >> 1) | (addr & 0x7f);

					if (flipx)
					{
						if ((base_addr & 0x4000) == 0)
							dat |= 0x80;
						addr ^= 0x007c;
					}

					reikaids_videoram_w(space, addr, dat);
				}
			}

			if (m_vreg[1] & 0x80)    /* flip screen */
				dest_addr-=4;
			else
				dest_addr+=4;
		} /* for( i=0; i<num_tiles; i++ ) */
	}

finish:
	m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

void homedata_state::pteacher_handleblit( address_space &space, int rom_base )
{
	int i;
	int dest_param;
	int source_addr;
	int dest_addr, base_addr;
	int opcode, data, num_tiles;
	UINT8 *pBlitData = memregion("user1")->base() + rom_base;

	dest_param = m_blitter_param[(m_blitter_param_count - 4) & 3] * 256 +
		m_blitter_param[(m_blitter_param_count - 3) & 3];

	source_addr = m_blitter_param[(m_blitter_param_count - 2) & 3] * 256 +
		m_blitter_param[(m_blitter_param_count - 1) & 3];

	/*  x---.----.----.---- not used?
	 *  -x--.----.----.---- layer   (0..1)
	 *  --*x.xxxx.*xxx.xxxx addr
	 */
	base_addr = (dest_param & 0x4000);
	dest_addr = (dest_param & 0x3fff);

//  logerror( "[blit %02x %04x %04x]->%d\n",blitter_bank,source_addr,dest_param,homedata_visible_page);

	if (m_visible_page == 0)
	{
		base_addr += 0x2000 << 2;
	}

	for(;;)
	{
		opcode = pBlitData[source_addr++];
		/* 00xxxxxx Raw Run
		 * 01xxxxxx RLE incrementing
		 * 1xxxxxxx RLE constant data
		 */
		if (opcode == 0x00)
		{
			/* end-of-graphic */
			goto finish;
		}
		data  = pBlitData[source_addr++];

		if (opcode & 0x80)
			num_tiles = 0x80 - (opcode & 0x7f);
		else
			num_tiles = 0x40 - (opcode & 0x3f);

		for (i = 0; i < num_tiles; i++)
		{
			if (i != 0)
			{
				switch (opcode & 0xc0)
				{
				case 0x00: // Raw Run
					data  = pBlitData[source_addr++];
					break;

				case 0x40: // RLE incrementing
					data++;
					break;
				}
			} /* i!=0 */

			if (data)   /* 00 is a nop */
			{
				int addr = base_addr + (dest_addr & 0x3fff);

				if ((addr & 0x2080) == 0)
				{
					addr = ((addr & 0xc000) >> 2) | ((addr & 0x1f00) >> 1) | (addr & 0x7f);
					mrokumei_videoram_w(space, addr, data);
				}
			}

			if (m_vreg[1] & 0x80)    /* flip screen */
				dest_addr -= 2;
			else
				dest_addr += 2;
		} /* for( i=0; i<num_tiles; i++ ) */
	} /* for(;;) */

finish:
	m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}




/***************************************************************************

  Palette setup

***************************************************************************/

PALETTE_INIT_MEMBER(homedata_state,mrokumei)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* initialize 555 RGB palette */
	for (i = 0; i < 0x8000; i++)
	{
		int r,g,b;
		int color = color_prom[i*2] * 256 + color_prom[i*2+1];
		/* xxxx--------x--- red
		 * ----xxxx-----x-- green
		 * --------xxxx--x- blue
		 * ---------------x unused
		 */
		r = ((color >> 11) & 0x1e) | ((color >> 3) & 1);
		g = ((color >>  7) & 0x1e) | ((color >> 2) & 1);
		b = ((color >>  3) & 0x1e) | ((color >> 1) & 1);

		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

PALETTE_INIT_MEMBER(homedata_state,reikaids)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* initialize 555 RGB palette */
	for (i = 0; i < 0x8000; i++)
	{
		int r,g,b;
		int color = color_prom[i*2] * 256 + color_prom[i*2+1];
		/* xxxx--------x--- green
		 * ----xxxx-----x-- red
		 * --------xxxx--x- blue
		 * ---------------x unused
		 */
		g = ((color >> 11) & 0x1e) | ((color >> 3) & 1);
		r = ((color >>  7) & 0x1e) | ((color >> 2) & 1);
		b = ((color >>  3) & 0x1e) | ((color >> 1) & 1);

		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

PALETTE_INIT_MEMBER(homedata_state,pteacher)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* initialize 555 RGB palette */
	for (i = 0; i < 0x8000; i++)
	{
		int r,g,b;
		int color = color_prom[i*2] * 256 + color_prom[i*2+1];
		/* xxxxx----------- green
		 * -----xxxxx------ red
		 * ----------xxxxx- blue
		 * ---------------x unused
		 */
		g = ((color >> 11) & 0x1f);
		r = ((color >>  6) & 0x1f);
		b = ((color >>  1) & 0x1f);

		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

PALETTE_INIT_MEMBER(homedata_state,mirderby)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 0x100; i++)
	{
		int r,g,b;
		r = color_prom[0x000+i];
		g = color_prom[0x100+i];
		b = color_prom[0x200+i];

		palette.set_pen_color(i,pal4bit(r),pal4bit(g),pal4bit(b));
	}
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

inline void homedata_state::mrokumei_info0( tile_data &tileinfo, int tile_index, int page, int gfxbank )
{
	int addr  = tile_index * 2 + 0x2000 * page;
	int attr  = m_videoram[addr];
	int code  = m_videoram[addr + 1] + ((attr & 0x03) << 8) + (gfxbank << 10);
	int color = (attr >> 2) + (gfxbank << 6);

	SET_TILE_INFO_MEMBER(0, code, color, m_flipscreen );
}
inline void homedata_state::mrokumei_info1( tile_data &tileinfo, int tile_index, int page, int gfxbank )
{
	int addr  = tile_index * 2 + 0x1000 + 0x2000 * page;
	int attr  = m_videoram[addr];
	int code  = m_videoram[addr + 1] + ((attr & 0x07) << 8) + (gfxbank << 11);
	int color = (attr >> 3) + ((gfxbank & 3) << 6);

	SET_TILE_INFO_MEMBER(1, code, color, m_flipscreen );
}

TILE_GET_INFO_MEMBER(homedata_state::mrokumei_get_info0_0)
{
	mrokumei_info0(tileinfo, tile_index, 0,  m_blitter_bank & 0x03 );
}

TILE_GET_INFO_MEMBER(homedata_state::mrokumei_get_info1_0)
{
	mrokumei_info0(tileinfo, tile_index, 1,  m_blitter_bank & 0x03 );
}

TILE_GET_INFO_MEMBER(homedata_state::mrokumei_get_info0_1)
{
	mrokumei_info1(tileinfo, tile_index, 0, (m_blitter_bank & 0x38) >> 3 );
}

TILE_GET_INFO_MEMBER(homedata_state::mrokumei_get_info1_1)
{
	mrokumei_info1(tileinfo, tile_index, 1, (m_blitter_bank & 0x38) >> 3 );
}


inline void homedata_state::reikaids_info( tile_data &tileinfo, int tile_index, int page, int layer, int gfxbank )
{
	int addr  = tile_index * 4 + layer + 0x2000 * page;
	int attr  = m_videoram[addr];
	int code  = m_videoram[addr + 0x1000] + ((attr & 0x03) << 8) + (gfxbank << 10);
	int color = (attr & 0x7c) >> 2;
	int flags = m_flipscreen;

	if (attr & 0x80) flags ^= TILE_FLIPX;

	SET_TILE_INFO_MEMBER(layer, code, color, flags );
}

	/* reikaids_gfx_bank[0]:
	 *      -xxx.x---   layer#1
	 *      ----.-xxx   layer#3
	 *
	 * reikaids_gfx_bank[1]:
	 *      xxxx.x---   layer#0
	 *      ----.-xxx   layer#2
	 */
TILE_GET_INFO_MEMBER(homedata_state::reikaids_get_info0_0)
{
	reikaids_info(tileinfo, tile_index, 0, 0,  (m_gfx_bank[1] >> 3));
}

TILE_GET_INFO_MEMBER(homedata_state::reikaids_get_info1_0)
{
	reikaids_info(tileinfo, tile_index, 1, 0,  (m_gfx_bank[1] >> 3));
}

TILE_GET_INFO_MEMBER(homedata_state::reikaids_get_info0_1)
{
	reikaids_info(tileinfo, tile_index, 0, 1, ((m_gfx_bank[0] & 0x78) >> 3));
}

TILE_GET_INFO_MEMBER(homedata_state::reikaids_get_info1_1)
{
	reikaids_info(tileinfo, tile_index, 1, 1, ((m_gfx_bank[0] & 0x78) >> 3));
}

TILE_GET_INFO_MEMBER(homedata_state::reikaids_get_info0_2)
{
	reikaids_info(tileinfo, tile_index, 0, 2,  (m_gfx_bank[1] & 0x7));
}

TILE_GET_INFO_MEMBER(homedata_state::reikaids_get_info1_2)
{
	reikaids_info(tileinfo, tile_index, 1, 2,  (m_gfx_bank[1] & 0x7));
}

TILE_GET_INFO_MEMBER(homedata_state::reikaids_get_info0_3)
{
	reikaids_info(tileinfo, tile_index, 0, 3,  (m_gfx_bank[0] & 0x7));
}

TILE_GET_INFO_MEMBER(homedata_state::reikaids_get_info1_3)
{
	reikaids_info(tileinfo, tile_index, 1, 3,  (m_gfx_bank[0] & 0x7));
}


inline void homedata_state::pteacher_info( tile_data &tileinfo, int tile_index, int page, int layer, int gfxbank )
{
	int addr  = tile_index * 2 + 0x1000 * layer + 0x2000 * page;
	int attr  = m_videoram[addr];
	int code  = m_videoram[addr + 1] + ((attr & 0x07) << 8) + (gfxbank << 11);
	int color = (attr >> 3) + ((gfxbank & 1) << 5);

	SET_TILE_INFO_MEMBER(layer, code, color, m_flipscreen);
}

TILE_GET_INFO_MEMBER(homedata_state::pteacher_get_info0_0)
{
	pteacher_info(tileinfo, tile_index, 0, 0, m_gfx_bank[0] & 0x0f);
}

TILE_GET_INFO_MEMBER(homedata_state::pteacher_get_info1_0)
{
	pteacher_info(tileinfo, tile_index, 1, 0, m_gfx_bank[0] & 0x0f);
}

TILE_GET_INFO_MEMBER(homedata_state::pteacher_get_info0_1)
{
	pteacher_info(tileinfo, tile_index, 0, 1, m_gfx_bank[0] >> 4);
}

TILE_GET_INFO_MEMBER(homedata_state::pteacher_get_info1_1)
{
	pteacher_info(tileinfo, tile_index, 1, 1, m_gfx_bank[0] >> 4);
}


inline void homedata_state::lemnangl_info( tile_data &tileinfo, int tile_index, int page, int layer, int gfxset, int gfxbank )
{
	int addr  = tile_index * 2 + 0x1000 * layer + 0x2000 * page;
	int attr  = m_videoram[addr];
	int code  = m_videoram[addr + 1] + ((attr & 0x07) << 8) + (gfxbank << 11);
	int color = 16 * (attr >> 3) + gfxbank;

	SET_TILE_INFO_MEMBER(2 * layer + gfxset, code, color, m_flipscreen);
}

TILE_GET_INFO_MEMBER(homedata_state::lemnangl_get_info0_0)
{
	lemnangl_info(tileinfo, tile_index, 0, 0,  m_blitter_bank & 1, m_gfx_bank[0] & 0x0f );
}

TILE_GET_INFO_MEMBER(homedata_state::lemnangl_get_info1_0)
{
	lemnangl_info(tileinfo, tile_index, 1, 0,  m_blitter_bank & 1, m_gfx_bank[0] & 0x0f );
}


TILE_GET_INFO_MEMBER(homedata_state::lemnangl_get_info0_1)
{
	lemnangl_info(tileinfo, tile_index, 0, 1, (m_blitter_bank & 2) >> 1, m_gfx_bank[0] >> 4 );
}

TILE_GET_INFO_MEMBER(homedata_state::lemnangl_get_info1_1)
{
	lemnangl_info(tileinfo, tile_index, 1, 1, (m_blitter_bank & 2) >> 1, m_gfx_bank[0] >> 4 );
}


inline void homedata_state::mirderby_info0( tile_data &tileinfo, int tile_index, int page, int gfxbank )
{
	int addr  = tile_index * 2 + 0x2000 * page;
	int attr  = m_videoram[addr];
	int code  = m_videoram[addr + 1] + ((attr & 0x03) << 8) + 0x400;// + (gfxbank << 10);
	int color = (attr >> 2) + (gfxbank << 6);

	SET_TILE_INFO_MEMBER(0, code, color, m_flipscreen );
}
inline void homedata_state::mirderby_info1( tile_data &tileinfo, int tile_index, int page, int gfxbank )
{
	int addr  = tile_index * 2 + 0x1000 + 0x2000 * page;
	int attr  = m_videoram[addr];
	int code  = m_videoram[addr + 1] + ((attr & 0x07) << 8) + 0x400;//(gfxbank << 11);
	int color = (attr >> 3) + ((gfxbank & 3) << 6);

	SET_TILE_INFO_MEMBER(1, code, color, m_flipscreen );
}

TILE_GET_INFO_MEMBER(homedata_state::mirderby_get_info0_0)
{
	mirderby_info0(tileinfo, tile_index, 0, 0);// m_blitter_bank & 0x03 );
}

TILE_GET_INFO_MEMBER(homedata_state::mirderby_get_info1_0)
{
	mirderby_info0(tileinfo, tile_index, 1, 0);// m_blitter_bank & 0x03 );
}

TILE_GET_INFO_MEMBER(homedata_state::mirderby_get_info0_1)
{
	mirderby_info1(tileinfo, tile_index, 0, 0);//(m_blitter_bank & 0x38) >> 3 );
}

TILE_GET_INFO_MEMBER(homedata_state::mirderby_get_info1_1)
{
	mirderby_info1(tileinfo, tile_index, 1, 0);//(m_blitter_bank & 0x38) >> 3 );
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(homedata_state,mrokumei)
{
	m_bg_tilemap[0][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::mrokumei_get_info0_0),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32 );
	m_bg_tilemap[0][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::mrokumei_get_info0_1),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32 );
	m_bg_tilemap[1][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::mrokumei_get_info1_0),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32 );
	m_bg_tilemap[1][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::mrokumei_get_info1_1),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32 );

	m_bg_tilemap[0][1]->set_transparent_pen(0);
	m_bg_tilemap[1][1]->set_transparent_pen(0);
}

VIDEO_START_MEMBER(homedata_state,reikaids)
{
	m_bg_tilemap[0][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::reikaids_get_info0_0),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_bg_tilemap[0][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::reikaids_get_info0_1),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_bg_tilemap[0][2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::reikaids_get_info0_2),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_bg_tilemap[0][3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::reikaids_get_info0_3),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_bg_tilemap[1][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::reikaids_get_info1_0),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_bg_tilemap[1][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::reikaids_get_info1_1),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_bg_tilemap[1][2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::reikaids_get_info1_2),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_bg_tilemap[1][3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::reikaids_get_info1_3),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);

	m_bg_tilemap[0][0]->set_transparent_pen(0xff);
	m_bg_tilemap[0][1]->set_transparent_pen(0xff);
	m_bg_tilemap[0][2]->set_transparent_pen(0xff);
	m_bg_tilemap[0][3]->set_transparent_pen(0xff);
	m_bg_tilemap[1][0]->set_transparent_pen(0xff);
	m_bg_tilemap[1][1]->set_transparent_pen(0xff);
	m_bg_tilemap[1][2]->set_transparent_pen(0xff);
	m_bg_tilemap[1][3]->set_transparent_pen(0xff);
}

VIDEO_START_MEMBER(homedata_state,pteacher)
{
	m_bg_tilemap[0][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::pteacher_get_info0_0),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[0][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::pteacher_get_info0_1),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::pteacher_get_info1_0),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::pteacher_get_info1_1),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap[0][1]->set_transparent_pen(0xff);
	m_bg_tilemap[1][1]->set_transparent_pen(0xff);
}

VIDEO_START_MEMBER(homedata_state,lemnangl)
{
	m_bg_tilemap[0][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::lemnangl_get_info0_0),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[0][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::lemnangl_get_info0_1),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::lemnangl_get_info1_0),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::lemnangl_get_info1_1),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap[0][1]->set_transparent_pen(0x0f);
	m_bg_tilemap[1][1]->set_transparent_pen(0x0f);
}

VIDEO_START_MEMBER(homedata_state,mirderby)
{
	m_bg_tilemap[0][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::mirderby_get_info0_0),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32 );
	m_bg_tilemap[0][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::mirderby_get_info0_1),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32 );
	m_bg_tilemap[1][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::mirderby_get_info1_0),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32 );
	m_bg_tilemap[1][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(homedata_state::mirderby_get_info1_1),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32 );

	m_bg_tilemap[0][1]->set_transparent_pen(0);
	m_bg_tilemap[1][1]->set_transparent_pen(0);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(homedata_state::mrokumei_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap[(offset & 0x2000) >> 13][(offset & 0x1000) >> 12]->mark_tile_dirty((offset & 0xffe) >> 1);
}

WRITE8_MEMBER(homedata_state::reikaids_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap[(offset & 0x2000) >> 13][offset & 3]->mark_tile_dirty((offset & 0xffc) >> 2);
}


WRITE8_MEMBER(homedata_state::reikaids_gfx_bank_w)
{
//logerror( "%04x: [setbank %02x]\n",space.device().safe_pc(),data);

	if (m_gfx_bank[m_reikaids_which] != data)
	{
		m_gfx_bank[m_reikaids_which] = data;
		machine().tilemap().mark_all_dirty();
	}

	m_reikaids_which ^= 1;
}

WRITE8_MEMBER(homedata_state::pteacher_gfx_bank_w)
{
//  logerror("%04x: gfxbank:=%02x\n", space.device().safe_pc(), data);
	if (m_gfx_bank[0] != data)
	{
		m_gfx_bank[0] = data;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(homedata_state::homedata_blitter_param_w)
{
//logerror("%04x: blitter_param_w %02x\n", space.device().safe_pc(), data);
	m_blitter_param[m_blitter_param_count] = data;
	m_blitter_param_count++;
	m_blitter_param_count &= 3;
}

WRITE8_MEMBER(homedata_state::mrokumei_blitter_bank_w)
{
	/* --xxx--- layer 1 gfx bank
	   -----x-- blitter ROM bank
	   ------xx layer 0 gfx bank
	 */

	if ((m_blitter_bank ^ data) & 0x3b)
		machine().tilemap().mark_all_dirty();

	m_blitter_bank = data;
}

WRITE8_MEMBER(homedata_state::reikaids_blitter_bank_w)
{
	/* xxx----- priority control
	   ----x--- target page? what's this for?
	   ------xx blitter ROM bank
	 */
	m_blitter_bank = data;
}

WRITE8_MEMBER(homedata_state::pteacher_blitter_bank_w)
{
	/* xxx----- blitter ROM bank
	   -----x-- pixel clock (normal/slow)
	   ------x- layer #1 gfx charset (lemnangl only)
	   -------x layer #0 gfx charset (lemnangl only)
	 */

	if ((m_blitter_bank ^ data) & 0x03)
		machine().tilemap().mark_all_dirty();

	m_blitter_bank = data;
}

WRITE8_MEMBER(homedata_state::mrokumei_blitter_start_w)
{
	if (data & 0x80)
		mrokumei_handleblit(space, ((m_blitter_bank & 0x04) >> 2) * 0x10000);

	/* bit 0 = bank switch; used by hourouki to access the
	   optional service mode ROM (not available in current dump) */
}

WRITE8_MEMBER(homedata_state::reikaids_blitter_start_w)
{
	reikaids_handleblit(space, (m_blitter_bank & 3) * 0x10000);
}

WRITE8_MEMBER(homedata_state::pteacher_blitter_start_w)
{
	pteacher_handleblit(space, (m_blitter_bank >> 5) * 0x10000 & (memregion("user1")->bytes() - 1));
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 homedata_state::screen_update_mrokumei(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flags,width;

	/* blank screen */
	if (m_vreg[0x3] == 0xc1 && m_vreg[0x4] == 0xc0 && m_vreg[0x5] == 0xff)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	flags = (m_vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != m_flipscreen)
	{
		m_flipscreen = flags;
		machine().tilemap().mark_all_dirty();
	}

	switch (m_vreg[0x3])
	{
		case 0xb7: width = 54; break;   // mjclinic
		case 0xae: width = 52; break;   // mrokumei
		case 0x9f: width = 49; break;   // hourouki, mhgaiden, mjhokite
		case 0x96: width = 49; break;   // mjclinic
		default:
			if (m_vreg[0x3])
				popmessage("unknown video control %02x %02x %02x %02x",
					m_vreg[0x3],
					m_vreg[0x4],
					m_vreg[0x5],
					m_vreg[0x6]);
			width = 54;
			break;
	}
	screen.set_visible_area(0*8, width*8-1, 2*8, 30*8-1);

	m_bg_tilemap[m_visible_page][0]->set_scrollx(0, m_vreg[0xc] << 1);

	m_bg_tilemap[m_visible_page][0]->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap[m_visible_page][1]->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

#ifdef UNUSED_FUNCTION
UINT32 homedata_state::screen_update_reikaids(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flags;
	static const int pritable[8][4] =
	{
		{ 3,1,0,2 },
		{ 1,3,0,2 },
		{ 0,3,1,2 },
		{ 0,1,3,2 },
		{ 3,0,1,2 },
		{ 1,0,3,2 },
		{ 2,3,1,0 },    // (bg color should be taken from 1)
		{ 3,1,2,0 },    // (bg color should be taken from 1)
	};
	int pri, i;


	flags = (m_vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != m_flipscreen)
	{
		m_flipscreen = flags;
		machine().tilemap().mark_all_dirty();
	}


	bitmap.fill(m_palette->black_pen(), cliprect);

	pri = (m_bank & 0x70) >> 4;
	for (i = 0; i < 4; i++)
		m_bg_tilemap[m_visible_page][pritable[pri][3 - i]]->draw(bitmap, cliprect, 0, 0);
	return 0;
}
#endif

UINT32 homedata_state::screen_update_reikaids(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flags;
	static const int pritable[2][8][4] =    /* table of priorities derived from the PROM */
	{
	{
		{ 3,1,0,2 },
		{ 1,3,0,2 },
		{ 0,3,1,2 },
		{ 0,1,3,2 },
		{ 3,0,1,2 },
		{ 1,0,3,2 },
		{ 2,3,1,0 },    // (bg color should be taken from 1)
		{ 3,1,2,0 } // (bg color should be taken from 1)
	},
	{
		{2,3,0,1},
		{2,0,3,1},
		{3,0,2,1},
		{0,3,2,1},
		{3,0,1,2},
		{2,1,3,0},
		{0,2,3,1},
		{3,2,0,1}
	},
	};

	int pri, i;

	flags = (m_vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != m_flipscreen)
	{
		m_flipscreen = flags;
		machine().tilemap().mark_all_dirty();
	}


	bitmap.fill(m_palette->black_pen(), cliprect);

	pri = (m_blitter_bank & 0x70) >> 4;
	for (i = 0; i < 4; i++)
		m_bg_tilemap[m_visible_page][pritable[m_priority][pri][3 - i]]->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


UINT32 homedata_state::screen_update_pteacher(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flags, scroll_low, scroll_high;


	/* blank screen */
	if (m_vreg[0x3] == 0xc1 && m_vreg[0x4] == 0xc0 && m_vreg[0x5] == 0xff)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	flags = (m_vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != m_flipscreen)
	{
		m_flipscreen = flags;
		machine().tilemap().mark_all_dirty();
	}

	/* bit 2 of blitter_bank stretches characters horizontally by 3/2,
	   so they look as if they were 12x8 instead of 8x8.

	   However, the visible area can be further reduced by fudging with the video
	   registers, but I haven't figured out how they work exactly.
	   In most games it's like this (note that register 4 controls fine
	   scrolling):

	   width      3  4  5  6
	   33*8    = a6 00 ef db (mjikaga)
	   35*8    = bc 0b ef f0
	   51*8    = a6 07 ef db (mjikaga)
	   54*8    = bc 07 ef e8

	   but in mjkinjas it's
	   42*8    = f0 ae df cc

	   note that mjkinjas has a 11MHz xtal instead of the 9MHz of all the others:
	   the two things are probably related, in fact 35 * 11 / 9 = 42.7777777.

	   and it also seems that
	   blanked = c1 c0 ff --
	  */

	if (m_blitter_bank & 0x04)
	{
		if (m_vreg[0x4] == 0xae || m_vreg[0x4] == 0xb8)
		{
			/* kludge for mjkinjas */
			screen.set_visible_area(0*8, 42*8-1, 2*8, 30*8-1);
			scroll_low = 0;
		}
		else
		{
			if (m_vreg[0x3] == 0xa6)
				screen.set_visible_area(0*8, 33*8-1, 2*8, 30*8-1);
			else
				screen.set_visible_area(0*8, 35*8-1, 2*8, 30*8-1);
			scroll_low = (11 - (m_vreg[0x4] & 0x0f)) * 8 / 12;
		}
	}
	else
	{
		if (m_vreg[0x3] == 0xa6)
			screen.set_visible_area(0*8, 51*8-1, 2*8, 30*8-1);
		else
			screen.set_visible_area(0*8, 54*8-1, 2*8, 30*8-1);
		scroll_low = 7 - (m_vreg[0x4] & 0x0f);
	}
	scroll_high = m_vreg[0xb] >> 2;

	m_bg_tilemap[m_visible_page][0]->set_scrollx(0, scroll_high * 8 + scroll_low);
	m_bg_tilemap[m_visible_page][1]->set_scrollx(0, scroll_high * 8 + scroll_low);

	m_bg_tilemap[m_visible_page][0]->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap[m_visible_page][1]->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 homedata_state::screen_update_mirderby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void homedata_state::screen_eof_homedata(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		m_visible_page ^= 1;
	}
}
