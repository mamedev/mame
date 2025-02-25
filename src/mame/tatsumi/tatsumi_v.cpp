// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
#include "emu.h"
#include "tatsumi.h"
#include "screen.h"


/**********************************
 *
 * Common routines
 *
 *********************************/

// TODO: move into device
void tatsumi_state::hd6445_crt_w(offs_t offset, uint8_t data)
{
	if (offset==0)
		m_hd6445_address = data & 0x3f;
	if (offset==1)
	{
		m_hd6445_reg[m_hd6445_address] = data;

		static char const *const regnames[40] =
		{
			"Horizontal Total Characters", "Horizontal Displayed Characters", "Horizontal Sync Position",   "Sync Width",
			"Vertical Total Rows",         "Vertical Total Adjust",           "Vertical Displayed Rows",    "Vertical Sync Position",
			"Interlace Mode and Skew",     "Max Raster Address",              "Cursor 1 Start",             "Cursor 1 End",
			"Screen 1 Start Address (H)",  "Screen 1 Start Address (L)",      "Cursor 1 Address (H)",       "Cursor 1 Address (L)",
			"Light Pen (H) (RO)",          "Light Pen (L) (RO)",              "Screen 2 Start Position",    "Screen 2 Start Address (H)",
			"Screen 2 Start Address (L)",  "Screen 3 Start Position",         "Screen 3 Start Address (H)", "Screen 3 Start Address (L)",
			"Screen 4 Start Position",     "Screen 4 Start Address (H)",      "Screen 4 Start Address (L)", "Vertical Sync Position Adjust",
			"Light Pen Raster (RO)",       "Smooth Scrolling",                "Control 1",                  "Control 2",
			"Control 3",                   "Memory Width Offset",             "Cursor 2 Start",             "Cursor 2 End",
			"Cursor 2 Address (H)",        "Cursor 2 Address (L)",            "Cursor 1 Width",             "Cursor 2 Width"
		};

		if(m_hd6445_address < 40)
			logerror("HD6445: register %s [%02x R%d] set %02x\n",regnames[m_hd6445_address],m_hd6445_address,m_hd6445_address,data);
		else
			logerror("HD6445: illegal register access [%02x R%d] set %02x\n",m_hd6445_address,m_hd6445_address,data);
	}
}

uint16_t tatsumi_state::tatsumi_sprite_control_r(offs_t offset)
{
	return m_sprite_control_ram[offset];
}

void tatsumi_state::tatsumi_sprite_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sprite_control_ram[offset]);

	/* 0xe0 is bank switch, others unknown */
//  if ((offset==0xe0 && data&0xefff) || offset!=0xe0)
//      logerror("%s:  Tatsumi TZB215 sprite control %04x %08x\n", m_maincpu->pc(), offset, data);
}

// apply shadowing to underlying layers
// TODO: it might mix up with the lower palette bank instead (color bank 0x1400?)
void tatsumi_state::apply_shadow_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &shadow_bitmap, uint8_t xor_output)
{
	for(int y=cliprect.min_y;y<cliprect.max_y;y++)
	{
		for(int x=cliprect.min_x;x<cliprect.max_x;x++)
		{
			uint8_t shadow = shadow_bitmap.pix(y, x);
			// xor_output is enabled during Chen boss fight (where shadows have more brightness than everything else)
			// TODO: transition before fighting him should also black out all the background tilemaps too!?
			//       (more evidence that we need to mix with color bank 0x1400 instead of doing true RGB mixing).
			if(shadow ^ xor_output)
			{
				rgb_t shadow_pen = bitmap.pix(y, x);
				bitmap.pix(y, x) = rgb_t(shadow_pen.r() >> 1,shadow_pen.g() >> 1, shadow_pen.b() >> 1);
			}
		}
	}
}

