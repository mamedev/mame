// license:BSD-3-Clause
// copyright-holders:David Haywood
/* this should probably be k054156.c and k054156_device (the base management device) */

/***************************************************************************/
/*                                                                         */
/*    054156 with                                                          */
/*    either 054157 (max 5bpp) or 056832 (max 8bpp)                        */
/*    Konami Tilemap Chips                                                 */
/*                                                                         */
/*    058143 == 054156 on newer GX boards (does it integrate the 056832?)  */
/*                                                                         */
/***************************************************************************/

/*

054156/054157
054156/056832
-------------

[Except for tilemap sizes, all numbers are in hex]

These work in pairs.  Similar in principle to the 052109/051962, they
manage 4 64x32 or 64x64 tilemaps.  They also handle linescroll on each
layer, and optional tile banking.  They use 4000 to 10000 bytes of
RAM, organized in 1000 or 2000 bytes banks.

The 56832 is a complete superset of the 54157 and supports higher color
depths (the 156/157 combo only goes to 5 bpp, the 156/832 combo goes to 8bpp).

These chips work in a fairly unusual way.  There are 4, 8, or 16 pages of VRAM, arranged
conceptually in a 4x4 2 dimensional grid.  Each page is a complete 64x32 tile tilemap.

The 4 physical tilemaps A, B, C, and, D are made up of these pages "glued together".
Each physical tilemap has an X and Y position in the 4x4 page grid indicating where
the page making up it's upper left corner is, as well as a width and height in pages.
If two tilemaps try to use the same page, the higher-letter one wins and the lower-letter
one is disabled completely.  E.g. A > B > C > D, so if A and B both try to use the
same page only A will be displayed.  Some games rely on this behavior to implicitly
disable tilemaps which otherwise should be displayed.

Tile encoding 2 bytes/tile (banks of 1000 bytes):
        pppx bbcc cccc cccc
  p = color palette
  x = flip x
  b = tile bank (0..3)
  c = tile code (0..3ff)


Tile encoding 4 bytes/tile (banks of 2000 bytes):
        ---- ---- pppp --yx  cccc cccc cccc cccc
  p = color palette
  x = flip x
  y = flip y
  b = tile bank (0..3)
  c = tile code (0..3ff)


Communication with these ics go through 4 memory zones:
  1000/2000 bytes: access to the currently selected ram bank
       2000 bytes: readonly access to the currently selected tile
                   rom bank for rom checksumming
         40 bytes: writeonly access to the first register bank
          8 bytes: writeonly access to the second register bank

One of the register banks is probably on the 054156, and the other on
the 054157.

First register bank map (offsets in bytes, '-' means unused):
00    ---- ---- ??yx ????
  flip control

02    ---- ---- ???? ????
  unknown

04    ---- ---- ???? ????
  unknown (bit 1 may be bank count selection, 0 in xexex, 1 everywhere
  else)

06    ---- ---- ???? ???e
  enable irq

08    ---- ---- ???? ????
  unknown

0a    ---- ---- 3322 1100
  linescroll control, each pair of bits indicates the mode for the
  corresponding layer:
    0: per-line linescroll
    1: unused/unknown
    2: per-8 lines linescroll
    3: no linescroll

0c    ---- ---- ???? ????
  unknown (bit 1 may be bank size selection, 1 in asterix, 0 everywhere
  else)

0e    ---- ---- ---- ----

10-13 ---- ---- ---y y-hh
   layer Y position in the VRAM grid and height in pages

14-17 ---- ---- ---x x-ww
   layer X position in the VRAM grid and width in pages
18-1f ---- ---- ???? ????

20-27 yyyy yyyy yyyy yyyy
  scroll y position for each layer

28-2f xxxx xxxx xxxx xxxx
  scroll x position for each layer

30    ---- ---- ---b b--b
  linescroll ram bank selection

32    ---- ---- ---b b--b
  cpu-accessible ram bank selection

34    bbbb bbbb bbbb bbbb
  rom bank selection for checksumming (each bank is 0x2000 bytes)

36    ---- ---- ---- bbbb
  secondary rom bank selection for checksumming when tile banking is
  used

38    3333 2222 1111 0000
  tile banking look up table.  4 bits are looked up here for the two
  bits in the tile data.

3a    ???? ???? ???? ????
  unknown

3c    ???? ???? ???? ????
  unknown

3e    ---- ---- ---- ----


Second register bank map:
00    ---- ---- ???? ????
  unknown

02-07 are copies of the 02-07 registers from the first bank.


  Linescroll:

The linescroll is controlled by the register 0b, and uses the data in
the ram bank pointed by register 31.  The data for tilemap <n> starts
at offset 400*n in the bank for 1000 bytes ram banks, and 800*n+2 for
2000 bytes ram banks.  The scrolling information is a vector of half
words separated by 1 word padding for 2000 bytes banks.

This is a source-oriented linescroll, i.e. the first word is
associated to the first one of the tilemap, not matter what the
current scrolly position is.

In per-line mode, each word indicates the horizontal scroll of the
associated line.  Global scrollx is ignored.

In per-8 lines mode, each word associated to a line multiple of 8
indicates the horizontal scroll for that line and the 7 following
ones.  The other 7 words are ignored.  Global scrollx is ignored.

*/

#include "emu.h"
#include "k054156_k054157_k056832.h"
#include "konami_helper.h"


/* end common functions */

#define K056832_PAGE_COLS 64
#define K056832_PAGE_ROWS 32
#define K056832_PAGE_HEIGHT (K056832_PAGE_ROWS*8)
#define K056832_PAGE_WIDTH  (K056832_PAGE_COLS*8)




const device_type K056832 = &device_creator<k056832_device>;

k056832_device::k056832_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K056832, "K056832 Tilemap Generator", tag, owner, clock, "k056832", __FILE__),
	//m_tilemap[K056832_PAGE_COUNT],
	//*m_pixmap[K056832_PAGE_COUNT],
	//m_regs[0x20],
	//m_regsb[4],
	m_rombase(nullptr),
	m_num_gfx_banks(0),
	m_cur_gfx_banks(0),
	m_gfx_memory_region(nullptr),
	m_gfx_num(0),
	m_bpp(-1),
	m_big(0),
	m_djmain_hack(0),
	m_k055555_tag(nullptr),
	//m_layer_assoc_with_page[K056832_PAGE_COUNT],
	//m_layer_offs[8][2],
	//m_lsram_page[8][2],
	//m_x[8],
	//m_y[8],
	//m_w[8],
	//m_h[8],
	//m_dx[8],
	//m_dy[8],
	//m_line_dirty[K056832_PAGE_COUNT][8],
	//m_all_lines_dirty[K056832_PAGE_COUNT],
	//m_page_tile_mode[K056832_PAGE_COUNT],
	//m_last_colorbase[K056832_PAGE_COUNT],
	//m_layer_tile_mode[8],
	m_default_layer_association(0),
	m_layer_association(0),
	m_active_layer(0),
	m_selected_page(0),
	m_selected_page_x4096(0),
	m_linemap_enabled(0),
	m_use_ext_linescroll(0),
	m_uses_tile_banks(0),
	m_cur_tile_bank(0),
	m_k055555(nullptr),
	m_gfxdecode(*this),
	m_palette(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void k056832_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<k056832_device &>(device).m_gfxdecode.set_tag(tag);
}


//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void k056832_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<k056832_device &>(device).m_palette.set_tag(tag);
}

void k056832_device::create_tilemaps()
{
	tilemap_t *tmap;
	int i;

	for (i = 0; i < 8; i++)
	{
		m_layer_offs[i][0] = 0;
		m_layer_offs[i][1] = 0;
		m_lsram_page[i][0] = i;
		m_lsram_page[i][1] = i << 11;
		m_x[i] = 0;
		m_y[i] = 0;
		m_w[i] = 0;
		m_h[i] = 0;
		m_dx[i] = 0;
		m_dy[i] = 0;
		m_layer_tile_mode[i] = 1;
	}

	m_default_layer_association = 1;
	m_active_layer = 0;
	m_linemap_enabled = 0;


	memset(m_line_dirty, 0, sizeof(UINT32) * K056832_PAGE_COUNT * 8);

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		m_all_lines_dirty[i] = 0;
		m_page_tile_mode[i] = 1;
	}



	m_videoram.resize(0x2000 * (K056832_PAGE_COUNT + 1) / 2);
	memset(&m_videoram[0], 0, 2*m_videoram.size());

	m_tilemap[0x0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info0),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0x1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info1),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0x2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info2),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0x3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info3),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0x4] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info4),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0x5] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info5),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0x6] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info6),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0x7] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info7),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0x8] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info8),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0x9] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_info9),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0xa] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_infoa),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0xb] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_infob),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0xc] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_infoc),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0xd] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_infod),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0xe] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_infoe),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_tilemap[0xf] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k056832_device::get_tile_infof),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		tmap = m_tilemap[i];

		m_pixmap[i] = &tmap->pixmap();

		tmap->set_transparent_pen(0);
	}
}


void k056832_device::finalize_init()
{
	int i;

	update_page_layout();

	change_rambank();
	change_rombank();

	save_item(NAME(m_videoram));
	save_item(NAME(m_regs));
	save_item(NAME(m_regsb));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_w));
	save_item(NAME(m_h));
	save_item(NAME(m_dx));
	save_item(NAME(m_dy));
	save_item(NAME(m_layer_tile_mode));

	save_item(NAME(m_default_layer_association));
	save_item(NAME(m_active_layer));
	save_item(NAME(m_linemap_enabled));
	save_item(NAME(m_use_ext_linescroll));
	save_item(NAME(m_uses_tile_banks));
	save_item(NAME(m_cur_tile_bank));
	save_item(NAME(m_rom_half));
	save_item(NAME(m_all_lines_dirty));
	save_item(NAME(m_page_tile_mode));

	for (i = 0; i < 8; i++)
	{
		save_item(NAME(m_layer_offs[i]), i);
		save_item(NAME(m_lsram_page[i]), i);
	}

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		save_item(NAME(m_line_dirty[i]), i);
		save_item(NAME(m_all_lines_dirty[i]), i);
		save_item(NAME(m_page_tile_mode[i]), i);
		save_item(NAME(m_last_colorbase[i]), i);
	}

	machine().save().register_postload(save_prepost_delegate(FUNC(k056832_device::postload), this));
}


