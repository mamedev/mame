#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "homedata.h"


UINT8 *homedata_vreg;	/* pointer to RAM associated with 0x7ffx */
UINT8 reikaids_which;
int homedata_visible_page;
int homedata_priority;
static int homedata_flipscreen;

static UINT8	reikaids_gfx_bank[2];
static UINT8	pteacher_gfx_bank;
static UINT8	blitter_bank;
static int		blitter_param_count;
static UINT8	blitter_param[4];		/* buffers last 4 writes to 0x8006 */

static tilemap *bg_tilemap[2][4];


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

static void mrokumei_handleblit( int rom_base )
{
	int i;
	int DestParam;
	int SourceAddr;
	int DestAddr;
	int BaseAddr;
	int opcode,data,NumTiles;
	UINT8 *pBlitData = memory_region(REGION_USER1) + rom_base;

	DestParam =
		blitter_param[(blitter_param_count-4)&3]*256+
		blitter_param[(blitter_param_count-3)&3];

	SourceAddr =
		blitter_param[(blitter_param_count-2)&3]*256+
		blitter_param[(blitter_param_count-1)&3];

	/*  xxx-.----.----.---- not used?
     *  ---x.----.----.---- layer
     *  ----.xxxx.xxxx.xxxx addr
     */
	BaseAddr= (DestParam&0x1000);
	DestAddr= (DestParam&0x0fff);

//  logerror( "[blit bank %02x src %04x dst %04x]\n",blitter_bank,SourceAddr,DestParam);

	if( homedata_visible_page == 0 )
	{
		BaseAddr += 0x2000;
	}

	for(;;)
	{
		opcode = pBlitData[SourceAddr++];
		/* 00xxxxxx RLE incrementing
         * 01xxxxxx Raw Run
         * 1xxxxxxx RLE constant data
         */
		if( opcode == 0x00 )
		{
 			/* end-of-graphic */
 			goto finish;
		}
		data  = pBlitData[SourceAddr++];

		if (opcode&0x80)
			NumTiles = 0x80-(opcode&0x7f);
		else
			NumTiles = 0x40-(opcode&0x3f);

		for( i=0; i<NumTiles; i++ )
		{
			if( i!=0 )
			{
				switch( opcode&0xc0 )
				{
				case 0x40: // Raw Run
					data  = pBlitData[SourceAddr++];
					break;

				case 0x00: // RLE incrementing
					data++;
					break;
				}
			} /* i!=0 */

			if (data)	/* 00 is a nop */
				mrokumei_videoram_w( BaseAddr + DestAddr, data );

			if (homedata_vreg[1] & 0x80)	/* flip screen */
			{
				DestAddr-=2;
				if (DestAddr < 0) goto finish;
			}
			else
			{
				DestAddr+=2;
				if (DestAddr >= 0x1000) goto finish;
			}
		} /* for( i=0; i<NumTiles; i++ ) */
	} /* for(;;) */

finish:
	cpunum_set_input_line(0,M6809_FIRQ_LINE,HOLD_LINE);
}