void tatsumi_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(tatsumi_state::get_text_tile_info)
{
	int tile = m_videoram[tile_index];
	tileinfo.set(0,
			tile & 0xfff,
			tile >> 12,
			0);
}




/**********************************
 *
 * Cycle Warriors / Big Fight
 *
 *********************************/

/*
 * these video registers never changes
 *
 * Big Fight
 * 72f2 5af2 3af2 22fa
 *
 * Cycle Warriors
 * 5673 92c2 3673 267b
 *
 * Following is complete guesswork (since nothing changes it's very hard to pinpoint what these bits do :/)
 * Layer order is 3-1-2-0 ?
 * x--- -x-- ---- ---- one of these might be enable page select
 * ---- ---- x--- ---- tilemap size
 * ---x ---- ---- x--- one these might be color bank
 *
 */
void cyclwarr_state::video_config_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_config[offset]);
}

// mixing control (seems to be available only for Big Fight and Cycle Warriors)
// --x- ---- enabled in Big Fight, disabled in Cycle Warriors (unknown purpose)
// ---- -x-- enable shadow mixing
// ---- ---x if 1 invert shadows, i.e. shadows are drawn with original pen while non shadows are halved (Chen stage in Big Fight)
void cyclwarr_state::mixing_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mixing_control);
}

template<int Bank>
TILE_GET_INFO_MEMBER(cyclwarr_state::get_tile_info_bigfight)
{
	int tile = m_cyclwarr_videoram[Bank >> 1][tile_index&0x7fff];
	int bank = (m_bigfight_a40000[0] >> (((tile&0xc00)>>10)*4))&0xf;
	uint16_t tileno = (tile&0x3ff)|(bank<<10);
	// color is bits 12-13
	uint8_t color = (tile >> 12) & 0x3;

	// all layers but 0 wants this palette bank (fade in/out effects)
	// a similar result is obtainable with priority bit, but then it's wrong for
	// Big Fight CRT test (dark red background) and character name bio in attract mode (reference shows it doesn't fade in like rest of text)
	// TODO: likely an HW config sets this up
	if(Bank != 0)
		color |= 4;
	// bit 14: ignore transparency on this tile
	int opaque = ((tile >> 14) & 1) == 1;

	tileinfo.set(0,
						 tileno,
						 color,
						opaque ? TILE_FORCE_LAYER0 : 0);

	// bit 15: tile appears in front of sprites
	tileinfo.category = (tile >> 15) & 1;
	tileinfo.mask_data = &m_mask[tileno<<3];
}

// same as above but additionally apply per-scanline color banking
// TODO: split for simplicity, need to merge with above
template<int Bank>
TILE_GET_INFO_MEMBER(cyclwarr_state::get_tile_info_cyclwarr_road)
{
	int tile = m_cyclwarr_videoram[Bank >> 1][tile_index&0x7fff];
	int bank = (m_bigfight_a40000[0] >> (((tile&0xc00)>>10)*4))&0xf;
	uint16_t tileno = (tile&0x3ff)|(bank<<10);
	uint8_t color = (tile >> 12) & 0x3;
//  if(Bank != 0)
	color |= 4;
	int opaque = ((tile >> 14) & 1) == 1;

	tileinfo.set(0,
						 tileno,
						 color | m_road_color_bank,
						 opaque ? TILE_FORCE_LAYER0 : 0);

	tileinfo.category = (tile >> 15) & 1;
	tileinfo.mask_data = &m_mask[((tile&0x3ff)|(bank<<10))<<3];
}