void k056832_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	memset(m_regs,     0x00, sizeof(m_regs) );
	memset(m_regsb,    0x00, sizeof(m_regsb) );

	if (m_k055555_tag)
		m_k055555 = machine().device<k055555_device>(m_k055555_tag);

/* TODO: understand which elements MUST be init here (to keep correct layer
   associations) and which ones can can be init at RESET, if any */

	create_gfx(m_gfx_memory_region, m_bpp, m_big);

	create_tilemaps();

	finalize_init();

	// bind callbacks
	m_k056832_cb.bind_relative_to(*owner());
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

#define mark_line_dirty(P, L) if (L < 0x100) m_line_dirty[P][L >> 5] |= 1 << (L & 0x1f)
#define mark_all_lines_dirty(P) m_all_lines_dirty[P] = 1

void k056832_device::mark_page_dirty( int page )
{
	if (m_page_tile_mode[page])
		m_tilemap[page]->mark_all_dirty();
	else
		mark_all_lines_dirty(page);
}


void k056832_device::mark_plane_dirty( int layer )
{
	int tilemode, i;

	tilemode = m_layer_tile_mode[layer];

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		if (m_layer_assoc_with_page[i] == layer)
		{
			m_page_tile_mode[i] = tilemode;
			mark_page_dirty(i);
		}
	}
}




void k056832_device::mark_all_tilemaps_dirty( )
{
	int i;

	for (i = 0; i < K056832_PAGE_COUNT; i++)
	{
		if (m_layer_assoc_with_page[i] != -1)
		{
			m_page_tile_mode[i] = m_layer_tile_mode[m_layer_assoc_with_page[i]];
			mark_page_dirty(i);
		}
	}
}

void k056832_device::update_page_layout( )
{
	int layer, rowstart, rowspan, colstart, colspan, r, c, page_idx, setlayer;

	// enable layer association by default
	m_layer_association = m_default_layer_association;

	// disable association if a layer grabs the entire 4x4 map (happens in Twinbee and Dadandarn)
	for (layer = 0; layer < 4; layer++)
	{
		if (!m_y[layer] && !m_x[layer] && m_h[layer] == 3 && m_w[layer] == 3)
		{
			m_layer_association = 0;
			break;
		}
	}

	// winning spike doesn't like layer association..
	if (m_djmain_hack == 2)
		m_layer_association = 0;

	// disable all tilemaps
	for (page_idx = 0; page_idx < K056832_PAGE_COUNT; page_idx++)
	{
		m_layer_assoc_with_page[page_idx] = -1;
	}


	// enable associated tilemaps
	for (layer = 0; layer < 4; layer++)
	{
		rowstart = m_y[layer];
		colstart = m_x[layer];
		rowspan  = m_h[layer] + 1;
		colspan  = m_w[layer] + 1;

		setlayer = (m_layer_association) ? layer : m_active_layer;

		for (r = 0; r < rowspan; r++)
		{
			for (c = 0; c < colspan; c++)
			{
				page_idx = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);
				if (!(m_djmain_hack==1) || m_layer_assoc_with_page[page_idx] == -1)
					m_layer_assoc_with_page[page_idx] = setlayer;
			}
		}
	}

	// refresh associated tilemaps
	mark_all_tilemaps_dirty();
}




int k056832_device::get_lookup( int bits )
{
	int res;

	res = (m_regs[0x1c] >> (bits << 2)) & 0x0f;

	if (m_uses_tile_banks)   /* Asterix */
		res |= m_cur_tile_bank << 4;

	return res;
}

void k056832_device::get_tile_info(  tile_data &tileinfo, int tile_index, int pageIndex )
{
	static const struct K056832_SHIFTMASKS
	{
		int flips, palm1, pals2, palm2;
	}
	k056832_shiftmasks[4] = {{6, 0x3f, 0, 0x00}, {4, 0x0f, 2, 0x30}, {2, 0x03, 2, 0x3c}, {0, 0x00, 2, 0x3f}};

	const struct K056832_SHIFTMASKS *smptr;
	int layer, flip, fbits, attr, code, color, flags;
	UINT16 *pMem;

	pMem  = &m_videoram[(pageIndex << 12) + (tile_index << 1)];

	if (m_layer_association)
	{
		layer = m_layer_assoc_with_page[pageIndex];
		if (layer == -1)
			layer = 0;  // use layer 0's palette info for unmapped pages
	}
	else
		layer = m_active_layer;

	fbits = (m_regs[3] >> 6) & 3;
	flip  = (m_regs[1] >> (layer << 1)) & 0x3; // tile-flip override (see p.20 3.2.2 "REG2")
	smptr = &k056832_shiftmasks[fbits];
	attr  = pMem[0];
	code  = pMem[1];

	// normalize the flip/palette flags
	// see the tables on pages 4 and 10 of the Pt. 2-3 "VRAM" manual
	// for a description of these bits "FBIT0" and "FBIT1"
	flip &= attr >> smptr->flips & 3;
	color = (attr & smptr->palm1) | (attr >> smptr->pals2 & smptr->palm2);
	flags = TILE_FLIPYX(flip);

	m_k056832_cb(layer, &code, &color, &flags);

	SET_TILE_INFO_MEMBER(m_gfx_num,
			code,
			color,
			flags);
}



TILE_GET_INFO_MEMBER( k056832_device::get_tile_info0 ) { get_tile_info(tileinfo,tile_index,0x0); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_info1 ) { get_tile_info(tileinfo,tile_index,0x1); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_info2 ) { get_tile_info(tileinfo,tile_index,0x2); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_info3 ) { get_tile_info(tileinfo,tile_index,0x3); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_info4 ) { get_tile_info(tileinfo,tile_index,0x4); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_info5 ) { get_tile_info(tileinfo,tile_index,0x5); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_info6 ) { get_tile_info(tileinfo,tile_index,0x6); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_info7 ) { get_tile_info(tileinfo,tile_index,0x7); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_info8 ) { get_tile_info(tileinfo,tile_index,0x8); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_info9 ) { get_tile_info(tileinfo,tile_index,0x9); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_infoa ) { get_tile_info(tileinfo,tile_index,0xa); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_infob ) { get_tile_info(tileinfo,tile_index,0xb); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_infoc ) { get_tile_info(tileinfo,tile_index,0xc); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_infod ) { get_tile_info(tileinfo,tile_index,0xd); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_infoe ) { get_tile_info(tileinfo,tile_index,0xe); }
TILE_GET_INFO_MEMBER( k056832_device::get_tile_infof ) { get_tile_info(tileinfo,tile_index,0xf); }


void k056832_device::change_rambank( )
{
	/* ------xx page col
	 * ---xx--- page row
	 */
	int bank = m_regs[0x19];

	if (m_regs[0] & 0x02)    // external linescroll enable
		m_selected_page = K056832_PAGE_COUNT;
	else
		m_selected_page = ((bank >> 1) & 0xc) | (bank & 3);

	m_selected_page_x4096 = m_selected_page << 12;

	// refresh associated tilemaps
	mark_all_tilemaps_dirty();
}





int k056832_device::get_current_rambank( )
{
	int bank = m_regs[0x19];

	return ((bank >> 1) & 0xc) | (bank & 3);
}

void k056832_device::change_rombank( )
{
	int bank;

	if (m_uses_tile_banks)   /* Asterix */
		bank = (m_regs[0x1a] >> 8) | (m_regs[0x1b] << 4) | (m_cur_tile_bank << 6);
	else
		bank = m_regs[0x1a] | (m_regs[0x1b] << 16);

	m_cur_gfx_banks = bank % m_num_gfx_banks;
}




void k056832_device::set_tile_bank( int bank )
{
	m_uses_tile_banks = 1;

	if (m_cur_tile_bank != bank)
	{
		m_cur_tile_bank = bank;

		mark_plane_dirty(0);
		mark_plane_dirty(1);
		mark_plane_dirty(2);
		mark_plane_dirty(3);
	}

	change_rombank();
}

/* call if a game uses external linescroll */
void k056832_device::SetExtLinescroll( )
{
	m_use_ext_linescroll = 1;
}

/* generic helper routine for ROM checksumming */
int k056832_device::rom_read_b( int offset, int blksize, int blksize2, int zerosec )
{
	UINT8 *rombase;
	int base, ret;

	rombase = (UINT8 *)machine().root_device().memregion(m_gfx_memory_region)->base();

	if ((m_rom_half) && (zerosec))
	{
		return 0;
	}

	// add in the bank offset
	offset += (m_cur_gfx_banks * 0x2000);

	// figure out the base of the ROM block
	base = (offset / blksize) * blksize2;

	// get the starting offset of the proper word inside the block
	base += (offset % blksize) * 2;

	if (m_rom_half)
	{
		ret = rombase[base + 1];
	}
	else
	{
		ret = rombase[base];
		m_rom_half = 1;
	}

	return ret;
}


READ16_MEMBER( k056832_device::k_5bpp_rom_word_r )
{
	if (mem_mask == 0xff00)
		return rom_read_b(offset * 2, 4, 5, 0)<<8;
	else if (mem_mask == 0x00ff)
		return rom_read_b(offset * 2 + 1, 4, 5, 0)<<16;
	else
	{
		//LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", space.device().safe_pc(), mem_mask));
	}
	return 0;
}

READ32_MEMBER( k056832_device::k_5bpp_rom_long_r )
{
	if (mem_mask == 0xff000000)
		return rom_read_b(offset * 4, 4, 5, 0) << 24;
	else if (mem_mask == 0x00ff0000)
		return rom_read_b(offset * 4 + 1, 4, 5, 0) << 16;
	else if (mem_mask == 0x0000ff00)
		return rom_read_b(offset * 4 + 2, 4, 5, 0) << 8;
	else if (mem_mask == 0x000000ff)
		return rom_read_b(offset * 4 + 3, 4, 5, 1);
	else
	{
		//LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", space.device().safe_pc(), mem_mask));
	}
	return 0;
}