static void reikaids_handleblit( int rom_base )
{
	int i;
	UINT16 DestParam;
	int flipx;
	int SourceAddr, BaseAddr;
	int DestAddr;
	UINT8 *pBlitData = memory_region(REGION_USER1) + rom_base;

	int opcode,data,NumTiles;

	DestParam =
		blitter_param[(blitter_param_count-4)&3]*256+
		blitter_param[(blitter_param_count-3)&3];

	SourceAddr =
		blitter_param[(blitter_param_count-2)&3]*256+
		blitter_param[(blitter_param_count-1)&3];

	/*  x---.----.----.---- flipx
     *  -x--.----.----.---- select: attr/tile
     *  --*x.xxxx.*xxx.xxxx addr
     */
	BaseAddr= (DestParam&0x4000);
	DestAddr= (DestParam&0x3fff);
	flipx	= (DestParam&0x8000);

//  logerror( "[blit %02x %04x %04x]\n",blitter_bank,SourceAddr,DestParam);

	if( homedata_visible_page == 0 )
	{
		BaseAddr += 0x2000<<2;
	}

	for(;;)
	{
		opcode = pBlitData[SourceAddr++];
		/* 00xxxxxx Raw Run
         * 01xxxxxx RLE incrementing
         * 1xxxxxxx RLE constant data
         */
		if( opcode == 0x00 )
		{
			/* end-of-graphic */
			goto finish;
		}

		data  = pBlitData[SourceAddr++];

		if( (opcode&0xc0)==0x80 )
			NumTiles = 0x80 - (opcode&0x7f);
		else
			NumTiles = 0x40 - (opcode&0x3f);

		for( i=0; i<NumTiles; i++ )
		{
			if( i!=0 )
			{
				switch( opcode&0xc0 )
				{
				case 0x00: // Raw Run
					data  = pBlitData[SourceAddr++];
					break;

				case 0x40: // RLE incrementing
					data++;
					break;
				}
			} /* i!=0 */

			if (data)	/* 00 is a nop */
			{
				int addr = BaseAddr + (DestAddr & 0x3fff);
				int dat = data;

				if ((addr & 0x2080) == 0)
				{
					addr = ((addr & 0xc000) >> 2) | ((addr & 0x1f00) >> 1) | (addr & 0x7f);

					if( flipx )
					{
						if ((BaseAddr & 0x4000) == 0) dat |= 0x80;
						addr ^= 0x007c;
					}

					reikaids_videoram_w( addr, dat );
				}
			}

			if (homedata_vreg[1] & 0x80)	/* flip screen */
				DestAddr-=4;
			else
				DestAddr+=4;
		} /* for( i=0; i<NumTiles; i++ ) */
	}

finish:
	cpunum_set_input_line(0,M6809_FIRQ_LINE,HOLD_LINE);
}

static void pteacher_handleblit( int rom_base )
{
	int i;
	int DestParam;
	int SourceAddr;
	int DestAddr, BaseAddr;
	int opcode,data,NumTiles;
	UINT8 *pBlitData = memory_region(REGION_USER1) + rom_base;

	DestParam =
		blitter_param[(blitter_param_count-4)&3]*256+
		blitter_param[(blitter_param_count-3)&3];

	SourceAddr =
		blitter_param[(blitter_param_count-2)&3]*256+
		blitter_param[(blitter_param_count-1)&3];

	/*  x---.----.----.---- not used?
     *  -x--.----.----.---- layer   (0..1)
     *  --*x.xxxx.*xxx.xxxx addr
     */
	BaseAddr= (DestParam&0x4000);
	DestAddr= (DestParam&0x3fff);

//  logerror( "[blit %02x %04x %04x]->%d\n",blitter_bank,SourceAddr,DestParam,homedata_visible_page);

	if( homedata_visible_page == 0 )
	{
		BaseAddr += 0x2000<<2;
	}

	for(;;)
	{
		opcode = pBlitData[SourceAddr++];
		/* 00xxxxxx Raw Run
         * 01xxxxxx RLE incrementing
         * 1xxxxxxx RLE constant data
         */
		if( opcode == 0x00 )
		{
 			/* end-of-graphic */
 			goto finish;
		}
		data  = pBlitData[SourceAddr++];

		if (opcode & 0x80)
			NumTiles = 0x80-(opcode&0x7f);
		else
			NumTiles = 0x40-(opcode&0x3f);

		for( i=0; i<NumTiles; i++ )
		{
			if( i!=0 )
			{
				switch( opcode&0xc0 )
				{
				case 0x00: // Raw Run
					data  = pBlitData[SourceAddr++];
					break;

				case 0x40: // RLE incrementing
					data++;
					break;
				}
			} /* i!=0 */

			if (data)	/* 00 is a nop */
			{
				int addr = BaseAddr + (DestAddr & 0x3fff);

				if ((addr & 0x2080) == 0)
				{
					addr = ((addr & 0xc000) >> 2) | ((addr & 0x1f00) >> 1) | (addr & 0x7f);
					pteacher_videoram_w( addr, data );
				}
			}

			if (homedata_vreg[1] & 0x80)	/* flip screen */
				DestAddr-=2;
			else
				DestAddr+=2;
		} /* for( i=0; i<NumTiles; i++ ) */
	} /* for(;;) */

finish:
	cpunum_set_input_line(0,M6809_FIRQ_LINE,HOLD_LINE);
}