void cyclwarr_state::tile_expand()
{
	/*
	    Each tile (0x4000 of them) has a lookup table in ROM to build an individual 3-bit palette
	    from sets of 8 bit palettes!
	*/
	gfx_element *gx0 = m_gfxdecode->gfx(0);
	m_mask.resize(gx0->elements() << 3,0);
	uint8_t *dest;

	// allocate memory for the assembled data
	m_decoded_gfx = std::make_unique<uint8_t[]>(gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	dest = m_decoded_gfx.get();
	for (int c = 0; c < gx0->elements(); c++)
	{
		const uint8_t *c0base = gx0->get_data(c);

		// loop over height
		for (int y = 0; y < gx0->height(); y++)
		{
			const uint8_t *c0 = c0base;

			for (int x = 0; x < gx0->width(); x++)
			{
				uint8_t pix = (*c0++ & 7);
				uint8_t respix = m_cyclwarr_tileclut[(c << 3)|pix];
				*dest++ = respix;
				// Transparent pixels are set by both the tile pixel data==0 AND colour palette & 7 == 0
				m_mask[(c << 3) | (y & 7)] |= ((pix&0x7)!=0 || ((pix&0x7)==0 && (respix&0x7)!=0)) ? (0x80 >> (x & 7)) : 0;
			}
			c0base += gx0->rowbytes();
		}
	}

	gx0->set_raw_layout(m_decoded_gfx.get(), gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(256);
}


VIDEO_START_MEMBER(cyclwarr_state,cyclwarr)
{
	tile_expand();
	m_layer[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<0>)),      TILEMAP_SCAN_ROWS, 8,8,  64,512);
	m_layer[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_cyclwarr_road<1>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<2>)),      TILEMAP_SCAN_ROWS, 8,8,  64,512);
	m_layer[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<3>)),      TILEMAP_SCAN_ROWS, 8,8,  64,512);


	// set up scroll bases
	// TODO: more HW configs
	m_layer[3]->set_scrolldx(-8,-8);
	m_layer_page_size[3] = 0x200;
	m_layer[2]->set_scrolldx(-8,-8);
	m_layer_page_size[2] = 0x200;
	m_layer[1]->set_scrolldx(-8,-8);
	m_layer_page_size[1] = 0x200;
	m_layer[0]->set_scrolldx(-0x10,-0x10);
	m_layer_page_size[0] = 0x100;

	m_layer1_can_be_road = true;
}

VIDEO_START_MEMBER(cyclwarr_state,bigfight)
{
	tile_expand();
	m_layer[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<0>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<1>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<2>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<3>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);

	// set up scroll bases
	// TODO: more HW configs
	m_layer[3]->set_scrolldx(-8,-8);
	m_layer[2]->set_scrolldx(-8,-8);
	m_layer[1]->set_scrolldx(-8,-8);
	m_layer[0]->set_scrolldx(-0x10,-0x10);
	for(int i=0;i<4;i++)
		m_layer_page_size[i] = 0x200;

	m_layer1_can_be_road = false;
}


void cyclwarr_state::draw_bg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *src, const uint16_t* scrollx, const uint16_t* scrolly, const uint16_t layer_page_size, bool is_road, int hi_priority)
{
	rectangle clip;
	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;
	// TODO: both always enabled when this occurs
	bool rowscroll_enable = (scrollx[0] & 0x1000) == 0;
	bool colscroll_enable = (scrollx[0] & 0x2000) == 0;
	// this controls wraparound (tilemap can't go above a threshold)
	// TODO: Actually scrolly registers 0xf0 to 0xff are used (can split the tilemap furthermore?)
	uint16_t page_select = scrolly[0xff];

	for (int y=cliprect.min_y; y<=cliprect.max_y; y++)
	{
		clip.min_y = clip.max_y = y;
		int y_base = rowscroll_enable ? y : 0;
		int x_base = colscroll_enable ? y : 0;
		int src_y = (scrolly[y_base] & 0x7ff);
		int src_x = (scrollx[x_base] & 0x7ff);
		// apparently if this is on disables wraparound target
		int page_disable = scrolly[y_base] & 0x800;
		int cur_page = src_y + y;

		// special handling for cycle warriors road: it reads in scrolly table bits 15-13 an
		// additional tile color bank and per scanline.
		if(is_road == true)
		{
			if(scrolly[y_base] & 0x8000)
			{
				m_road_color_bank = (scrolly[y_base] >> 13) & 3;
				// road mode disables page wraparound
				page_disable = 1;
			}
			else
				m_road_color_bank = 0;

			if(m_road_color_bank != m_prev_road_bank)
			{
				m_prev_road_bank = m_road_color_bank;
				src->mark_all_dirty();
			}
		}

		// apply wraparound, if enabled tilemaps can't go above a certain threshold
		// cfr. Cycle Warriors scrolling text (ranking, ending), backgrounds when uphill,
		// Big Fight vertical scrolling in the morning Funnel stage (not the one chosen at start),
		// also Big Fight text garbage in the stage after Mevella joins you (forgot the name)
		if((cur_page - page_select) >= layer_page_size && page_disable == 0)
			src_y -= layer_page_size;

		src->set_scrollx(0,src_x);
		src->set_scrolly(0,src_y);
		src->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(hi_priority), 0);
	}
}