READ32_MEMBER( k056832_device::k_6bpp_rom_long_r )
{
	if (mem_mask == 0xff000000)
		return rom_read_b(offset * 4, 4, 6, 0) << 24;
	else if (mem_mask == 0x00ff0000)
		return rom_read_b(offset * 4 + 1, 4, 6, 0) << 16;
	else if (mem_mask == 0x0000ff00)
		return rom_read_b(offset * 4 + 2, 4, 6, 0) << 8;
	else if (mem_mask == 0x000000ff)
		return rom_read_b(offset * 4 + 3, 4, 6, 0);
	else
	{
		//LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", space.device().safe_pc(), mem_mask));
	}
	return 0;
}




READ16_MEMBER( k056832_device::rom_word_r )
{
	int addr = 0x2000 * m_cur_gfx_banks + 2 * offset;

	if (!m_rombase)
		m_rombase = space.machine().root_device().memregion(m_gfx_memory_region)->base();

	return m_rombase[addr + 1] | (m_rombase[addr] << 8);
}

// data is arranged like this:
// 0000 1111 22 0000 1111 22
READ16_MEMBER( k056832_device::mw_rom_word_r )
{
	int bank = 10240 * m_cur_gfx_banks;
	int addr;

	if (!m_rombase)
		m_rombase = space.machine().root_device().memregion(m_gfx_memory_region)->base();

	if (m_regsb[2] & 0x8)
	{
		// we want only the 2s
		int bit;
		int res, temp;

		bit = offset % 4;
		addr = (offset / 4) * 5;

		temp = m_rombase[addr + 4 + bank];

		switch (bit)
		{
			default:
			case 0:
				res = (temp & 0x80) << 5;
				res |= ((temp & 0x40) >> 2);
				break;

			case 1:
				res = (temp & 0x20) << 7;
				res |= (temp & 0x10);
				break;

			case 2:
				res = (temp & 0x08) << 9;
				res |= ((temp & 0x04) << 2);
				break;

			case 3:
				res = (temp & 0x02) << 11;
				res |= ((temp & 0x01) << 4);
				break;
		}

		return res;
	}
	else
	{
		// we want only the 0s and 1s.

		addr = (offset >> 1) * 5;

		if (offset & 1)
		{
			addr += 2;
		}

		addr += bank;

		return m_rombase[addr + 1] | (m_rombase[addr] << 8);
	}

}

READ16_MEMBER( k056832_device::bishi_rom_word_r )
{
	int addr = 0x4000 * m_cur_gfx_banks + offset;

	if (!m_rombase)
		m_rombase = space.machine().root_device().memregion(m_gfx_memory_region)->base();

	return m_rombase[addr + 2] | (m_rombase[addr] << 8);
}

READ16_MEMBER( k056832_device::rom_word_8000_r )
{
	int addr = 0x8000 * m_cur_gfx_banks + 2 * offset;

	if (!m_rombase)
		m_rombase = space.machine().root_device().memregion(m_gfx_memory_region)->base();

	return m_rombase[addr + 2] | (m_rombase[addr] << 8);
}

READ16_MEMBER( k056832_device::old_rom_word_r )
{
	int addr = 0x2000 * m_cur_gfx_banks + 2 * offset;

	if (!m_rombase)
		m_rombase = space.machine().root_device().memregion(m_gfx_memory_region)->base();

	return m_rombase[addr + 1] | (m_rombase[addr] << 8);
}

READ32_MEMBER( k056832_device::rom_long_r )
{
	offset <<= 1;
	return (rom_word_r(space, offset + 1, 0xffff) | (rom_word_r(space, offset, 0xffff) << 16));
}

/* only one page is mapped to videoram at a time through a window */
READ16_MEMBER( k056832_device::ram_word_r )
{
	// reading from tile RAM resets the ROM readback "half" offset
	m_rom_half = 0;

	return m_videoram[m_selected_page_x4096 + offset];
}

READ16_MEMBER( k056832_device::ram_half_word_r )
{
	return m_videoram[m_selected_page_x4096 + (((offset << 1) & 0xffe) | ((offset >> 11) ^ 1))];
}





READ32_MEMBER( k056832_device::ram_long_r )
{
	UINT16 *pMem = &m_videoram[m_selected_page_x4096 + offset * 2];

	// reading from tile RAM resets the ROM readback "half" offset
	m_rom_half = 0;

	return (pMem[0]<<16 | pMem[1]);
}

READ32_MEMBER( k056832_device::unpaged_ram_long_r )
{
	UINT16 *pMem = &m_videoram[offset * 2];

	// reading from tile RAM resets the ROM readback "half" offset
	m_rom_half = 0;

	return (pMem[0]<<16 | pMem[1]);
}

/* special 8-bit handlers for Lethal Enforcers */
READ8_MEMBER( k056832_device::ram_code_lo_r )
{
	UINT16 *adr = &m_videoram[m_selected_page_x4096 + (offset * 2) + 1];

	return *adr & 0xff;
}

READ8_MEMBER( k056832_device::ram_code_hi_r )
{
	UINT16 *adr = &m_videoram[m_selected_page_x4096 + (offset * 2) + 1];

	return *adr >> 8;
}

READ8_MEMBER( k056832_device::ram_attr_lo_r )
{
	UINT16 *adr = &m_videoram[m_selected_page_x4096 + (offset * 2)];

	return *adr & 0xff;
}

READ8_MEMBER( k056832_device::ram_attr_hi_r )
{
	UINT16 *adr = &m_videoram[m_selected_page_x4096 + (offset * 2)];

	return *adr >> 8;
}

WRITE8_MEMBER( k056832_device::ram_code_lo_w )
{
	UINT16 *adr = &m_videoram[m_selected_page_x4096 + (offset * 2) + 1];

	*adr &= 0xff00;
	*adr |= data;

	if (!(m_regs[0] & 0x02)) // external linescroll enable
	{
		if (m_page_tile_mode[m_selected_page])
			m_tilemap[m_selected_page]->mark_tile_dirty(offset);
		else
			mark_line_dirty(m_selected_page, offset);
	}
}

WRITE8_MEMBER( k056832_device::ram_code_hi_w )
{
	UINT16 *adr = &m_videoram[m_selected_page_x4096 + (offset * 2) + 1];

	*adr &= 0x00ff;
	*adr |= data << 8;

	if (!(m_regs[0] & 0x02)) // external linescroll enable
	{
		if (m_page_tile_mode[m_selected_page])
			m_tilemap[m_selected_page]->mark_tile_dirty(offset);
		else
			mark_line_dirty(m_selected_page, offset);
	}
}

WRITE8_MEMBER( k056832_device::ram_attr_lo_w )
{
	UINT16 *adr = &m_videoram[m_selected_page_x4096 + (offset * 2)];

	*adr &= 0xff00;
	*adr |= data;

	if (!(m_regs[0] & 0x02)) // external linescroll enable
	{
		if (m_page_tile_mode[m_selected_page])
			m_tilemap[m_selected_page]->mark_tile_dirty(offset);
		else
			mark_line_dirty(m_selected_page, offset);
	}
}

WRITE8_MEMBER( k056832_device::ram_attr_hi_w )
{
	UINT16 *adr = &m_videoram[m_selected_page_x4096 + (offset * 2)];

	*adr &= 0x00ff;
	*adr |= data << 8;

	if (!(m_regs[0] & 0x02)) // external linescroll enable
	{
		if (m_page_tile_mode[m_selected_page])
			m_tilemap[m_selected_page]->mark_tile_dirty(offset);
		else
			mark_line_dirty(m_selected_page, offset);
	}
}

WRITE16_MEMBER( k056832_device::ram_word_w )
{
	UINT16 *tile_ptr;
	UINT16 old_mask, old_data;

	tile_ptr = &m_videoram[m_selected_page_x4096 + offset];
	old_mask = ~mem_mask;
	old_data = *tile_ptr;
	data = (data & mem_mask) | (old_data & old_mask);

	if(data != old_data)
	{
		offset >>= 1;
		*tile_ptr = data;

		if (m_page_tile_mode[m_selected_page])
			m_tilemap[m_selected_page]->mark_tile_dirty(offset);
		else
			mark_line_dirty(m_selected_page, offset);
	}
}



WRITE16_MEMBER( k056832_device::ram_half_word_w )
{
	UINT16 *adr = &m_videoram[m_selected_page_x4096 + (((offset << 1) & 0xffe) | 1)];
	UINT16 old = *adr;

	COMBINE_DATA(adr);
	if(*adr != old)
	{
		int dofs = (((offset << 1) & 0xffe) | 1);

		dofs >>= 1;

		if (m_page_tile_mode[m_selected_page])
			m_tilemap[m_selected_page]->mark_tile_dirty(dofs);
		else
			mark_line_dirty(m_selected_page, dofs);
	}
}

WRITE32_MEMBER( k056832_device::ram_long_w )
{
	UINT16 *tile_ptr;
	UINT32 old_mask, old_data;

	tile_ptr = &m_videoram[m_selected_page_x4096 + offset * 2];
	old_mask = ~mem_mask;
	old_data = (UINT32)tile_ptr[0] << 16 | (UINT32)tile_ptr[1];
	data = (data & mem_mask) | (old_data & old_mask);

	if (data != old_data)
	{
		tile_ptr[0] = data >> 16;
		tile_ptr[1] = data;

		if (m_page_tile_mode[m_selected_page])
			m_tilemap[m_selected_page]->mark_tile_dirty(offset);
		else
			mark_line_dirty(m_selected_page, offset);
	}
}