/***************************************************************************

  Palette setup

***************************************************************************/

PALETTE_INIT( mrokumei )
{
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

		palette_set_color_rgb(machine,i,pal5bit(r),pal5bit(g),pal5bit(b));
	}
}

PALETTE_INIT( reikaids )
{
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

		palette_set_color_rgb(machine,i,pal5bit(r),pal5bit(g),pal5bit(b));
	}
}

PALETTE_INIT( pteacher )
{
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

		palette_set_color_rgb(machine,i,pal5bit(r),pal5bit(g),pal5bit(b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void mrokumei_info0( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int gfxbank )
{
	int addr  = tile_index * 2 + 0x2000 * page;
	int attr  = videoram[addr];
	int code  = videoram[addr + 1] + ((attr & 0x03) << 8) + (gfxbank << 10);
	int color = (attr >> 2) + (gfxbank << 6);

	SET_TILE_INFO( 0, code, color, homedata_flipscreen );
}
INLINE void mrokumei_info1( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int gfxbank )
{
	int addr  = tile_index * 2 + 0x1000 + 0x2000 * page;
	int attr  = videoram[addr];
	int code  = videoram[addr + 1] + ((attr & 0x07) << 8) + (gfxbank << 11);
	int color = (attr >> 3) + ((gfxbank & 3) << 6);

	SET_TILE_INFO( 1, code, color, homedata_flipscreen );
}

static TILE_GET_INFO( mrokumei_get_info0_0 ) { mrokumei_info0( machine, tileinfo, tile_index, 0,  blitter_bank & 0x03 ); }
static TILE_GET_INFO( mrokumei_get_info1_0 ) { mrokumei_info0( machine, tileinfo, tile_index, 1,  blitter_bank & 0x03 ); }
static TILE_GET_INFO( mrokumei_get_info0_1 ) { mrokumei_info1( machine, tileinfo, tile_index, 0, (blitter_bank & 0x38) >> 3 ); }
static TILE_GET_INFO( mrokumei_get_info1_1 ) { mrokumei_info1( machine, tileinfo, tile_index, 1, (blitter_bank & 0x38) >> 3 ); }


INLINE void reikaids_info( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int layer, int gfxbank )
{
	int addr  = tile_index * 4 + layer + 0x2000 * page;
	int attr  = videoram[addr];
	int code  = videoram[addr + 0x1000] + ((attr & 0x03) << 8) + (gfxbank << 10);
	int color = (attr & 0x7c) >> 2;
	int flags = homedata_flipscreen;

	if (attr & 0x80) flags ^= TILE_FLIPX;

	SET_TILE_INFO( layer, code, color, flags );
}

	/* reikaids_gfx_bank[0]:
     *      -xxx.x---   layer#1
     *      ----.-xxx   layer#3
     *
     * reikaids_gfx_bank[1]:
     *      xxxx.x---   layer#0
     *      ----.-xxx   layer#2
     */
static TILE_GET_INFO( reikaids_get_info0_0 ) { reikaids_info( machine, tileinfo, tile_index, 0, 0,  (reikaids_gfx_bank[1]>>3)); }
static TILE_GET_INFO( reikaids_get_info1_0 ) { reikaids_info( machine, tileinfo, tile_index, 1, 0,  (reikaids_gfx_bank[1]>>3)); }
static TILE_GET_INFO( reikaids_get_info0_1 ) { reikaids_info( machine, tileinfo, tile_index, 0, 1, ((reikaids_gfx_bank[0]&0x78)>>3)); }
static TILE_GET_INFO( reikaids_get_info1_1 ) { reikaids_info( machine, tileinfo, tile_index, 1, 1, ((reikaids_gfx_bank[0]&0x78)>>3)); }
static TILE_GET_INFO( reikaids_get_info0_2 ) { reikaids_info( machine, tileinfo, tile_index, 0, 2,  (reikaids_gfx_bank[1]&0x7)); }
static TILE_GET_INFO( reikaids_get_info1_2 ) { reikaids_info( machine, tileinfo, tile_index, 1, 2,  (reikaids_gfx_bank[1]&0x7)); }
static TILE_GET_INFO( reikaids_get_info0_3 ) { reikaids_info( machine, tileinfo, tile_index, 0, 3,  (reikaids_gfx_bank[0]&0x7)); }
static TILE_GET_INFO( reikaids_get_info1_3 ) { reikaids_info( machine, tileinfo, tile_index, 1, 3,  (reikaids_gfx_bank[0]&0x7)); }


INLINE void pteacher_info( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int layer, int gfxbank )
{
	int addr  = tile_index * 2 + 0x1000 * layer + 0x2000 * page;
	int attr  = videoram[addr];
	int code  = videoram[addr + 1] + ((attr & 0x07) << 8) + (gfxbank << 11);
	int color = (attr >> 3) + ((gfxbank & 1) << 5);

	SET_TILE_INFO( layer, code, color, homedata_flipscreen );
}

static TILE_GET_INFO( pteacher_get_info0_0 ) { pteacher_info( machine, tileinfo, tile_index, 0, 0, pteacher_gfx_bank & 0x0f ); }
static TILE_GET_INFO( pteacher_get_info1_0 ) { pteacher_info( machine, tileinfo, tile_index, 1, 0, pteacher_gfx_bank & 0x0f ); }
static TILE_GET_INFO( pteacher_get_info0_1 ) { pteacher_info( machine, tileinfo, tile_index, 0, 1, pteacher_gfx_bank >> 4 ); }
static TILE_GET_INFO( pteacher_get_info1_1 ) { pteacher_info( machine, tileinfo, tile_index, 1, 1, pteacher_gfx_bank >> 4 ); }


INLINE void lemnangl_info( running_machine *machine, tile_data *tileinfo, int tile_index, int page, int layer, int gfxset, int gfxbank )
{
	int addr  = tile_index * 2 + 0x1000 * layer + 0x2000 * page;
	int attr  = videoram[addr];
	int code  = videoram[addr + 1] + ((attr & 0x07) << 8) + (gfxbank << 11);
	int color = 16 * (attr >> 3) + gfxbank;

	SET_TILE_INFO( 2*layer + gfxset, code, color, homedata_flipscreen );
}

static TILE_GET_INFO( lemnangl_get_info0_0 ) { lemnangl_info( machine, tileinfo, tile_index, 0, 0,  blitter_bank & 1,       pteacher_gfx_bank & 0x0f ); }
static TILE_GET_INFO( lemnangl_get_info1_0 ) { lemnangl_info( machine, tileinfo, tile_index, 1, 0,  blitter_bank & 1,       pteacher_gfx_bank & 0x0f ); }
static TILE_GET_INFO( lemnangl_get_info0_1 ) { lemnangl_info( machine, tileinfo, tile_index, 0, 1, (blitter_bank & 2) >> 1, pteacher_gfx_bank >> 4 ); }
static TILE_GET_INFO( lemnangl_get_info1_1 ) { lemnangl_info( machine, tileinfo, tile_index, 1, 1, (blitter_bank & 2) >> 1, pteacher_gfx_bank >> 4 ); }


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( mrokumei )
{
	bg_tilemap[0][0] = tilemap_create( mrokumei_get_info0_0, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64,32 );
	bg_tilemap[0][1] = tilemap_create( mrokumei_get_info0_1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64,32 );
	bg_tilemap[1][0] = tilemap_create( mrokumei_get_info1_0, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64,32 );
	bg_tilemap[1][1] = tilemap_create( mrokumei_get_info1_1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64,32 );

	tilemap_set_transparent_pen(bg_tilemap[0][1],0);
	tilemap_set_transparent_pen(bg_tilemap[1][1],0);
}

VIDEO_START( reikaids )
{
	bg_tilemap[0][0] = tilemap_create( reikaids_get_info0_0, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32 );
	bg_tilemap[0][1] = tilemap_create( reikaids_get_info0_1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32 );
	bg_tilemap[0][2] = tilemap_create( reikaids_get_info0_2, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32 );
	bg_tilemap[0][3] = tilemap_create( reikaids_get_info0_3, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32 );
	bg_tilemap[1][0] = tilemap_create( reikaids_get_info1_0, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32 );
	bg_tilemap[1][1] = tilemap_create( reikaids_get_info1_1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32 );
	bg_tilemap[1][2] = tilemap_create( reikaids_get_info1_2, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32 );
	bg_tilemap[1][3] = tilemap_create( reikaids_get_info1_3, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32 );

	tilemap_set_transparent_pen(bg_tilemap[0][0],0xff);
	tilemap_set_transparent_pen(bg_tilemap[0][1],0xff);
	tilemap_set_transparent_pen(bg_tilemap[0][2],0xff);
	tilemap_set_transparent_pen(bg_tilemap[0][3],0xff);
	tilemap_set_transparent_pen(bg_tilemap[1][0],0xff);
	tilemap_set_transparent_pen(bg_tilemap[1][1],0xff);
	tilemap_set_transparent_pen(bg_tilemap[1][2],0xff);
	tilemap_set_transparent_pen(bg_tilemap[1][3],0xff);
}

VIDEO_START( pteacher )
{
	bg_tilemap[0][0] = tilemap_create( pteacher_get_info0_0, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64,32 );
	bg_tilemap[0][1] = tilemap_create( pteacher_get_info0_1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64,32 );
	bg_tilemap[1][0] = tilemap_create( pteacher_get_info1_0, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64,32 );
	bg_tilemap[1][1] = tilemap_create( pteacher_get_info1_1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64,32 );

	tilemap_set_transparent_pen(bg_tilemap[0][1],0xff);
	tilemap_set_transparent_pen(bg_tilemap[1][1],0xff);
}

VIDEO_START( lemnangl )
{
	bg_tilemap[0][0] = tilemap_create( lemnangl_get_info0_0, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64,32 );
	bg_tilemap[0][1] = tilemap_create( lemnangl_get_info0_1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64,32 );
	bg_tilemap[1][0] = tilemap_create( lemnangl_get_info1_0, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64,32 );
	bg_tilemap[1][1] = tilemap_create( lemnangl_get_info1_1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64,32 );

	tilemap_set_transparent_pen(bg_tilemap[0][1],0x0f);
	tilemap_set_transparent_pen(bg_tilemap[1][1],0x0f);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( mrokumei_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty( bg_tilemap[(offset & 0x2000) >> 13][(offset & 0x1000) >> 12], (offset & 0xffe) >> 1 );
}

WRITE8_HANDLER( reikaids_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty( bg_tilemap[(offset & 0x2000) >> 13][offset & 3], (offset & 0xffc) >> 2 );
}

WRITE8_HANDLER( pteacher_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty( bg_tilemap[(offset & 0x2000) >> 13][(offset & 0x1000) >> 12], (offset & 0xffe) >> 1 );
}

WRITE8_HANDLER( reikaids_gfx_bank_w )
{

//logerror( "%04x: [setbank %02x]\n",activecpu_get_pc(),data);

	if (reikaids_gfx_bank[reikaids_which] != data)
	{
		reikaids_gfx_bank[reikaids_which] = data;
		tilemap_mark_all_tiles_dirty( ALL_TILEMAPS );
	}

	reikaids_which ^= 1;
}

WRITE8_HANDLER( pteacher_gfx_bank_w )
{
//  logerror( "%04x: gfxbank:=%02x\n", activecpu_get_pc(), data );
	if (pteacher_gfx_bank != data)
	{
		pteacher_gfx_bank = data;
		tilemap_mark_all_tiles_dirty( ALL_TILEMAPS );
	}
}

WRITE8_HANDLER( homedata_blitter_param_w )
{
//logerror("%04x: blitter_param_w %02x\n",activecpu_get_pc(),data);
	blitter_param[blitter_param_count] = data;
	blitter_param_count++;
	blitter_param_count&=3;
}

WRITE8_HANDLER( mrokumei_blitter_bank_w )
{
	/* --xxx--- layer 1 gfx bank
       -----x-- blitter ROM bank
       ------xx layer 0 gfx bank
     */

	if ((blitter_bank ^ data) & 0x3b)
		tilemap_mark_all_tiles_dirty( ALL_TILEMAPS);

	blitter_bank = data;
}

WRITE8_HANDLER( reikaids_blitter_bank_w )
{
	/* xxx----- priority control
       ----x--- target page? what's this for?
       ------xx blitter ROM bank
     */
	blitter_bank = data;
}

WRITE8_HANDLER( pteacher_blitter_bank_w )
{
	/* xxx----- blitter ROM bank
       -----x-- pixel clock (normal/slow)
       ------x- layer #1 gfx charset (lemnangl only)
       -------x layer #0 gfx charset (lemnangl only)
     */

	if ((blitter_bank ^ data) & 0x03)
		tilemap_mark_all_tiles_dirty( ALL_TILEMAPS);

	blitter_bank = data;
}

WRITE8_HANDLER( mrokumei_blitter_start_w )
{
	if (data & 0x80) mrokumei_handleblit(((blitter_bank & 0x04) >> 2) * 0x10000);

	/* bit 0 = bank switch; used by hourouki to access the
       optional service mode ROM (not available in current dump) */
}

WRITE8_HANDLER( reikaids_blitter_start_w )
{
	reikaids_handleblit((blitter_bank & 3) * 0x10000);
}

WRITE8_HANDLER( pteacher_blitter_start_w )
{
	pteacher_handleblit((blitter_bank >> 5) * 0x10000 & (memory_region_length(REGION_USER1) - 1));
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( mrokumei )
{
	int flags,width;

	/* blank screen */
	if (homedata_vreg[0x3] == 0xc1 && homedata_vreg[0x4] == 0xc0 && homedata_vreg[0x5] == 0xff)
	{
		fillbitmap(bitmap,get_black_pen(machine),cliprect);
		return 0;
	}

	flags = (homedata_vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != homedata_flipscreen)
	{
		homedata_flipscreen = flags;
		tilemap_mark_all_tiles_dirty( ALL_TILEMAPS );
	}

	switch (homedata_vreg[0x3])
	{
		case 0xb7: width = 54; break;	// mjclinic
		case 0xae: width = 52; break;	// mrokumei
		case 0x9f: width = 49; break;	// hourouki, mhgaiden, mjhokite
		case 0x96: width = 49; break;	// mjclinic
		default:
			if (homedata_vreg[0x3])
				popmessage("unknown video control %02x %02x %02x %02x",
					homedata_vreg[0x3],
					homedata_vreg[0x4],
					homedata_vreg[0x5],
					homedata_vreg[0x6]);
			width = 54;
			break;
	}
	video_screen_set_visarea(0, 0*8, width*8-1, 2*8, 30*8-1);

	tilemap_set_scrollx(bg_tilemap[homedata_visible_page][0],0,homedata_vreg[0xc] << 1);

	tilemap_draw(bitmap, cliprect, bg_tilemap[homedata_visible_page][0], 0, 0);
	tilemap_draw(bitmap, cliprect, bg_tilemap[homedata_visible_page][1], 0, 0);
	return 0;
}
/*
VIDEO_UPDATE( reikaids )
{
    int flags;
    static int pritable[8][4] =
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
    int pri,i;


    flags = (homedata_vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
    if (flags != homedata_flipscreen)
    {
        homedata_flipscreen = flags;
        tilemap_mark_all_tiles_dirty( ALL_TILEMAPS );
    }


    fillbitmap(bitmap,get_black_pen(machine),cliprect);

    pri = (blitter_bank & 0x70) >> 4;
    for (i = 0;i < 4;i++)
        tilemap_draw(bitmap, cliprect, bg_tilemap[homedata_visible_page][pritable[pri][3-i]], 0, 0);
    return 0;
}

*/
VIDEO_UPDATE( reikaids )
{
	int flags;
	static int pritable[2][8][4] =	/* table of priorities derived from the PROM */
	{
	{
		{ 3,1,0,2 },
		{ 1,3,0,2 },
		{ 0,3,1,2 },
		{ 0,1,3,2 },
		{ 3,0,1,2 },
		{ 1,0,3,2 },
		{ 2,3,1,0 },	// (bg color should be taken from 1)
		{ 3,1,2,0 }	// (bg color should be taken from 1)
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

	int pri,i;

	flags = (homedata_vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != homedata_flipscreen)
	{
		homedata_flipscreen = flags;
		tilemap_mark_all_tiles_dirty( ALL_TILEMAPS );
	}


	fillbitmap(bitmap,get_black_pen(machine),cliprect);

	pri = (blitter_bank & 0x70) >> 4;
	for (i = 0;i < 4;i++)
		tilemap_draw(bitmap, cliprect, bg_tilemap[homedata_visible_page][pritable[homedata_priority][pri][3-i]], 0, 0);
	return 0;
}


VIDEO_UPDATE( pteacher )
{
	int flags,scroll_low,scroll_high;


	/* blank screen */
	if (homedata_vreg[0x3] == 0xc1 && homedata_vreg[0x4] == 0xc0 && homedata_vreg[0x5] == 0xff)
	{
		fillbitmap(bitmap,get_black_pen(machine),cliprect);
		return 0;
	}

	flags = (homedata_vreg[1] & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0;
	if (flags != homedata_flipscreen)
	{
		homedata_flipscreen = flags;
		tilemap_mark_all_tiles_dirty( ALL_TILEMAPS );
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

	if (blitter_bank & 0x04)
	{
		if (homedata_vreg[0x4] == 0xae || homedata_vreg[0x4] == 0xb8)
		{
			/* kludge for mjkinjas */
			video_screen_set_visarea(0, 0*8, 42*8-1, 2*8, 30*8-1);
			scroll_low = 0;
		}
		else
		{
			if (homedata_vreg[0x3] == 0xa6)
				video_screen_set_visarea(0, 0*8, 33*8-1, 2*8, 30*8-1);
			else
				video_screen_set_visarea(0, 0*8, 35*8-1, 2*8, 30*8-1);
			scroll_low = (11 - (homedata_vreg[0x4] & 0x0f)) * 8 / 12;
		}
	}
	else
	{
		if (homedata_vreg[0x3] == 0xa6)
			video_screen_set_visarea(0, 0*8, 51*8-1, 2*8, 30*8-1);
		else
			video_screen_set_visarea(0, 0*8, 54*8-1, 2*8, 30*8-1);
		scroll_low = 7 - (homedata_vreg[0x4] & 0x0f);
	}
	scroll_high = homedata_vreg[0xb] >> 2;

	tilemap_set_scrollx(bg_tilemap[homedata_visible_page][0],0,scroll_high*8 + scroll_low);
	tilemap_set_scrollx(bg_tilemap[homedata_visible_page][1],0,scroll_high*8 + scroll_low);

	tilemap_draw(bitmap, cliprect, bg_tilemap[homedata_visible_page][0], 0, 0);
	tilemap_draw(bitmap, cliprect, bg_tilemap[homedata_visible_page][1], 0, 0);
	return 0;
}


VIDEO_EOF( homedata )
{
	homedata_visible_page ^= 1;
}