void cyclwarr_state::draw_bg_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int hi_priority)
{
	draw_bg(screen, bitmap, cliprect, m_layer[3], &m_cyclwarr_videoram[1][0x000], &m_cyclwarr_videoram[1][0x100], m_layer_page_size[3], false, hi_priority);
	draw_bg(screen, bitmap, cliprect, m_layer[2], &m_cyclwarr_videoram[1][0x200], &m_cyclwarr_videoram[1][0x300], m_layer_page_size[2],false, hi_priority);
	draw_bg(screen, bitmap, cliprect, m_layer[1], &m_cyclwarr_videoram[0][0x000], &m_cyclwarr_videoram[0][0x100], m_layer_page_size[1],m_layer1_can_be_road, hi_priority);
	draw_bg(screen, bitmap, cliprect, m_layer[0], &m_cyclwarr_videoram[0][0x200], &m_cyclwarr_videoram[0][0x300], m_layer_page_size[0], false, hi_priority);
}

uint32_t cyclwarr_state::screen_update_cyclwarr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bigfight_bank=m_bigfight_a40000[0];
	if (m_bigfight_bank!=m_bigfight_last_bank)
	{
		for (int i = 0; i < 4; i++)
		{
			m_layer[i]->mark_all_dirty();
		}
		m_bigfight_last_bank=m_bigfight_bank;
	}
	m_rotatingsprites->update_cluts();

	bitmap.fill(m_palette->pen(0), cliprect);

#if 0
	popmessage("%04x %04x (%04x)|%04x %04x (%04x)|%04x %04x (%04x)|%04x %04x (%04x)"
														,m_cyclwarr_videoram[1][0x000],m_cyclwarr_videoram[1][0x100],m_cyclwarr_videoram[1][0x1ff]
														,m_cyclwarr_videoram[1][0x200],m_cyclwarr_videoram[1][0x300],m_cyclwarr_videoram[1][0x3ff]
														,m_cyclwarr_videoram[0][0x000],m_cyclwarr_videoram[0][0x100],m_cyclwarr_videoram[0][0x1ff]
														,m_cyclwarr_videoram[0][0x200],m_cyclwarr_videoram[0][0x300],m_cyclwarr_videoram[0][0x3ff]);
#endif

//  popmessage("%04x %04x %04x %04x",m_video_config[0],m_video_config[1],m_video_config[2],m_video_config[3]);

	screen.priority().fill(0, cliprect);
	m_rotatingsprites->draw_sprites(screen.priority(),cliprect,1,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Alpha pass only
	draw_bg_layers(screen, bitmap, cliprect, 0);
	apply_shadow_bitmap(bitmap,cliprect,screen.priority(), m_mixing_control & 1);
	m_rotatingsprites->draw_sprites(bitmap,cliprect,0,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0);
	draw_bg_layers(screen, bitmap, cliprect, 1);
	return 0;
}