WRITE32_MEMBER( k056832_device::unpaged_ram_long_w )
{
	UINT16 *tile_ptr;
	UINT32 old_mask, old_data;

	tile_ptr = &m_videoram[offset * 2];
	old_mask = ~mem_mask;
	old_data = (UINT32)tile_ptr[0] << 16 | (UINT32)tile_ptr[1];
	data = (data & mem_mask) | (old_data & old_mask);

	if (data != old_data)
	{
		tile_ptr[0] = data >> 16;
		tile_ptr[1] = data;

		if (m_page_tile_mode[offset/0x800])
			m_tilemap[offset/0x800]->mark_tile_dirty(offset&0x7ff);
		else
			mark_line_dirty(offset/0x800, (offset&0x7ff));
	}
}

WRITE16_MEMBER( k056832_device::word_w )
{
	int layer, flip, mask, i;
	UINT32 old_data, new_data;

	old_data = m_regs[offset];
	COMBINE_DATA(&m_regs[offset]);
	new_data = m_regs[offset];

	if (new_data != old_data)
	{
		switch(offset)
		{
			/* -x-- ---- ---- hardcodes 8bpp when enabled (ignores next field)
			 * --xx ---- ---- gfx bpp select: n + 4 (1 << n), i.e. from 7bpp to 4bpp
			 * ---- -x-- ---- dotclock select: 0=8Mhz, 1=6Mhz (not used by GX)
			 * ---- --x- ---- screen flip y
			 * ---- ---x ---- screen flip x
			 * ---- ---- --x- external linescroll RAM page enable
			 */
			case 0x00/2:
				if ((new_data & 0x30) != (old_data & 0x30))
				{
					flip = 0;
					if (new_data & 0x20) flip |= TILEMAP_FLIPY;
					if (new_data & 0x10) flip |= TILEMAP_FLIPX;
					for (i = 0; i < K056832_PAGE_COUNT; i++)
					{
						m_tilemap[i]->set_flip(flip);
					}
				}

				if ((new_data & 0x02) != (old_data & 0x02))
				{
					change_rambank();
				}
			break;

			/* -------- -----xxx external irqlines enable (not used by GX)
			 * -------- xx------ tilemap attribute config (FBIT0 and FBIT1)
			 */
			//case 0x06/2: break;

			// -------- ----DCBA tile mode: 0=512x1, 1=8x8
			// -------- DCBA---- synchronous scroll: 0=off, 1=on
			case 0x08/2:
				for (layer = 0; layer < 4; layer++)
				{
					mask = 1 << layer;
					i = new_data & mask;
					if (i != (old_data & mask))
					{
						m_layer_tile_mode[layer] = i;
						mark_plane_dirty(layer);
					}
				}
			break;

			/* -------- ------xx layer A linescroll config
			 * -------- ----xx-- layer B linescroll config
			 * -------- --xx---- layer C linescroll config
			 * -------- xx------ layer D linescroll config
			 *
			 * 0: linescroll
			 * 2: rowscroll
			 * 3: xy scroll
			 */
			//case 0x0a/2: break;

			case 0x32/2:
				change_rambank();
			break;

			case 0x34/2: /* ROM bank select for checksum */
			case 0x36/2: /* secondary ROM bank select for use with tile banking */
				change_rombank();
			break;

			// extended tile address
			//case 0x38/2: break;

			// 12 bit (signed) horizontal offset if global HFLIP enabled
			//case 0x3a/2: break;

			// 11 bit (signed) vertical offset if global VFLIP enabled
			//case 0x3c/2: break;

			default:
				layer = offset & 3;

				if (offset >= 0x10/2 && offset <= 0x16/2)
				{
					m_y[layer] = (new_data & 0x18) >> 3;
					m_h[layer] = (new_data & 0x3);
					m_active_layer = layer;
					update_page_layout();
				} else

				if (offset >= 0x18/2 && offset <= 0x1e/2)
				{
					m_x[layer] = (new_data & 0x18) >> 3;
					m_w[layer] = (new_data & 0x03);
					m_active_layer = layer;
					update_page_layout();
				} else

				if (offset >= 0x20/2 && offset <= 0x26/2)
				{
					m_dy[layer] = (INT16)new_data;
				} else

				if (offset >= 0x28/2 && offset <= 0x2e/2)
				{
					m_dx[layer] = (INT16)new_data;
				}
			break;
		}
	}
}


WRITE32_MEMBER( k056832_device::long_w )
{
	// GX does access of all 3 widths (8/16/32) so we can't do the
	// if (ACCESSING_xxx) trick.  in particular, 8-bit writes
	// are used to the tilemap bank register.
	offset <<= 1;
	word_w(space, offset, data >> 16, mem_mask >> 16);
	word_w(space, offset + 1, data, mem_mask);
}


WRITE16_MEMBER( k056832_device::b_word_w )
{
	COMBINE_DATA(&m_regsb[offset]);
}




WRITE8_MEMBER( k056832_device::write )
{
	if (offset & 1)
	{
		word_w(space, (offset >> 1), data, 0x00ff);
	}
	else
	{
		word_w(space, (offset >> 1), data << 8, 0xff00);
	}
}

WRITE8_MEMBER( k056832_device::b_w )
{
	if (offset & 1)
	{
		b_word_w(space, (offset >> 1), data, 0x00ff);
	}
	else
	{
		b_word_w(space, (offset >> 1), data<<8, 0xff00);
	}
}

WRITE32_MEMBER( k056832_device::b_long_w )
{
	if (ACCESSING_BITS_16_31)
	{
		b_word_w(space, offset << 1, data >> 16, mem_mask >> 16);
	}
	if (ACCESSING_BITS_0_15)
	{
		b_word_w(space, (offset << 1) + 1, data, mem_mask);
	}
}

template<class _BitmapClass>
int k056832_device::update_linemap( screen_device &screen, _BitmapClass &bitmap, int page, int flags )
{
	if (m_page_tile_mode[page])
		return(0);
	if (!m_linemap_enabled)
		return(1);

	{
		rectangle zerorect;
		tilemap_t *tmap;
		UINT32 *dirty;
		int all_dirty;
		UINT8 *xprdata;

		tmap = m_tilemap[page];
		bitmap_ind8 &xprmap  = tmap->flagsmap();
		xprdata = tmap->tile_flags();

		dirty = m_line_dirty[page];
		all_dirty = m_all_lines_dirty[page];

		if (all_dirty)
		{
			dirty[7] = dirty[6] = dirty[5] = dirty[4] = dirty[3] = dirty[2] = dirty[1] = dirty[0] = 0;
			m_all_lines_dirty[page] = 0;

			// force tilemap into a clean, static state
			// *really ugly but it minimizes alteration to tilemap.c
			memset(&zerorect, 0, sizeof(rectangle));    // zero dimension
			tmap->draw(screen, bitmap, zerorect, 0, 0); // dummy call to reset tile_dirty_map
			xprmap.fill(0);                     // reset pixel transparency_bitmap;
			memset(xprdata, TILEMAP_PIXEL_LAYER0, 0x800);   // reset tile transparency_data;
		}
		else
		{
			if (!(dirty[0] | dirty[1] | dirty[2] | dirty[3] | dirty[4] | dirty[5] | dirty[6] | dirty[7]))
				return 0;
		}

#if 0   /* this code is broken.. really broken .. gijoe uses it for some line/column scroll style effects (lift level of attract mode)
            we REALLY shouldn't be writing directly back into the pixmap, surely this should
            be done when rendering instead

        */
		{
			bitmap_ind16 *pixmap;

			UINT8 code_transparent, code_opaque;
			const pen_t *pal_ptr;
			const UINT8  *src_ptr;
			UINT8  *xpr_ptr;
			UINT16 *dst_ptr;
			UINT16 pen, basepen;
			int count, src_pitch, src_modulo;
			int dst_pitch;
			int line;
			gfx_element *src_gfx;
			int offs, mask;

			#define LINE_WIDTH 512

			#define DRAW_PIX(N) \
				pen = src_ptr[N]; \
				if (pen) \
				{ pen += basepen; xpr_ptr[count+N] = TILEMAP_PIXEL_LAYER0; dst_ptr[count+N] = pen; } else \
				{ xpr_ptr[count+N] = 0; }

			pixmap  = m_pixmap[page];
			pal_ptr = machine().pens;
			src_gfx = m_gfxdecode->gfx(m_gfx_num];
			src_pitch  = src_gfx->rowbytes();
			src_modulo = src_gfx->char_modulo;
			dst_pitch  = pixmap->rowpixels;

			for (line = 0; line < 256; line++)
			{
				tile_data tileinfo = {0};

				dst_ptr = &pixmap->pix16(line);
				xpr_ptr = &xprmap.pix8(line);

				if (!all_dirty)
				{
					offs = line >> 5;
					mask = 1 << (line & 0x1f);
					if (!(dirty[offs] & mask)) continue;
					dirty[offs) ^= mask;
				}

				for (count = 0; count < LINE_WIDTH; count += 8)
				{
					get_tile_info(&tileinfo, line, page);
					basepen = tileinfo.palette_base;
					code_transparent = tileinfo.category;
					code_opaque = code_transparent | TILEMAP_PIXEL_LAYER0;

					src_ptr = tileinfo.pen_data + count * 8;//src_base + ((tileinfo.tile_number & ~7) << 6);

					DRAW_PIX(0)
					DRAW_PIX(1)
					DRAW_PIX(2)
					DRAW_PIX(3)
					DRAW_PIX(4)
					DRAW_PIX(5)
					DRAW_PIX(6)
					DRAW_PIX(7)
				}
			}

			#undef LINE_WIDTH
			#undef DRAW_PIX
		}
#endif

	}

	return(0);
}

template<class _BitmapClass>
void k056832_device::tilemap_draw_common( screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int layer, UINT32 flags, UINT32 priority )
{
	UINT32 last_dx, last_visible, new_colorbase, last_active;
	int sx, sy, ay, tx, ty, width, height;
	int clipw, clipx, cliph, clipy, clipmaxy;
	int line_height, line_endy, line_starty, line_y;
	int sdat_start, sdat_walk, sdat_adv, sdat_wrapmask, sdat_offs;
	int pageIndex, flipx, flipy, corr, r, c;
	int cminy, cmaxy, cminx, cmaxx;
	int dminy, dmaxy, dminx, dmaxx;
	rectangle drawrect;
	tilemap_t *tmap;
	UINT16 *p_scroll_data;
	UINT16 ram16[2];

	int rowstart = m_y[layer];
	int colstart = m_x[layer];
	int rowspan  = m_h[layer] + 1;
	int colspan  = m_w[layer] + 1;
	int dy = m_dy[layer];
	int dx = m_dx[layer];
	int scrollbank = ((m_regs[0x18] >> 1) & 0xc) | (m_regs[0x18] & 3);
	int scrollmode = m_regs[0x05] >> (m_lsram_page[layer][0] << 1) & 3;

	if (m_use_ext_linescroll)
	{
		scrollbank = K056832_PAGE_COUNT;
	}

	height = rowspan * K056832_PAGE_HEIGHT;
	width  = colspan * K056832_PAGE_WIDTH;

	cminx = cliprect.min_x;
	cmaxx = cliprect.max_x;
	cminy = cliprect.min_y;
	cmaxy = cliprect.max_y;

	// flip correction registers
	flipy = m_regs[0] & 0x20;
	if (flipy)
	{
		corr = m_regs[0x3c/2];
		if (corr & 0x400)
			corr |= 0xfffff800;
	}
	else
		corr = 0;

	dy += corr;
	ay = (unsigned)(dy - m_layer_offs[layer][1]) % height;

	flipx = m_regs[0] & 0x10;
	if (flipx)
	{
		corr = m_regs[0x3a/2];
		if (corr & 0x800)
			corr |= 0xfffff000;
	}
	else
		corr = 0;

	corr -= m_layer_offs[layer][0];

	switch( scrollmode )
	{
		case 0: // linescroll
			p_scroll_data = &m_videoram[scrollbank<<12] + (m_lsram_page[layer][1]>>1);
			line_height = 1;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 2;
		break;
		case 2: // rowscroll
			p_scroll_data = &m_videoram[scrollbank << 12] + (m_lsram_page[layer][1] >> 1);
			line_height = 8;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 16;
		break;
		default: // xyscroll
			p_scroll_data = ram16;
			line_height = K056832_PAGE_HEIGHT;
			sdat_wrapmask = 0;
			sdat_adv = 0;
			ram16[0] = 0;
			ram16[1] = dx;
	}
	if (flipy)
		sdat_adv = -sdat_adv;
/*
if (scrollmode==2)
{
printf("%08x    %08x    %08x\n",layer,scrollbank<<12,m_lsram_page[layer][1]>>1);
printf("\n000-100:\n");
for (int zz=0x000; zz<0x100; zz++)
    printf("%04x    ",m_videoram[(scrollbank<<12)+(m_lsram_page[layer][1]>>1)+zz]);
printf("\n100-200:\n");
for (int zz=0x100; zz<0x200; zz++)
    printf("%04x    ",m_videoram[(scrollbank<<12)+(m_lsram_page[layer][1]>>1)+zz]);
printf("\n200-300:\n");
for (int zz=0x200; zz<0x300; zz++)
    printf("%04x    ",m_videoram[(scrollbank<<12)+(m_lsram_page[layer][1]>>1)+zz]);
printf("\n300-400:\n");
for (int zz=0x300; zz<0x400; zz++)
    printf("%04x    ",m_videoram[(scrollbank<<12)+(m_lsram_page[layer][1]>>1)+zz]);
printf("\nend\n");
}
*/
	last_active = m_active_layer;
	new_colorbase = (m_k055555 != nullptr) ? m_k055555->K055555_get_palette_index(layer) : 0;

	for (r = 0; r < rowspan; r++)
	{
		if (rowspan > 1)
		{
			sy = ay;
			ty = r * K056832_PAGE_HEIGHT;

			if (!flipy)
			{
				// handle bottom-edge wraparoundness and cull off-screen tilemaps
				if ((r == 0) && (sy > height - K056832_PAGE_HEIGHT)) sy -= height;
				if ((sy + K056832_PAGE_HEIGHT <= ty) || (sy - K056832_PAGE_HEIGHT >= ty)) continue;

				// switch frame of reference and clip y
				if ((ty -= sy) >= 0)
				{
					cliph = K056832_PAGE_HEIGHT - ty;
					clipy = line_starty = ty;
					line_endy = K056832_PAGE_HEIGHT;
					sdat_start = 0;
				}
				else
				{
					cliph = K056832_PAGE_HEIGHT + ty;
					ty = -ty;
					clipy = line_starty = 0;
					line_endy = cliph;
					sdat_start = ty;
					if (scrollmode == 2) { sdat_start &= ~7; line_starty -= ty & 7; }
				}
			}
			else
			{
				ty += K056832_PAGE_HEIGHT;

				// handle top-edge wraparoundness and cull off-screen tilemaps
				if ((r == rowspan - 1) && (sy < K056832_PAGE_HEIGHT)) sy += height;
				if ((sy + K056832_PAGE_HEIGHT <= ty) || (sy - K056832_PAGE_HEIGHT >= ty)) continue;

				// switch frame of reference and clip y
				if ((ty -= sy) <= 0)
				{
					cliph = K056832_PAGE_HEIGHT + ty;
					clipy = line_starty = -ty;
					line_endy = K056832_PAGE_HEIGHT;
					sdat_start = K056832_PAGE_HEIGHT - 1;
					if (scrollmode == 2) sdat_start &= ~7;
				}
				else
				{
					cliph = K056832_PAGE_HEIGHT - ty;
					clipy = line_starty = 0;
					line_endy = cliph;
					sdat_start = cliph - 1;
					if (scrollmode == 2)
					{
						sdat_start &= ~7;
						line_starty -= ty & 7;
					}
				}
			}
		}
		else
		{
			cliph = line_endy = K056832_PAGE_HEIGHT;
			clipy = line_starty = 0;

			if (!flipy)
				sdat_start = dy;
			else
				/*
				    doesn't work with Metamorphic Force and Martial Champion (software Y-flipped) but
				    LE2U (naturally Y-flipped) seems to expect this condition as an override.

				    sdat_start = K056832_PAGE_HEIGHT-1 -dy;
				*/
			sdat_start = K056832_PAGE_HEIGHT - 1;

			if (scrollmode == 2) { sdat_start &= ~7; line_starty -= dy & 7; }
		}

		sdat_start += r * K056832_PAGE_HEIGHT;
		sdat_start <<= 1;

		clipmaxy = clipy + cliph - 1;

		for (c = 0; c < colspan; c++)
		{
			pageIndex = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);

			if (m_layer_association)
			{
				if (m_layer_assoc_with_page[pageIndex] != layer)
					continue;
			}
			else
			{
				if (m_layer_assoc_with_page[pageIndex] == -1)
					continue;

				m_active_layer = layer;
			}

			if (m_k055555 != nullptr)       // are we using k055555 palette?
			{
				if (m_last_colorbase[pageIndex] != new_colorbase)
				{
					m_last_colorbase[pageIndex] = new_colorbase;
					mark_page_dirty(pageIndex);
				}
			}
			else
			{
				if (!pageIndex)
					m_active_layer = 0;
			}

			if (update_linemap(screen, bitmap, pageIndex, flags))
				continue;

			tmap = m_tilemap[pageIndex];

			tmap->set_scrolly(0, ay);

			last_dx = 0x100000;
			last_visible = 0;

			for (sdat_walk = sdat_start, line_y = line_starty; line_y < line_endy; sdat_walk += sdat_adv, line_y += line_height)
			{
				dminy = line_y;
				dmaxy = line_y + line_height - 1;

				if (dminy < clipy) dminy = clipy;
				if (dmaxy > clipmaxy) dmaxy = clipmaxy;
				if (dminy > cmaxy || dmaxy < cminy) continue;

				sdat_offs = sdat_walk & sdat_wrapmask;

				drawrect.min_y = (dminy < cminy ) ? cminy : dminy;
				drawrect.max_y = (dmaxy > cmaxy ) ? cmaxy : dmaxy;
// printf("%04x  %04x\n",layer,flipy);
				// in xexex: K056832_DRAW_FLAG_MIRROR != flipy
				if ((scrollmode == 2) && (flags & K056832_DRAW_FLAG_MIRROR) && (flipy))
					dx = ((int)p_scroll_data[sdat_offs + 0x1e0 + 14]<<16 | (int)p_scroll_data[sdat_offs + 0x1e0 + 15]) + corr;
				else
					dx = ((int)p_scroll_data[sdat_offs]<<16 | (int)p_scroll_data[sdat_offs + 1]) + corr;

				if (last_dx == dx) { if (last_visible) goto LINE_SHORTCIRCUIT; continue; }
				last_dx = dx;

				if (colspan > 1)
				{
					//sx = (unsigned)dx % width;
					sx = (unsigned)dx & (width-1);

					//tx = c * K056832_PAGE_WIDTH;
					tx = c << 9;

					if (!flipx)
					{
						// handle right-edge wraparoundness and cull off-screen tilemaps
						if ((c == 0) && (sx > width - K056832_PAGE_WIDTH)) sx -= width;
						if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
							{ last_visible = 0; continue; }

						// switch frame of reference and clip x
						if ((tx -= sx) <= 0) { clipw = K056832_PAGE_WIDTH + tx; clipx = 0; }
						else { clipw = K056832_PAGE_WIDTH - tx; clipx = tx; }
					}
					else
					{
						tx += K056832_PAGE_WIDTH;

						// handle left-edge wraparoundness and cull off-screen tilemaps
						if ((c == colspan-1) && (sx < K056832_PAGE_WIDTH)) sx += width;
						if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
							{ last_visible = 0; continue; }

						// switch frame of reference and clip y
						if ((tx -= sx) >= 0) { clipw = K056832_PAGE_WIDTH - tx; clipx = 0; }
						else { clipw = K056832_PAGE_WIDTH + tx; clipx = -tx; }
					}
				}
				else { clipw = K056832_PAGE_WIDTH; clipx = 0; }

				last_visible = 1;

				dminx = clipx;
				dmaxx = clipx + clipw - 1;

				drawrect.min_x = (dminx < cminx ) ? cminx : dminx;
				drawrect.max_x = (dmaxx > cmaxx ) ? cmaxx : dmaxx;

				// soccer superstars visible area is >512 pixels, this causes problems with the logic because
				// the tilemaps are 512 pixels across.  Assume that if the limits were set as below that we
				// want the tilemap to be drawn on the right hand side..  this is probably not the correct
				// logic, but it works.
				if ((drawrect.min_x>0) && (drawrect.max_x==511))
					drawrect.max_x=cliprect.max_x;

				tmap->set_scrollx(0, dx);

				LINE_SHORTCIRCUIT:
					tmap->draw(screen, bitmap, drawrect, flags, priority);

			} // end of line loop
		} // end of column loop
	} // end of row loop

	m_active_layer = last_active;
} // end of function

void k056832_device::tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, UINT32 flags, UINT32 priority )
{ tilemap_draw_common(screen, bitmap, cliprect, layer, flags, priority); }

void k056832_device::tilemap_draw( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, UINT32 flags, UINT32 priority )
{ tilemap_draw_common(screen, bitmap, cliprect, layer, flags, priority); }


void k056832_device::tilemap_draw_dj( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, UINT32 flags, UINT32 priority )
{
	UINT32 last_dx, last_visible, new_colorbase, last_active;
	int sx, sy, ay, tx, ty, width, height;
	int clipw, clipx, cliph, clipy, clipmaxy;
	int line_height, line_endy, line_starty, line_y;
	int sdat_start, sdat_walk, sdat_adv, sdat_wrapmask, sdat_offs;
	int pageIndex, flipx, flipy, corr, r, c;
	int cminy, cmaxy, cminx, cmaxx;
	int dminy, dmaxy, dminx, dmaxx;
	rectangle drawrect;
	tilemap_t *tmap;
	UINT16 *p_scroll_data;
	UINT16 ram16[2];

	int rowstart = m_y[layer];
	int colstart = m_x[layer];
	int rowspan  = m_h[layer] + 1;
	int colspan  = m_w[layer] + 1;
	int dy = m_dy[layer];
	int dx = m_dx[layer];
	int scrollbank = ((m_regs[0x18] >> 1) & 0xc) | (m_regs[0x18] & 3);
	int scrollmode = m_regs[0x05] >> (m_lsram_page[layer][0] << 1) & 3;
	int need_wrap = -1;

	height = rowspan * K056832_PAGE_HEIGHT;
	width  = colspan * K056832_PAGE_WIDTH;

	cminx = cliprect.min_x;
	cmaxx = cliprect.max_x;
	cminy = cliprect.min_y;
	cmaxy = cliprect.max_y;

	// flip correction registers
	flipy = m_regs[0] & 0x20;
	if (flipy)
	{
		corr = m_regs[0x3c/2];
		if (corr & 0x400)
			corr |= 0xfffff800;
	}
	else
		corr = 0;
	dy += corr;
	ay = (unsigned)(dy - m_layer_offs[layer][1]) % height;

	flipx = m_regs[0] & 0x10;
	if (flipx)
	{
		corr = m_regs[0x3a/2];
		if (corr & 0x800)
			corr |= 0xfffff000;
	}
	else
		corr = 0;

	corr -= m_layer_offs[layer][0];

	switch( scrollmode )
	{
		case 0: // linescroll
			p_scroll_data = &m_videoram[scrollbank << 12] + (m_lsram_page[layer][1] >> 1);
			line_height = 1;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 2;
		break;
		case 2: // rowscroll
			p_scroll_data = &m_videoram[scrollbank << 12] + (m_lsram_page[layer][1] >> 1);
			line_height = 8;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 16;
		break;
		default: // xyscroll
			p_scroll_data = ram16;
			line_height = K056832_PAGE_HEIGHT;
			sdat_wrapmask = 0;
			sdat_adv = 0;
			ram16[0] = 0;
			ram16[1] = dx;
	}
	if (flipy)
		sdat_adv = -sdat_adv;

	last_active = m_active_layer;
	new_colorbase = (m_k055555 != nullptr) ? m_k055555->K055555_get_palette_index(layer) : 0;

	for (r = 0; r <= rowspan; r++)
	{
		sy = ay;
		if (r == rowspan)
		{
			if (need_wrap < 0)
				continue;

			ty = need_wrap * K056832_PAGE_HEIGHT;
		}
		else
		{
			ty = r * K056832_PAGE_HEIGHT;
		}

		// cull off-screen tilemaps
		if ((sy + height <= ty) || (sy - height >= ty))
			continue;

		// switch frame of reference
		ty -= sy;

			// handle top-edge wraparoundness
			if (r == rowspan)
			{
				cliph = K056832_PAGE_HEIGHT + ty;
				clipy = line_starty = 0;
				line_endy = cliph;
				ty = -ty;
				sdat_start = ty;
				if (scrollmode == 2) { sdat_start &= ~7; line_starty -= ty & 7; }
			}

			// clip y
			else
			{
				if (ty < 0)
					ty += height;

				clipy = ty;
				cliph = K056832_PAGE_HEIGHT;

				if (clipy + cliph > height)
				{
					cliph = height - clipy;
					need_wrap =r;
				}

				line_starty = ty;
				line_endy = line_starty + cliph;
				sdat_start = 0;
			}

		if (r == rowspan)
			sdat_start += need_wrap * K056832_PAGE_HEIGHT;
		else
			sdat_start += r * K056832_PAGE_HEIGHT;
		sdat_start <<= 1;

		clipmaxy = clipy + cliph - 1;

		for (c = 0; c < colspan; c++)
		{
			if (r == rowspan)
				pageIndex = (((rowstart + need_wrap) & 3) << 2) + ((colstart + c) & 3);
			else
				pageIndex = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);

			if (m_layer_association)
			{
				if (m_layer_assoc_with_page[pageIndex] != layer)
					continue;
			}
			else
			{
				if (m_layer_assoc_with_page[pageIndex] == -1) continue;
				m_active_layer = layer;
			}

			if (m_k055555 != nullptr)       // are we using k055555 palette?
			{
				if (m_last_colorbase[pageIndex] != new_colorbase)
				{
					m_last_colorbase[pageIndex] = new_colorbase;
					mark_page_dirty(pageIndex);
				}
			}
			else
			{
				if (!pageIndex)
					m_active_layer = 0;
			}

			if (update_linemap(screen, bitmap, pageIndex, flags))
				continue;

			tmap = m_tilemap[pageIndex];
			tmap->set_scrolly(0, ay);

			last_dx = 0x100000;
			last_visible = 0;

			for (sdat_walk = sdat_start, line_y = line_starty; line_y < line_endy; sdat_walk += sdat_adv, line_y += line_height)
			{
				dminy = line_y;
				dmaxy = line_y + line_height - 1;

				if (dminy < clipy) dminy = clipy;
				if (dmaxy > clipmaxy) dmaxy = clipmaxy;
				if (dminy > cmaxy || dmaxy < cminy) continue;

				sdat_offs = sdat_walk & sdat_wrapmask;

				drawrect.min_y = (dminy < cminy ) ? cminy : dminy;
				drawrect.max_y = (dmaxy > cmaxy ) ? cmaxy : dmaxy;

				dx = ((int)p_scroll_data[sdat_offs] << 16 | (int)p_scroll_data[sdat_offs + 1]) + corr;

				if (last_dx == dx) { if (last_visible) goto LINE_SHORTCIRCUIT; continue; }
				last_dx = dx;

				if (colspan > 1)
				{
					//sx = (unsigned)dx % width;
					sx = (unsigned)dx & (width-1);

					//tx = c * K056832_PAGE_WIDTH;
					tx = c << 9;

					if (!flipx)
					{
						// handle right-edge wraparoundness and cull off-screen tilemaps
						if ((c == 0) && (sx > width - K056832_PAGE_WIDTH)) sx -= width;
						if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
							{ last_visible = 0; continue; }

						// switch frame of reference and clip x
						if ((tx -= sx) <= 0) { clipw = K056832_PAGE_WIDTH + tx; clipx = 0; }
						else { clipw = K056832_PAGE_WIDTH - tx; clipx = tx; }
					}
					else
					{
						tx += K056832_PAGE_WIDTH;

						// handle left-edge wraparoundness and cull off-screen tilemaps
						if ((c == colspan-1) && (sx < K056832_PAGE_WIDTH)) sx += width;
						if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
							{ last_visible = 0; continue; }

						// switch frame of reference and clip y
						if ((tx -= sx) >= 0) { clipw = K056832_PAGE_WIDTH - tx; clipx = 0; }
						else { clipw = K056832_PAGE_WIDTH + tx; clipx = -tx; }
					}
				}
				else
				{
					clipw = K056832_PAGE_WIDTH;
					clipx = 0;
				}

				last_visible = 1;

				dminx = clipx;
				dmaxx = clipx + clipw - 1;

				drawrect.min_x = (dminx < cminx ) ? cminx : dminx;
				drawrect.max_x = (dmaxx > cmaxx ) ? cmaxx : dmaxx;

				tmap->set_scrollx(0, dx);

				LINE_SHORTCIRCUIT:
					tmap->draw(screen, bitmap, drawrect, flags, priority);

			} // end of line loop
		} // end of column loop
	} // end of row loop

	m_active_layer = last_active;

} // end of function


void k056832_device::set_layer_association( int status )
{
	m_default_layer_association = status;
}


void k056832_device::set_layer_offs( int layer, int offsx, int offsy )
{
	m_layer_offs[layer][0] = offsx;
	m_layer_offs[layer][1] = offsy;
}

void k056832_device::set_lsram_page( int logical_page, int physical_page, int physical_offset )
{
	m_lsram_page[logical_page][0] = physical_page;
	m_lsram_page[logical_page][1] = physical_offset;
}

void k056832_device::linemap_enable( int enable )
{
	m_linemap_enabled = enable;
}

int k056832_device::is_irq_enabled( int irqline )
{
	return(m_regs[0x06/2] & (1 << irqline & 7));
}

void k056832_device::read_avac( int *mode, int *data )
{
	*mode = m_regs[0x04/2] & 7;
	*data = m_regs[0x38/2];
}

int k056832_device::read_register( int regnum )
{
	return(m_regs[regnum]);
}

void k056832_device::postload()
{
	update_page_layout();
	change_rambank();
	change_rombank();
}

//misc debug handlers

READ32_MEMBER( k056832_device::long_r )
{
	offset <<= 1;
	return (word_r(space, offset + 1, 0xffff) | word_r(space, offset, 0xffff) << 16);
}

READ16_MEMBER( k056832_device::word_r )
{
	return (m_regs[offset]);
}       // VACSET

READ16_MEMBER( k056832_device::b_word_r )
{
	return (m_regsb[offset]);
}   // VSCCS (board dependent)

/*

  some drivers still rely on this non-device implementation, this should be collapsed into the device.

*/



/***************************************************************************/
/*                                                                         */
/*                                 054157 / 056832                         */
/*                                                                         */
/***************************************************************************/


void k056832_device::create_gfx(const char *gfx_memory_region, int bpp, int big)
{
	int gfx_index;
	int i;
	UINT32 total;
	static const gfx_layout charlayout8_tasman =
	{
		8,8,
		RGN_FRAC(1,1),
		8,
		{ 0,8,16,24,32,40,48,56 },
		{ 0,1,2,3,4,5,6,7 },
		{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64},
		8*64
	};
	static const gfx_layout charlayout8 =
	{
		8, 8,
		0,
		8,
		{ 8*7,8*3,8*5,8*1,8*6,8*2,8*4,8*0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 8*8, 8*8*2, 8*8*3, 8*8*4, 8*8*5, 8*8*6, 8*8*7 },
		8*8*8
	};
	static const gfx_layout charlayout8le =
	{
		8, 8,
		0,
		8,
//      { 0, 1, 2, 3, 0+(0x200000*8), 1+(0x200000*8), 2+(0x200000*8), 3+(0x200000*8) },
		{ 0+(0x200000*8), 1+(0x200000*8), 2+(0x200000*8), 3+(0x200000*8), 0, 1, 2, 3 },
		{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
		8*8*4
	};
	static const gfx_layout charlayout6 =
	{
		8, 8,
		0,
		6,
		{ 40, 32, 24, 8, 16, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 6*8, 6*8*2, 6*8*3, 6*8*4, 6*8*5, 6*8*6, 6*8*7 },
		8*8*6
	};
	static const gfx_layout charlayout5 =
	{
		8, 8,
		0,
		5,
		{ 32, 24, 8, 16, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 5*8, 5*8*2, 5*8*3, 5*8*4, 5*8*5, 5*8*6, 5*8*7 },
		8*8*5
	};
	static const gfx_layout charlayout4 =
	{
		8, 8,
		0,
		4,
		{ 0, 1, 2, 3 },
		{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
		8*8*4
	};
	static const gfx_layout charlayout4dj =
	{
		8, 8,
		0,
		4,
		{ 8*3,8*1,8*2,8*0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 8*4, 8*4*2, 8*4*3, 8*4*4, 8*4*5, 8*4*6, 8*4*7 },
		8*8*4
	};

	m_bpp = bpp;

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
	{
		if (m_gfxdecode->gfx(gfx_index) == nullptr) break;
	}
	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* handle the various graphics formats */
	i = (big) ? 8 : 16;

	/* decode the graphics */
	switch (bpp)
	{
		case K056832_BPP_4:
			total = machine().root_device().memregion(gfx_memory_region)->bytes() / (i*4);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, machine().root_device().memregion(gfx_memory_region)->base(), total, &charlayout4, 4);
			break;

		case K056832_BPP_5:
			total = machine().root_device().memregion(gfx_memory_region)->bytes() / (i*5);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, machine().root_device().memregion(gfx_memory_region)->base(), total, &charlayout5, 5);
			break;

		case K056832_BPP_6:
			total = machine().root_device().memregion(gfx_memory_region)->bytes() / (i*6);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, machine().root_device().memregion(gfx_memory_region)->base(), total, &charlayout6, 6);
			break;

		case K056832_BPP_8:
			total = machine().root_device().memregion(gfx_memory_region)->bytes() / (i*8);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, machine().root_device().memregion(gfx_memory_region)->base(), total, &charlayout8, 8);
			break;

		case K056832_BPP_8LE:
			total = machine().root_device().memregion(gfx_memory_region)->bytes() / (i*8);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, machine().root_device().memregion(gfx_memory_region)->base(), total, &charlayout8le, 8);
			break;

		case K056832_BPP_8TASMAN:
			total = machine().root_device().memregion(gfx_memory_region)->bytes() / (i*8);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, machine().root_device().memregion(gfx_memory_region)->base(), total, &charlayout8_tasman, 8);
			break;

		case K056832_BPP_4dj:
			total = machine().root_device().memregion(gfx_memory_region)->bytes() / (i*4);
			konami_decode_gfx(machine(), m_gfxdecode, m_palette, gfx_index, machine().root_device().memregion(gfx_memory_region)->base(), total, &charlayout4dj, 4);
			break;

		default:
			fatalerror("Unsupported bpp\n");
	}

	m_gfxdecode->gfx(gfx_index)->set_granularity(16); /* override */
	m_gfxdecode->gfx(gfx_index)->set_colors(m_palette->entries() / 16);

	m_gfx_memory_region = gfx_memory_region;
	m_gfx_num = gfx_index;

	m_rombase = machine().root_device().memregion(gfx_memory_region)->base();
	m_num_gfx_banks = machine().root_device().memregion(gfx_memory_region)->bytes() / 0x2000;
	m_cur_gfx_banks = 0;
	m_use_ext_linescroll = 0;
	m_uses_tile_banks = 0;
}


int k056832_device::altK056832_update_linemap(screen_device &screen, bitmap_rgb32 &bitmap, int page, int flags)
{
	if (m_page_tile_mode[page]) return(0);
	if (!m_linemap_enabled) return(1);


	{
		rectangle zerorect;
		tilemap_t *tmap;
		UINT32 *dirty;
		int all_dirty;
		UINT8 *xprdata;

		tmap = m_tilemap[page];
		bitmap_ind8 &xprmap  = tmap->flagsmap();
		xprdata = tmap->tile_flags();

		dirty = m_line_dirty[page];
		all_dirty = m_all_lines_dirty[page];

		if (all_dirty)
		{
			dirty[7]=dirty[6]=dirty[5]=dirty[4]=dirty[3]=dirty[2]=dirty[1]=dirty[0] = 0;
			m_all_lines_dirty[page] = 0;

			// force tilemap into a clean, static state
			// *really ugly but it minimizes alteration to tilemap.c
			memset (&zerorect, 0, sizeof(rectangle));   // zero dimension
			tmap->draw(screen, bitmap, zerorect, 0, 0); // dummy call to reset tile_dirty_map
			xprmap.fill(0);                     // reset pixel transparency_bitmap;
			memset(xprdata, TILEMAP_PIXEL_LAYER0, 0x800);   // reset tile transparency_data;
		}
		else
		{
			if (!(dirty[0]|dirty[1]|dirty[2]|dirty[3]|dirty[4]|dirty[5]|dirty[6]|dirty[7])) return(0);
		}

#if 0   /* this code is broken.. really broken .. gijoe uses it for some line/column scroll style effects (lift level of attract mode)
            we REALLY shouldn't be writing directly back into the pixmap, surely this should
            be done when rendering instead

        */
		{
			bitmap_ind16 *pixmap;

			UINT8 code_transparent, code_opaque;
			const pen_t *pal_ptr;
			const UINT8  *src_ptr;
			UINT8  *xpr_ptr;
			UINT16 *dst_ptr;
			UINT16 pen, basepen;
			int count, src_pitch, src_modulo;
			int dst_pitch;
			int line;
			gfx_element *src_gfx;
			int offs, mask;

			#define LINE_WIDTH 512

			#define DRAW_PIX(N) \
				pen = src_ptr[N]; \
				if (pen) \
				{ pen += basepen; xpr_ptr[count+N] = TILEMAP_PIXEL_LAYER0; dst_ptr[count+N] = pen; } else \
				{ xpr_ptr[count+N] = 0; }

			pixmap  = m_pixmap[page];
			pal_ptr    = machine().pens;
			src_gfx    = m_gfxdecode->gfx(m_gfx_num];
			src_pitch  = src_gfx->rowbytes();
			src_modulo = src_gfx->char_modulo;
			dst_pitch  = pixmap->rowpixels;

			for (line=0; line<256; line++)
			{
				tile_data tileinfo = {0};

				dst_ptr = &pixmap->pix16(line);
				xpr_ptr = &xprmap.pix8(line);

				if (!all_dirty)
				{
					offs = line >> 5;
					mask = 1 << (line & 0x1f);
					if (!(dirty[offs] & mask)) continue;
					dirty[offs) ^= mask;
				}

				for (count = 0; count < LINE_WIDTH; count+=8)
				{
					get_tile_info(machine, &tileinfo, line, page);
					basepen = tileinfo.palette_base;
					code_transparent = tileinfo.category;
					code_opaque = code_transparent | TILEMAP_PIXEL_LAYER0;

					src_ptr = tileinfo.pen_data + count*8;//src_base + ((tileinfo.tile_number & ~7) << 6);

					DRAW_PIX(0)
					DRAW_PIX(1)
					DRAW_PIX(2)
					DRAW_PIX(3)
					DRAW_PIX(4)
					DRAW_PIX(5)
					DRAW_PIX(6)
					DRAW_PIX(7)
				}
			}

			#undef LINE_WIDTH
			#undef DRAW_PIX
		}
#endif

	}

	return(0);
}

void k056832_device::m_tilemap_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, UINT32 flags, UINT32 priority)
{
	UINT32 last_dx, last_visible, new_colorbase, last_active;
	int sx, sy, ay, tx, ty, width, height;
	int clipw, clipx, cliph, clipy, clipmaxy;
	int line_height, line_endy, line_starty, line_y;
	int sdat_start, sdat_walk, sdat_adv, sdat_wrapmask, sdat_offs;
	int pageIndex, flipx, flipy, corr, r, c;
	int cminy, cmaxy, cminx, cmaxx;
	int dminy, dmaxy, dminx, dmaxx;
	rectangle drawrect;
	tilemap_t *tmap;
	UINT16 *pScrollData;
	UINT16 ram16[2];

	int rowstart = m_y[layer];
	int colstart = m_x[layer];
	int rowspan  = m_h[layer]+1;
	int colspan  = m_w[layer]+1;
	int dy = m_dy[layer];
	int dx = m_dx[layer];
	int scrollbank = ((m_regs[0x18]>>1) & 0xc) | (m_regs[0x18] & 3);
	int scrollmode = m_regs[0x05]>>(m_lsram_page[layer][0]<<1) & 3;

	if (m_use_ext_linescroll)
	{
		scrollbank = K056832_PAGE_COUNT;
	}

	height = rowspan * K056832_PAGE_HEIGHT;
	width  = colspan * K056832_PAGE_WIDTH;

	cminx = cliprect.min_x;
	cmaxx = cliprect.max_x;
	cminy = cliprect.min_y;
	cmaxy = cliprect.max_y;

	// flip correction registers
	flipy = m_regs[0] & 0x20;
	if (flipy)
	{
		corr = m_regs[0x3c/2];
		if (corr & 0x400)
			corr |= 0xfffff800;
	} else corr = 0;
	dy += corr;
	ay = (unsigned)(dy - m_layer_offs[layer][1]) % height;

	flipx = m_regs[0] & 0x10;
	if (flipx)
	{
		corr = m_regs[0x3a/2];
		if (corr & 0x800)
			corr |= 0xfffff000;
	} else corr = 0;
	corr -= m_layer_offs[layer][0];

	if (scrollmode == 0 && (flags & K056382_DRAW_FLAG_FORCE_XYSCROLL))
	{
		scrollmode = 3;
		flags &= ~K056382_DRAW_FLAG_FORCE_XYSCROLL;
	}

	switch( scrollmode )
	{
		case 0: // linescroll
			pScrollData = &m_videoram[scrollbank<<12] + (m_lsram_page[layer][1]>>1);
			line_height = 1;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 2;
		break;
		case 2: // rowscroll

			pScrollData = &m_videoram[scrollbank<<12] + (m_lsram_page[layer][1]>>1);
			line_height = 8;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 16;
		break;
		default: // xyscroll
			pScrollData = ram16;
			line_height = K056832_PAGE_HEIGHT;
			sdat_wrapmask = 0;
			sdat_adv = 0;
			ram16[0] = 0;
			ram16[1] = dx;
	}
	if (flipy) sdat_adv = -sdat_adv;

	last_active = m_active_layer;
	new_colorbase = (m_k055555 != nullptr) ? m_k055555->K055555_get_palette_index(layer) : 0;

	for (r=0; r<rowspan; r++)
	{
	if (rowspan > 1)
	{
		sy = ay;
		ty = r * K056832_PAGE_HEIGHT;

		if (!flipy)
		{
			// handle bottom-edge wraparoundness and cull off-screen tilemaps
			if ((r == 0) && (sy > height - K056832_PAGE_HEIGHT)) sy -= height;
			if ((sy + K056832_PAGE_HEIGHT <= ty) || (sy - K056832_PAGE_HEIGHT >= ty)) continue;

			// switch frame of reference and clip y
			if ((ty -= sy) >= 0)
			{
				cliph = K056832_PAGE_HEIGHT - ty;
				clipy = line_starty = ty;
				line_endy = K056832_PAGE_HEIGHT;
				sdat_start = 0;
			}
			else
			{
				cliph = K056832_PAGE_HEIGHT + ty;
				ty = -ty;
				clipy = line_starty = 0;
				line_endy = cliph;
				sdat_start = ty;
				if (scrollmode == 2) { sdat_start &= ~7; line_starty -= ty & 7; }
			}
		}
		else
		{
			ty += K056832_PAGE_HEIGHT;

			// handle top-edge wraparoundness and cull off-screen tilemaps
			if ((r == rowspan-1) && (sy < K056832_PAGE_HEIGHT)) sy += height;
			if ((sy + K056832_PAGE_HEIGHT <= ty) || (sy - K056832_PAGE_HEIGHT >= ty)) continue;

			// switch frame of reference and clip y
			if ((ty -= sy) <= 0)
			{
				cliph = K056832_PAGE_HEIGHT + ty;
				clipy = line_starty = -ty;
				line_endy = K056832_PAGE_HEIGHT;
				sdat_start = K056832_PAGE_HEIGHT-1;
				if (scrollmode == 2) sdat_start &= ~7;
			}
			else
			{
				cliph = K056832_PAGE_HEIGHT - ty;
				clipy = line_starty = 0;
				line_endy = cliph;
				sdat_start = cliph-1;
				if (scrollmode == 2) { sdat_start &= ~7; line_starty -= ty & 7; }
			}
		}
	}
	else
	{
		cliph = line_endy = K056832_PAGE_HEIGHT;
		clipy = line_starty = 0;

		if (!flipy)
			sdat_start = dy;
		else
			/*
			    doesn't work with Metamorphic Force and Martial Champion (software Y-flipped) but
			    LE2U (naturally Y-flipped) seems to expect this condition as an override.

			    sdat_start = K056832_PAGE_HEIGHT-1 -dy;
			*/
			sdat_start = K056832_PAGE_HEIGHT-1;

		if (scrollmode == 2) { sdat_start &= ~7; line_starty -= dy & 7; }
	}

	sdat_start += r * K056832_PAGE_HEIGHT;
	sdat_start <<= 1;

	clipmaxy = clipy + cliph - 1;

	for (c=0; c<colspan; c++)
	{
		pageIndex = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);

		if (m_layer_association)
		{
			if (m_layer_assoc_with_page[pageIndex] != layer) continue;
		}
		else
		{
			if (m_layer_assoc_with_page[pageIndex] == -1) continue;
			m_active_layer = layer;
		}

		if (m_k055555 != nullptr)
		{
			if (m_last_colorbase[pageIndex] != new_colorbase)
			{
				m_last_colorbase[pageIndex] = new_colorbase;
				mark_page_dirty(pageIndex);
			}
		}
		else
			if (!pageIndex) m_active_layer = 0;

		if (altK056832_update_linemap(screen, bitmap, pageIndex, flags)) continue;

		tmap = m_tilemap[pageIndex];
		tmap->set_scrolly(0, ay);

		last_dx = 0x100000;
		last_visible = 0;

		for (sdat_walk=sdat_start, line_y=line_starty; line_y<line_endy; sdat_walk+=sdat_adv, line_y+=line_height)
		{
			dminy = line_y;
			dmaxy = line_y + line_height - 1;

			if (dminy < clipy) dminy = clipy;
			if (dmaxy > clipmaxy) dmaxy = clipmaxy;
			if (dminy > cmaxy || dmaxy < cminy) continue;

			sdat_offs = sdat_walk & sdat_wrapmask;

			drawrect.min_y = (dminy < cminy ) ? cminy : dminy;
			drawrect.max_y = (dmaxy > cmaxy ) ? cmaxy : dmaxy;

			dx = ((int)pScrollData[sdat_offs]<<16 | (int)pScrollData[sdat_offs+1]) + corr;

			if (last_dx == dx) { if (last_visible) goto LINE_SHORTCIRCUIT; continue; }
			last_dx = dx;

			if (colspan > 1)
			{
				//sx = (unsigned)dx % width;
				sx = (unsigned)dx & (width-1);

				//tx = c * K056832_PAGE_WIDTH;
				tx = c << 9;

				if (!flipx)
				{
					// handle right-edge wraparoundness and cull off-screen tilemaps
					if ((c == 0) && (sx > width - K056832_PAGE_WIDTH)) sx -= width;
					if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
						{ last_visible = 0; continue; }

					// switch frame of reference and clip x
					if ((tx -= sx) <= 0) { clipw = K056832_PAGE_WIDTH + tx; clipx = 0; }
					else { clipw = K056832_PAGE_WIDTH - tx; clipx = tx; }
				}
				else
				{
					tx += K056832_PAGE_WIDTH;

					// handle left-edge wraparoundness and cull off-screen tilemaps
					if ((c == colspan-1) && (sx < K056832_PAGE_WIDTH)) sx += width;
					if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
						{ last_visible = 0; continue; }

					// switch frame of reference and clip y
					if ((tx -= sx) >= 0) { clipw = K056832_PAGE_WIDTH - tx; clipx = 0; }
					else { clipw = K056832_PAGE_WIDTH + tx; clipx = -tx; }
				}
			}
			else { clipw = K056832_PAGE_WIDTH; clipx = 0; }

			last_visible = 1;

			dminx = clipx;
			dmaxx = clipx + clipw - 1;

			drawrect.min_x = (dminx < cminx ) ? cminx : dminx;
			drawrect.max_x = (dmaxx > cmaxx ) ? cmaxx : dmaxx;

			// soccer superstars visible area is >512 pixels, this causes problems with the logic because
			// the tilemaps are 512 pixels across.  Assume that if the limits were set as below that we
			// want the tilemap to be drawn on the right hand side..  this is probably not the correct
			// logic, but it works.
			if ((drawrect.min_x>0) && (drawrect.max_x==511)) drawrect.max_x=cliprect.max_x;

			tmap->set_scrollx(0, dx);

			LINE_SHORTCIRCUIT:
			tmap->draw(screen, bitmap, drawrect, flags, priority);

		} // end of line loop
	} // end of column loop
	} // end of row loop

	m_active_layer = last_active;

} // end of function


int k056832_device::get_layer_association(void)
{
	return(m_layer_association);
}
