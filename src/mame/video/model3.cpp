// license:BSD-3-Clause
// copyright-holders:R. Belmont, Ville Linde
#include "emu.h"
#include "video/rgbutil.h"
#include "includes/model3.h"

/*
    TODO:
    - Tilemap flash effect
    - Fog
    - Mipmapping
    - Mipmap uploads smaller than a tile
    - Some of the 4-bit and 8-bit textures need their alpha values rechecked
    - Spotlights
    - Recheck normal vector transform

*/

#define ENABLE_BILINEAR     1

#define TRI_PARAM_TEXTURE_PAGE          0x1
#define TRI_PARAM_TEXTURE_MIRROR_U      0x2
#define TRI_PARAM_TEXTURE_MIRROR_V      0x4
#define TRI_PARAM_TEXTURE_ENABLE        0x8
#define TRI_PARAM_ALPHA_TEST            0x10
#define TRI_PARAM_COLOR_MOD             0x20

#define TRI_BUFFER_SIZE                 50000
#define TRI_ALPHA_BUFFER_SIZE           15000

struct model3_polydata
{
	cached_texture *texture;
	UINT32 color;
	UINT32 texture_param;
	int transparency;
	int intensity;
};

class model3_renderer : public poly_manager<float, model3_polydata, 6, 50000>
{
public:
	model3_renderer(model3_state &state, int width, int height)
		: poly_manager<float, model3_polydata, 6, 50000>(state.machine())
	{
		m_fb = std::make_unique<bitmap_rgb32>(width, height);
		m_zb = std::make_unique<bitmap_ind32>(width, height);
	}

	void draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_opaque_triangles(const m3_triangle* tris, int num_tris);
	void draw_alpha_triangles(const m3_triangle* tris, int num_tris);
	void clear_fb();
	void clear_zb();
	void draw_scanline_solid(INT32 scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_solid_trans(INT32 scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex(INT32 scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex_colormod(INT32 scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex_contour(INT32 scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex_trans(INT32 scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex_alpha(INT32 scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void wait_for_polys();

private:
	std::unique_ptr<bitmap_rgb32> m_fb;
	std::unique_ptr<bitmap_ind32> m_zb;
};



/*****************************************************************************/

/* matrix stack */
#define MATRIX_STACK_SIZE   256


#define BYTE_REVERSE32(x)       (((x >> 24) & 0xff) | \
								((x >> 8) & 0xff00) | \
								((x << 8) & 0xff0000) | \
								((x << 24) & 0xff000000))

#define BYTE_REVERSE16(x)       (((x >> 8) & 0xff) | ((x << 8) & 0xff00))


void model3_state::model3_exit()
{
#if 0
	FILE* file;
	int i;
	file = fopen("m3_texture_ram.bin","wb");
	for (i=0; i < 0x200000; i++)
	{
		fputc((UINT8)(m_texture_ram[0][i] >> 8), file);
		fputc((UINT8)(m_texture_ram[0][i] >> 0), file);
	}
	for (i=0; i < 0x200000; i++)
	{
		fputc((UINT8)(m_texture_ram[1][i] >> 8), file);
		fputc((UINT8)(m_texture_ram[1][i] >> 0), file);
	}
	fclose(file);

	file = fopen("m3_displist.bin","wb");
	for (i=0; i < 0x40000; i++)
	{
		fputc((UINT8)(m_display_list_ram[i] >> 24), file);
		fputc((UINT8)(m_display_list_ram[i] >> 16), file);
		fputc((UINT8)(m_display_list_ram[i] >> 8), file);
		fputc((UINT8)(m_display_list_ram[i] >> 0), file);
	}
	fclose(file);

	file = fopen("m3_culling_ram.bin","wb");
	for (i=0; i < 0x100000; i++)
	{
		fputc((UINT8)(m_culling_ram[i] >> 24), file);
		fputc((UINT8)(m_culling_ram[i] >> 16), file);
		fputc((UINT8)(m_culling_ram[i] >> 8), file);
		fputc((UINT8)(m_culling_ram[i] >> 0), file);
	}
	fclose(file);

	file = fopen("m3_polygon_ram.bin","wb");
	for (i=0; i < 0x100000; i++)
	{
		fputc((UINT8)(m_polygon_ram[i] >> 24), file);
		fputc((UINT8)(m_polygon_ram[i] >> 16), file);
		fputc((UINT8)(m_polygon_ram[i] >> 8), file);
		fputc((UINT8)(m_polygon_ram[i] >> 0), file);
	}
	fclose(file);

	file = fopen("m3_vrom.bin","wb");
	for (i=0; i < 0x1000000; i++)
	{
		fputc((UINT8)(m_vrom[i] >> 24), file);
		fputc((UINT8)(m_vrom[i] >> 16), file);
		fputc((UINT8)(m_vrom[i] >> 8), file);
		fputc((UINT8)(m_vrom[i] >> 0), file);
	}
	fclose(file);
#endif

//  invalidate_texture(0, 0, 0, 6, 5);
//  invalidate_texture(1, 0, 0, 6, 5);
}

void model3_state::video_start()
{
	static const gfx_layout char4_layout =
	{
		8, 8,
		31744,
		4,
		{ 0,1,2,3 },
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
		{ 1*32, 0*32, 3*32, 2*32, 5*32, 4*32, 7*32, 6*32 },
		4 * 8*8
	};

	static const gfx_layout char8_layout =
	{
		8, 8,
		15872,
		8,
		{ 0,1,2,3,4,5,6,7 },
		{ 4*8, 5*8, 6*8, 7*8, 0*8, 1*8, 2*8, 3*8 },
		{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
		8 * 8*8
	};

	int width = m_screen->width();
	int height = m_screen->height();

	m_renderer = auto_alloc(machine(), model3_renderer(*this, width, height));

	m_tri_buffer = auto_alloc_array_clear(machine(), m3_triangle, TRI_BUFFER_SIZE);
	m_tri_alpha_buffer = auto_alloc_array_clear(machine(), m3_triangle, TRI_ALPHA_BUFFER_SIZE);

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(model3_state::model3_exit), this));

	m_m3_char_ram = make_unique_clear<UINT64[]>(0x100000/8);
	m_m3_tile_ram = make_unique_clear<UINT64[]>(0x8000/8);

	m_texture_fifo = make_unique_clear<UINT32[]>(0x100000/4);

	/* 2x 4MB texture sheets */
	m_texture_ram[0] = std::make_unique<UINT16[]>(0x400000/2);
	m_texture_ram[1] = std::make_unique<UINT16[]>(0x400000/2);

	/* 1MB Display List RAM */
	m_display_list_ram = make_unique_clear<UINT32[]>(0x100000/4);
	/* 4MB for nodes (< Step 2.0 have only 2MB) */
	m_culling_ram = make_unique_clear<UINT32[]>(0x400000/4);
	/* 4MB Polygon RAM */
	m_polygon_ram = make_unique_clear<UINT32[]>(0x400000/4);

	m_vid_reg0 = 0;

	m_layer4[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(model3_state::tile_info_layer0_4bit), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_layer8[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(model3_state::tile_info_layer0_8bit), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_layer4[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(model3_state::tile_info_layer1_4bit), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_layer8[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(model3_state::tile_info_layer1_8bit), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_layer4[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(model3_state::tile_info_layer2_4bit), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_layer8[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(model3_state::tile_info_layer2_8bit), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_layer4[3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(model3_state::tile_info_layer3_4bit), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_layer8[3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(model3_state::tile_info_layer3_8bit), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	// 4-bit tiles
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, char4_layout, (UINT8*)m_m3_char_ram.get(), 0, m_palette->entries() / 16, 0));

	// 8-bit tiles
	m_gfxdecode->set_gfx(1, std::make_unique<gfx_element>(m_palette, char8_layout, (UINT8*)m_m3_char_ram.get(), 0, m_palette->entries() / 256, 0));

	init_matrix_stack();
}

#define MODEL3_TILE_INFO4(address)  \
do { \
	UINT16 *tiles = (UINT16*)&m_m3_tile_ram[address + (tile_index / 4)];    \
	UINT16 t = BYTE_REVERSE16(tiles[(tile_index & 3) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0)]); \
	int tile = ((t << 1) & 0x7ffe) | ((t >> 15) & 0x1); \
	int color = (t & 0x7ff0) >> 4; \
	SET_TILE_INFO_MEMBER(0, tile, color, 0); \
} while (0)

#define MODEL3_TILE_INFO8(address)  \
do { \
	UINT16 *tiles = (UINT16*)&m_m3_tile_ram[address + (tile_index / 4)];    \
	UINT16 t = BYTE_REVERSE16(tiles[(tile_index & 3) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0)]); \
	int tile = ((t << 1) & 0x7ffe) | ((t >> 15) & 0x1); \
	int color = (t & 0x7f00) >> 8; \
	SET_TILE_INFO_MEMBER(1, tile >> 1, color, 0); \
} while (0)

TILE_GET_INFO_MEMBER(model3_state::tile_info_layer0_4bit) { MODEL3_TILE_INFO4(0x000); }
TILE_GET_INFO_MEMBER(model3_state::tile_info_layer0_8bit) { MODEL3_TILE_INFO8(0x000); }
TILE_GET_INFO_MEMBER(model3_state::tile_info_layer1_4bit) { MODEL3_TILE_INFO4(0x400); }
TILE_GET_INFO_MEMBER(model3_state::tile_info_layer1_8bit) { MODEL3_TILE_INFO8(0x400); }
TILE_GET_INFO_MEMBER(model3_state::tile_info_layer2_4bit) { MODEL3_TILE_INFO4(0x800); }
TILE_GET_INFO_MEMBER(model3_state::tile_info_layer2_8bit) { MODEL3_TILE_INFO8(0x800); }
TILE_GET_INFO_MEMBER(model3_state::tile_info_layer3_4bit) { MODEL3_TILE_INFO4(0xc00); }
TILE_GET_INFO_MEMBER(model3_state::tile_info_layer3_8bit) { MODEL3_TILE_INFO8(0xc00); }

#ifdef UNUSED_FUNCTION
void model3_state::draw_texture_sheet(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	for(y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *d = &bitmap.pix16(y);
		int index = (y*2)*2048;
		for(x = cliprect.min_x; x <= cliprect.max_x; x++) {
			UINT16 pix = m_texture_ram[0][index];
			index+=4;
			if(pix != 0) {
				d[x] = pix;
			}
		}
	}
}
#endif

void model3_state::draw_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int sx, int sy, int prio)
{
	int bitdepth = (m_layer_priority & (0x10 << layer)) ? 1 : 0;
//  int layer_prio = (m_layer_priority & (0x1 << layer)) ? 1 : 0;

	tilemap_t *tmap = bitdepth ? m_layer4[layer] : m_layer8[layer];
	bitmap_ind16 &pixmap = tmap->pixmap();
	const pen_t *pens = m_palette->pens();

	UINT32* palram = (UINT32*)&m_paletteram64[0];
	UINT16* rowscroll_ram = (UINT16*)&m_m3_char_ram[0x1ec00];
	UINT32* rowmask_ram = (UINT32*)&m_m3_char_ram[0x1ee00];

	int x1 = cliprect.min_x;
	int y1 = cliprect.min_y;
	int x2 = cliprect.max_x;
	int y2 = cliprect.max_y;

	int ix = sx;
	int iy = sy;
	if (ix < 0)
	{
		ix = 0 - ix;
	}
	if (iy < 0)
	{
		iy = 0 - iy;
	}

	for (int y = y1; y <= y2; y++)
	{
		UINT32* dst = &bitmap.pix32(y);
		UINT16* src = &pixmap.pix16(iy & 0x1ff);

		int rowscroll = BYTE_REVERSE16(rowscroll_ram[((layer * 0x200) + y) ^ NATIVE_ENDIAN_VALUE_LE_BE(3,0)]) & 0x7fff;
		if (rowscroll & 0x100)
			rowscroll |= ~0x1ff;

		UINT16 rowmask;
		if (prio && (layer == 1 || layer == 2))
			rowmask = BYTE_REVERSE32(rowmask_ram[(y & 0x1ff) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)]) & 0xffff;
		else
			rowmask = 0xffff;

		int iix = ix & 0x1ff;

		int rx1 = x1 - (rowscroll * 2);
		int rx2 = x2 - (rowscroll * 2);

		if (rx1 < 0)
		{
			iix += (0 - rx1);
			rx1 = 0;
		}
		if (rx2 > cliprect.max_x)
			rx2 = cliprect.max_x;

		for (int x = rx1; x <= rx2; x++)
		{
			UINT32 mask = rowmask & (1 << ((iix & 0x1ff) >> 5));

			if (mask)
			{
				UINT16 p0 = src[iix & 0x1ff];
				if ((palram[p0^NATIVE_ENDIAN_VALUE_LE_BE(1,0)] & NATIVE_ENDIAN_VALUE_LE_BE(0x00800000,0x00008000)) == 0)
				{
					dst[x] = pens[p0];
				}
			}
			iix++;
		}

		iy++;
	}
}

UINT32 model3_state::screen_update_model3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int layer_scroll_x[4], layer_scroll_y[4];
	UINT32 layer_data[4];

	layer_data[0] = BYTE_REVERSE32((UINT32)(m_layer_scroll[0] >> 32));
	layer_data[1] = BYTE_REVERSE32((UINT32)(m_layer_scroll[0] >> 0));
	layer_data[2] = BYTE_REVERSE32((UINT32)(m_layer_scroll[1] >> 32));
	layer_data[3] = BYTE_REVERSE32((UINT32)(m_layer_scroll[1] >> 0));
	layer_scroll_x[0] = layer_data[0] & 0x1ff;
	layer_scroll_y[0] = (layer_data[0] >> 16) & 0x1ff;
	layer_scroll_x[1] = layer_data[1] & 0x1ff;
	layer_scroll_y[1] = (layer_data[1] >> 16) & 0x1ff;
	layer_scroll_x[2] = layer_data[2] & 0x1ff;
	layer_scroll_y[2] = (layer_data[2] >> 16) & 0x1ff;
	layer_scroll_x[3] = layer_data[3] & 0x1ff;
	layer_scroll_y[3] = (layer_data[3] >> 16) & 0x1ff;

	m_screen_clip = (rectangle*)&cliprect;
	m_clip3d = cliprect;

	bitmap.fill(0, cliprect);

	// render enabled layers with priority 0
	if ((layer_data[3] & 0x80000000) && (m_layer_priority & 0x8) == 0)
		draw_layer(bitmap, cliprect, 3, layer_scroll_x[3], layer_scroll_y[3], 0);
	if ((layer_data[2] & 0x80000000) && (m_layer_priority & 0x4) == 0)
		draw_layer(bitmap, cliprect, 2, layer_scroll_x[2], layer_scroll_y[2], 0);
	if ((layer_data[1] & 0x80000000) && (m_layer_priority & 0x2) == 0)
		draw_layer(bitmap, cliprect, 1, layer_scroll_x[1], layer_scroll_y[1], 0);
	if ((layer_data[0] & 0x80000000) && (m_layer_priority & 0x1) == 0)
		draw_layer(bitmap, cliprect, 0, layer_scroll_x[0], layer_scroll_y[0], 0);

	// render 3D
	m_renderer->draw(bitmap, cliprect);

	// render enabled layers with priority 1
	if ((layer_data[3] & 0x80000000) && (m_layer_priority & 0x8) != 0)
		draw_layer(bitmap, cliprect, 3, layer_scroll_x[3], layer_scroll_y[3], 1);
	if ((layer_data[2] & 0x80000000) && (m_layer_priority & 0x4) != 0)
		draw_layer(bitmap, cliprect, 2, layer_scroll_x[2], layer_scroll_y[2], 1);
	if ((layer_data[0] & 0x80000000) && (m_layer_priority & 0x1) != 0)
		draw_layer(bitmap, cliprect, 0, layer_scroll_x[0], layer_scroll_y[0], 1);
	if ((layer_data[1] & 0x80000000) && (m_layer_priority & 0x2) != 0)
		draw_layer(bitmap, cliprect, 1, layer_scroll_x[1], layer_scroll_y[1], 1);

	return 0;
}



READ64_MEMBER(model3_state::model3_char_r)
{
	return m_m3_char_ram[offset];
}

WRITE64_MEMBER(model3_state::model3_char_w)
{
	COMBINE_DATA(&m_m3_char_ram[offset]);
	m_gfxdecode->gfx(0)->mark_dirty(offset / 4);
	m_gfxdecode->gfx(1)->mark_dirty(offset / 8);
}

READ64_MEMBER(model3_state::model3_tile_r)
{
	return m_m3_tile_ram[offset];
}

WRITE64_MEMBER(model3_state::model3_tile_w)
{
	COMBINE_DATA(&m_m3_tile_ram[offset]);

	/*
	m_layer4[0]->mark_all_dirty();
	m_layer8[0]->mark_all_dirty();
	m_layer4[1]->mark_all_dirty();
	m_layer8[1]->mark_all_dirty();
	m_layer4[2]->mark_all_dirty();
	m_layer8[2]->mark_all_dirty();
	m_layer4[3]->mark_all_dirty();
	m_layer8[3]->mark_all_dirty();
	*/

	int layer = (offset >> 10) & 0x3;
	int tile = (offset & 0x3ff) * 4;
	m_layer4[layer]->mark_tile_dirty(tile+0);
	m_layer4[layer]->mark_tile_dirty(tile+1);
	m_layer4[layer]->mark_tile_dirty(tile+2);
	m_layer4[layer]->mark_tile_dirty(tile+3);
	m_layer8[layer]->mark_tile_dirty(tile+0);
	m_layer8[layer]->mark_tile_dirty(tile+1);
	m_layer8[layer]->mark_tile_dirty(tile+2);
	m_layer8[layer]->mark_tile_dirty(tile+3);
}

/*
    Video registers:

    0xF1180000:         ?
    0xF1180004:         ?
    0xF1180008:         ?                                   lostwsga: writes 0x7f010000
                                                            lemans24, magtruck, von2, lamachin: writes 0xee000000
                                                            bass, vs2, harley, scud, skichamp, fvipers2, eca: writes 0xef000000
                                                            srally2, swtrilgy: writes 0x70010000
                                                            daytona2: writes 0x4f010000

    0xF1180010:                                             VBL IRQ acknowledge

    0xF1180020:         xxxxxxxx -------- -------- -------- ?
                        -------- x------- -------- -------- Layer 3 bitdepth (0 = 8-bit, 1 = 4-bit)
                        -------- -x------ -------- -------- Layer 2 bitdepth (0 = 8-bit, 1 = 4-bit)
                        -------- --x----- -------- -------- Layer 1 bitdepth (0 = 8-bit, 1 = 4-bit)
                        -------- ---x---- -------- -------- Layer 0 bitdepth (0 = 8-bit, 1 = 4-bit)
                        -------- ----x--- -------- -------- Layer 3 priority (0 = below 3D, 1 = above 3D)
                        -------- -----x-- -------- -------- Layer 2 priority (0 = below 3D, 1 = above 3D)
                        -------- ------x- -------- -------- Layer 1 priority (0 = below 3D, 1 = above 3D)
                        -------- -------x -------- -------- Layer 0 priority (0 = below 3D, 1 = above 3D)

    0xF1180040:                                             Foreground layer color modulation?
                        -------- xxxxxxxx -------- -------- Red component
                        -------- -------- xxxxxxxx -------- Green component
                        -------- -------- -------- xxxxxxxx Blue component

    0xF1180044:                                             Background layer color modulation?
                        -------- xxxxxxxx -------- -------- Red component
                        -------- -------- xxxxxxxx -------- Green component
                        -------- -------- -------- xxxxxxxx Blue component

    0xF1180060:         x------- -------- -------- -------- Layer 0 enable
                        -------x xxxxxxxx -------- -------- Layer 0 Y scroll position
                        -------- -------- -------x xxxxxxxx Layer 0 X scroll position

    0xF1180064:         x------- -------- -------- -------- Layer 1 enable
                        -------x xxxxxxxx -------- -------- Layer 1 Y scroll position
                        -------- -------- -------x xxxxxxxx Layer 1 X scroll position

    0xF1180068:         x------- -------- -------- -------- Layer 2 enable
                        -------x xxxxxxxx -------- -------- Layer 2 Y scroll position
                        -------- -------- -------x xxxxxxxx Layer 2 X scroll position

    0xF118006C:         x------- -------- -------- -------- Layer 3 enable
                        -------x xxxxxxxx -------- -------- Layer 3 Y scroll position
                        -------- -------- -------x xxxxxxxx Layer 3 X scroll position
*/


READ64_MEMBER(model3_state::model3_vid_reg_r)
{
	switch(offset)
	{
		case 0x00/8:    return m_vid_reg0;
		case 0x08/8:    return U64(0xffffffffffffffff);     /* ??? */
		case 0x20/8:    return (UINT64)m_layer_priority << 48;
		case 0x40/8:    return ((UINT64)m_layer_modulate1 << 32) | (UINT64)m_layer_modulate2;
		default:        logerror("read reg %02X\n", offset);break;
	}
	return 0;
}

WRITE64_MEMBER(model3_state::model3_vid_reg_w)
{
	switch(offset)
	{
		case 0x00/8:    logerror("vid_reg0: %08X%08X\n", (UINT32)(data>>32),(UINT32)(data)); m_vid_reg0 = data; break;
		case 0x08/8:    break;      /* ??? */
		case 0x10/8:    set_irq_line((data >> 56) & 0x0f, CLEAR_LINE); break;     /* VBL IRQ Ack */

		case 0x20/8:    m_layer_priority = (data >> 48); break;

		case 0x40/8:    m_layer_modulate1 = (UINT32)(data >> 32);
						m_layer_modulate2 = (UINT32)(data);
						break;
		case 0x60/8:    COMBINE_DATA(&m_layer_scroll[0]); break;
		case 0x68/8:    COMBINE_DATA(&m_layer_scroll[1]); break;
		default:        logerror("model3_vid_reg_w: %02X, %08X%08X\n", offset, (UINT32)(data >> 32), (UINT32)(data)); break;
	}
}

WRITE64_MEMBER(model3_state::model3_palette_w)
{
	COMBINE_DATA(&m_paletteram64[offset]);
	UINT32 data1 = BYTE_REVERSE32((UINT32)(m_paletteram64[offset] >> 32));
	UINT32 data2 = BYTE_REVERSE32((UINT32)(m_paletteram64[offset] >> 0));

	m_palette->set_pen_color((offset*2)+0, pal5bit(data1 >> 0), pal5bit(data1 >> 5), pal5bit(data1 >> 10));
	m_palette->set_pen_color((offset*2)+1, pal5bit(data2 >> 0), pal5bit(data2 >> 5), pal5bit(data2 >> 10));
}

READ64_MEMBER(model3_state::model3_palette_r)
{
	return m_paletteram64[offset];
}


/*****************************************************************************/
/* texture caching */

/*
    array of cached textures:
        4 potential textures for 4-bit grayscale
        2 pages
        1024 pixels / 32 pixel resolution vertically
        2048 pixels / 32 pixel resolution horizontally
*/
void model3_state::invalidate_texture(int page, int texx, int texy, int texwidth, int texheight)
{
	int wtiles = 1 << texwidth;
	int htiles = 1 << texheight;

	for (int y = 0; y < htiles; y++)
		for (int x = 0; x < wtiles; x++)
			while (m_texcache[page][texy + y][texx + x] != nullptr)
			{
				cached_texture *freeme = m_texcache[page][texy + y][texx + x];
				m_texcache[page][texy + y][texx + x] = freeme->next;
				auto_free(machine(), freeme);
			}
}

cached_texture *model3_state::get_texture(int page, int texx, int texy, int texwidth, int texheight, int format)
{
	cached_texture *tex = m_texcache[page][texy][texx];
	int pixheight = 32 << texheight;
	int pixwidth = 32 << texwidth;
	UINT32 alpha = ~0;
	int x, y;

	/* if we have one already, validate it */
	for (tex = m_texcache[page][texy][texx]; tex != nullptr; tex = tex->next)
		if (tex->width == texwidth && tex->height == texheight && tex->format == format)
			return tex;

	/* create a new texture */
	tex = (cached_texture *)auto_alloc_array(machine(), UINT8, sizeof(cached_texture) + (2 * pixwidth * 2 * pixheight) * sizeof(rgb_t));
	tex->width = texwidth;
	tex->height = texheight;
	tex->format = format;

	/* set the new texture */
	tex->next = m_texcache[page][texy][texx];
	m_texcache[page][texy][texx] = tex;

	/* decode it */
	for (y = 0; y < pixheight; y++)
	{
		const UINT16 *texsrc = &m_texture_ram[page][(texy * 32 + y) * 2048 + texx * 32];
		rgb_t *dest = tex->data + 2 * pixwidth * y;

		switch (format)
		{
			case 0:     /* 1-5-5-5 ARGB */
				for (x = 0; x < pixwidth; x++)
				{
					UINT16 pixdata = texsrc[x];
					alpha &= dest[x] = rgb_t(pal1bit(~pixdata >> 15), pal5bit(pixdata >> 10), pal5bit(pixdata >> 5), pal5bit(pixdata >> 0));
				}
				break;

			case 1:     /* A4L4 interleaved */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = pal4bit(texsrc[x] & 0xf);
					UINT8 a = pal4bit((texsrc[x] >> 4) & 0xf);
					alpha &= dest[x] = rgb_t(a, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 2:     /* A4L4? */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = pal4bit((texsrc[x] >> 0) & 0xf);
					UINT8 a = pal4bit((texsrc[x] >> 4) & 0xf);
					alpha &= dest[x] = rgb_t(a, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 3:     /* A4L4 interleaved */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = pal4bit((texsrc[x] >> 8) & 0xf);
					UINT8 a = pal4bit((texsrc[x] >> 12) & 0xf);
					alpha &= dest[x] = rgb_t(a, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 4:     /* 8-bit A4L4 */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 pixdata = texsrc[x] >> 8;
					alpha &= dest[x] = rgb_t(pal4bit(pixdata), pal4bit(pixdata >> 4), pal4bit(pixdata >> 4), pal4bit(pixdata >> 4));
				}
				break;

			case 5:     /* L8 */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = texsrc[x];
					alpha &= dest[x] = rgb_t(0xff, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 6:     /* L8 */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = texsrc[x] >> 8;
					alpha &= dest[x] = rgb_t(0xff, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 7:     /* 4-4-4-4 ARGB */
				for (x = 0; x < pixwidth; x++)
				{
					UINT16 pixdata = texsrc[x];
					alpha &= dest[x] = rgb_t(pal4bit(pixdata >> 0), pal4bit(pixdata >> 12), pal4bit(pixdata >> 8), pal4bit(pixdata >> 4));
				}
				break;
		}

		/* create the horizontal mirror of this line */
		for (x = 0; x < pixwidth; x++)
			dest[pixwidth * 2 - 1 - x] = dest[x];
	}

	/* create the vertical mirror of the texture */
	for (y = 0; y < pixheight; y++)
		memcpy(tex->data + 2 * pixwidth * (pixheight * 2 - 1 - y), tex->data + 2 * pixwidth * y, sizeof(rgb_t) * pixwidth * 2);

	/* remember the overall alpha */
	tex->alpha = alpha >> 24;

	/* return a pointer to the texture */
	return tex;
}

/*****************************************************************************/
/* Real3D Graphics stuff */

/*
    Real3D Pro-1000 capabilities:

    Coordinate sets
    - 4096 matrices (matrix base pointer in viewport node)

    Polygons
    - 32MB max polygon memory. VROM in Model 3, the low 4MB of VROM is overlaid by Polygon RAM for runtime generated content.

    Texture
    - 2 texture sheets of 2048x1024
    - Mipmaps located in the bottom right corner
    - Texture size 32x32 to 1024x1024
    - Microtextures (is this featured in Model 3?)

    LODs
    - 127 blend types per viewport with 4 sets of min/max angle or range (where is this in the viewport node?)

    Lighting
    - Self-luminous lighting (enable and luminosity parameter in polygon structure)
    - Fixed polygon shading, fixed shading weight per vertex (not found in Model 3, yet)
    - Flat sun shading (lighting parameters in viewport node) needs a separate enable?
    - Smooth polygon shading (lighting parameters in viewport, use vertex normals)

    Gamma table
    - 256 entry 8-bit table, possibly in Polygon RAM
*/

/*
    Real3D Memory Structures:

    Culling Nodes:
    - Located in Culling RAM (0x8E000000)
    - Limit of 15 child nodes (nesting), not including polygon nodes
    - Color table (is this featured in Model 3?)

    0x00:   -------- -------- ------xx -------- Viewport number 0-3
            -------- -------- -------- ---xx--- Viewport priority

    0x01:   Child node pointer (inherits parameters from this node)
    0x02:   Sibling node pointer
    0x03:   (float) Focal length? Affected by frustum angles and viewport size
    0x04:   Sun light vector Z-component (float)
    0x05:   Sun light vector X-component (float)
    0x06:   Sun light vector Y-component (float)
    0x07:   Sun light intensity (float)
    0x08:   ? (float) Affected by left and right angle
    0x09:   ? (float) Affected by top and bottom angle
    0x0a:   ? (float) Affected by top and bottom angle
    0x0b:   ? (float) Affected by left and right angle
    0x0c:   (float) Frustum Left Angle Y (these angles are defined in polar coordinates)
    0x0d:   (float) Frustum Left Angle X
    0x0e:   (float) Frustum Top Angle Y
    0x0f:   (float) Frustum Top Angle X
    0x10:   (float) Frustum Right Angle Y
    0x11:   (float) Frustum Right Angle X
    0x12:   (float) Frustum Bottom Angle Y
    0x13:   (float) Frustum Bottom Angle X

    0x14:   xxxxxxxx xxxxxxxx -------- -------- Viewport height (14.2 fixed-point)
            -------- -------- xxxxxxxx xxxxxxxx Viewport width (14.2 fixed-point)

    0x15:   ?
    0x16:   Matrix base pointer
    0x17:   LOD blend type table pointer?       (seems to be 8x float per entry)
    0x18:   ?
    0x19:   ?

    0x1a:   xxxxxxxx xxxxxxxx -------- -------- Viewport Y coordinate (12.4 fixed-point)
            -------- -------- xxxxxxxx xxxxxxxx Viewport X coordinate (12.4 fixed-point)

    0x1b:   Copy of word 0x00
    0x1c:   ?

    0x1d:   xxxxxxxx xxxxxxxx -------- -------- Spotlight Y size
            -------- -------- xxxxxxxx xxxxxxxx Spotlight Y position (13.3 fixed-point?)

    0x1e:   xxxxxxxx xxxxxxxx -------- -------- Spotlight X size
            -------- -------- xxxxxxxx xxxxxxxx Spotlight X position (13.3 fixed-point?)

    0x1f:   Light extent (float)

    0x20:   xxxxxxxx -------- -------- -------- ?
            -------- xxxxxxxx -------- -------- ?
            -------- -------- --xxx--- -------- Light RGB (RGB111?)
            -------- -------- -----xxx -------- Light RGB Fog (RGB111?)
            -------- -------- -------- xxxxxxxx Scroll Fog (0.8 fixed-point?) What is this???

    0x21:   ? seen 8.0, 0.125, 1000000000.0
    0x22:   Fog Color (RGB888)
    0x23:   Fog Density (float)

    0x24:   xxxxxxxx xxxxxxxx -------- -------- ?
            -------- -------- xxxxxxxx -------- Sun light ambient (0.8 fixed-point)
            -------- -------- -------- xxxxxxxx Scroll attenuation (0.8 fixed-point) What is this???

    0x25:   Fog offset
    0x26:   ?
    0x27:   ?
    0x28:   ?
    0x29:   ?
    0x2a:   ?
    0x2b:   ?
    0x2c:   ?
    0x2d:   ?
    0x2e:   ?
    0x2f:   ?


    Sub types:
    LOD Culling Node. Up to 4 LODs.

    Articulated Part Culling Node (is this used by Model 3?)
    - An Articulated Part culling node, or six degree?of?freedom node, is used to define
      geometry that can move relative to the parent coordinate set to which it is attached.
      Fifteen levels of coordinate set nesting (levels of articulation) are supported.

    Animation Culling Node
    - Animation culling nodes are used to build a culling hierarchy for an object with different
      representations, or animation frames. which can be turned on and off by the
      application. Each child (culling node or polygon) added to an Animation culling node
      specifies the frame for which the child is valid.

    Instance Culling Node
    - Instance culling nodes define the top of a shared display list segment that can be
      referenced from other parts of the scene display list.

    Instance Reference Culling Node
    - An Instance Reference node is considered a leaf node; its
      "child" is the shared geometry segment. An Instance Reference may be attached to
      a parent node and may not have any other children, but may have siblings.

    Point Light
    - A Point Light is used to create an instance of a point luminous feature. The size,
      feature type, and number of sides of the point light model may be customized.

    Instance Set
    - An Instance Set is a culling node which defines a set of point features. Each feature
      is positioned individually. This type of culling node can be used to simulate particles.



    Instance Node?

    0x00:   xxxxxxxx xxxxxxxx xxxxxx-- -------- Node number/ID?, num of bits unknown
            -------- -------- -------- ---x---- This node applies translation, else matrix
            -------- -------- -------- ----x--- LOD enable?
            -------- -------- -------- -----x-- ?
            -------- -------- -------- ------x- ?
            -------- -------- -------- -------x ?


    0x01:   ? (not present on Step 1.0)
    0x02:   ? (not present on Step 1.0)         Scud Race has 0x00000101

    0x03:   --x----- -------- -------- -------- ?
            -------- -xxxxxxx xxxx---- -------- LOD?
            -------- -------- ----xxxx xxxxxxxx Node matrix

    0x04:   Translation X coordinate
    0x05:   Translation Y coordinate
    0x06:   Translation Z coordinate
    0x07:   Child node pointer
    0x08:   Sibling node pointer

    0x09:   xxxxxxxx xxxxxxxx -------- -------- Culling or sorting related?
            -------- -------- xxxxxxxx xxxxxxxx Culling or sorting related?


    Polygon Data

    0x00:   x------- -------- -------- -------- Supermodel says specular enable
            -xxxxx-- -------- -------- -------- ?
            ------xx xxxxxxxx xxxxxx-- -------- Polygon ID
            -------- -------- -------- -x------ 0 = Triangle, 1 = Quad
            -------- -------- -------- ----x--- Vertex 3 shared from previous polygon
            -------- -------- -------- -----x-- Vertex 2 shared from previous polygon
            -------- -------- -------- ------x- Vertex 1 shared from previous polygon
            -------- -------- -------- -------x Vertex 0 shared from previous polygon
            -------- -------- -------- x-xx---- ?
            -------- -------- ------xx -------- Broken polygons in srally2 set these (a way to mark them for HW to not render?)

    0x01:   xxxxxxxx xxxxxxxx xxxxxxxx -------- Polygon normal X coordinate (2.22 fixed point)
            -------- -------- -------- -x------ UV format (0 = 13.3, 1 = 16.0)
            -------- -------- -------- ---x---- 1 = Double-sided polygon
            -------- -------- -------- -----x-- If set, this is the last polygon
            -------- -------- -------- ------x- Poly color, 1 = RGB, 0 = color table
            -------- -------- -------- x-x-x--x ?


    0x02:   xxxxxxxx xxxxxxxx xxxxxxxx -------- Polygon normal Y coordinate (2.22 fixed point)
            -------- -------- -------- ------x- Texture U mirror enable
            -------- -------- -------- -------x Texture V mirror enable
            -------- -------- -------- xxxxxx-- ?

    0x03:   xxxxxxxx xxxxxxxx xxxxxxxx -------- Polygon normal Z coordinate (2.22 fixed point)
            -------- -------- -------- --xxx--- Texture width (in 8-pixel tiles)
            -------- -------- -------- -----xxx Texture height (in 8-pixel tiles)

    0x04:   xxxxxxxx xxxxxxxx xxxxxxxx -------- Color (RGB888)
            -------- -------- -------- -x------ Texture page
            -------- -------- -------- ---xxxxx Upper 5 bits of texture U coordinate
            -------- -------- -------- x------- ?
            -------- -------- -------- --x----- ?

    0x05:   xxxxxxxx xxxxxxxx xxxxxxxx -------- Specular color?
            -------- -------- -------- x------- Low bit of texture U coordinate
            -------- -------- -------- ---xxxxx Low 5 bits of texture V coordinate
            -------- -------- -------- -xx----- ?

    0x06:   x------- -------- -------- -------- Texture contour enable
            -xxxxx-- -------- -------- -------- Fixed shading?
            ------x- -------- -------- -------- Enable fixed shading?
            -------x -------- -------- -------- Could be high priority polygon?
            -------- x------- -------- -------- 1 = disable transparency?
            -------- -xxxxx-- -------- -------- Polygon translucency (0 = fully transparent)
            -------- -------x -------- -------- 1 = disable lighting
            -------- -------- xxxxx--- -------- Polygon light modifier (Amount that a luminous polygon will burn through fog.
                                                                        Valid range is 0.0 to 1.0. 0.0 is completely fogged;
                                                                        1.0 has no fog.)
            -------- -------- -----x-- -------- Texture enable
            -------- -------- ------xx x------- Texture format
            -------- -------- -------- -------x Alpha enable?
            -------- ------x- -------- -------- Never seen set?
            -------- -------- -------- -----xx- Always set?
            -------- -------- -------- -xxxx--- ?

    TODO:   Bits to find (from Real3D dev guide):
            SetHighPriority(): Indicates that the polygon has higher priority than others in scene
            PolygonIsLayered(): Indicates a stencil polygon.
            DoSmoothShading(): Indicates that the polygon will be smooth shaded
            void SetNPScale ( float np_scale ) ; Sets the texture lod scale for the polygon. A value greater than 1 will increase the
                                                 range of the transition of texture level of detail.


    Vertex entry

    0x00:   xxxxxxxx xxxxxxxx xxxxxxxx -------- Vertex X coordinate (17.7 fixed-point in Step 1.0, 13.11 otherwise)
            -------- -------- -------- xxxxxxxx Vertex normal X (offset from polygon normal)

    0x01:   xxxxxxxx xxxxxxxx xxxxxxxx -------- Vertex Y coordinate
            -------- -------- -------- xxxxxxxx Vertex normal Y

    0x02:   xxxxxxxx xxxxxxxx xxxxxxxx -------- Vertex Z coordinate
            -------- -------- -------- xxxxxxxx Vertex normal Z

    0x03:   xxxxxxxx xxxxxxxx -------- -------- Vertex U coordinate
            -------- -------- xxxxxxxx xxxxxxxx Vertex V coordinate

*/


WRITE64_MEMBER(model3_state::real3d_display_list_w)
{
	if (ACCESSING_BITS_32_63)
	{
		m_display_list_ram[offset*2] = BYTE_REVERSE32((UINT32)(data >> 32));
	}
	if (ACCESSING_BITS_0_31)
	{
		m_display_list_ram[(offset*2)+1] = BYTE_REVERSE32((UINT32)(data));
	}
}

WRITE64_MEMBER(model3_state::real3d_polygon_ram_w)
{
	if (ACCESSING_BITS_32_63)
	{
		m_polygon_ram[offset*2] = BYTE_REVERSE32((UINT32)(data >> 32));
	}
	if (ACCESSING_BITS_0_31)
	{
		m_polygon_ram[(offset*2)+1] = BYTE_REVERSE32((UINT32)(data));
	}
}

static const UINT8 texture_decode16[64] =
{
		0,  1,  4,  5,  8,  9, 12, 13,
		2,  3,  6,  7, 10, 11, 14, 15,
	16, 17, 20, 21, 24, 25, 28, 29,
	18, 19, 22, 23, 26, 27, 30, 31,
	32, 33, 36, 37, 40, 41, 44, 45,
	34, 35, 38, 39, 42, 43, 46, 47,
	48, 49, 52, 53, 56, 57, 60, 61,
	50, 51, 54, 55, 58, 59, 62, 63
};

static const UINT8 texture_decode8[32] =
{
		1,  3,  5,  7,
		0,  2,  4,  6,
		9, 11, 13, 15,
		8, 10, 12, 14,
	17, 19, 21, 23,
	16, 18, 20, 22,
	25, 27, 29, 31,
	24, 26, 28, 30
};

inline void model3_state::write_texture16(int xpos, int ypos, int width, int height, int page, UINT16 *data)
{
	int x,y,i,j;

	for(y=ypos; y < ypos+height; y+=8)
	{
		for(x=xpos; x < xpos+width; x+=8)
		{
			UINT16 *texture = &m_texture_ram[page][y*2048+x];
			int b = 0;
			for(j=y; j < y+8; j++) {
				for(i=x; i < x+8; i++) {
					*texture++ = data[texture_decode16[b^1]];
					++b;
				}
				texture += 2048-8;
			}
			data += 64;
		}
	}
}

inline void model3_state::write_texture8(int xpos, int ypos, int width, int height, int page, int upper, int lower, UINT16 *data)
{
	int x,y,i,j;

	for(y=ypos; y < ypos+height; y+=8)
	{
		for(x=xpos; x < xpos+width; x+=8)
		{
			UINT16 *texture = &m_texture_ram[page][y*2048+x];
			int b = 0;
			for(j=y; j < y+8; j++)
			{
				for(i=x; i < x+8; i+=2)
				{
					UINT16 d = data[texture_decode8[b]];

					if (upper)
						*texture = (*texture & 0xff) | (d & 0xff00);
					if (lower)
						*texture = (*texture & 0xff00) | ((d >> 8) & 0xff);
					texture++;

					if (upper)
						*texture = (*texture & 0xff) | ((d & 0xff) << 8);
					if (lower)
						*texture = (*texture & 0xff00) | (d & 0xff);
					texture++;

					++b;
				}
				texture += 2048-8;
			}
			data += 32;
		}
	}
}

/*
    Texture header:

    -------- -------- -------- --xxxxxx X-position
    -------- -------- ----xxxx x------- Y-position
    -------- -------x xx------ -------- Width
    -------- ----xxx- -------- -------- Height
    -------- ---x---- -------- -------- Texture page
    -------- --x----- -------- -------- Write 8-bit data to the lower byte of texel
    -------- -x------ -------- -------- Write 8-bit data to the upper byte of texel
    -------- x------- -------- -------- Bitdepth, 0 = 8-bit, 1 = 16-bit
    xxxxxxxx -------- -------- -------- Texture type
                                            0x00 = texture with mipmaps
                                            0x01 = texture without mipmaps
                                            0x02 = only mipmaps
                                            0x80 = possibly gamma table

*/

static const int mipmap_coords[9][2] =
{
	{ 1024,  512 },
	{ 1536,  768 },
	{ 1792,  896 },
	{ 1920,  960 },
	{ 1984,  992 },
	{ 2016, 1008 },
	{ 2032, 1016 },
	{ 2040, 1020 },
	{ 2044, 1022 },
};

static const int mipmap_divider[9] = { 2, 4, 8, 16, 32, 64, 128, 256, 512 };

void model3_state::real3d_upload_texture(UINT32 header, UINT32 *data)
{
	int width   = 32 << ((header >> 14) & 0x7);
	int height  = 32 << ((header >> 17) & 0x7);
	int xpos    = (header & 0x3f) * 32;
	int ypos    = ((header >> 7) & 0x1f) * 32;
	int page    = (header >> 20) & 0x1;
	int bitdepth = (header >> 23) & 0x1;
	int upper_byte = (header >> 22) & 0x1;
	int lower_byte = (header >> 21) & 0x1;

	//printf("write tex: %08X, w %d, h %d, x %d, y %d, p %d, b %d\n", header, width, height, xpos, ypos, page, bitdepth);

	switch(header >> 24)
	{
		case 0x00:      /* Texture with mipmaps */
		{
			int x = xpos;
			int y = ypos;
			int w = width;
			int h = height;

			int mipmap = 0;

			while (w >= 8 && h >= 8)
			{
				if (bitdepth)
				{
					write_texture16(x, y, w, h, page, (UINT16*)data);
				}
				else
				{
					//printf("write tex8: %08X, w %d, h %d, x %d, y %d, p %d, b %d\n", header, width, height, xpos, ypos, page, bitdepth);
					write_texture8(x, y, w, h, page, upper_byte, lower_byte, (UINT16*)data);
				}

				data += (w * h * (bitdepth ? 2 : 1)) / 4;
				w /= 2;
				h /= 2;

				x = mipmap_coords[mipmap][0] + (xpos / mipmap_divider[mipmap]);
				y = mipmap_coords[mipmap][1] + (ypos / mipmap_divider[mipmap]);
				mipmap++;
			}

			invalidate_texture(page, header & 0x3f, (header >> 7) & 0x1f, (header >> 14) & 0x7, (header >> 17) & 0x7);
			break;
		}
		case 0x01:      /* Texture without mipmaps */
		{
			if (bitdepth)
			{
				write_texture16(xpos, ypos, width, height, page, (UINT16*)data);
			}
			else
			{
				//printf("write tex8: %08X, w %d, h %d, x %d, y %d, p %d, b %d\n", header, width, height, xpos, ypos, page, bitdepth);
				write_texture8(xpos, ypos, width, height, page, upper_byte, lower_byte, (UINT16*)data);
			}

			invalidate_texture(page, header & 0x3f, (header >> 7) & 0x1f, (header >> 14) & 0x7, (header >> 17) & 0x7);
			break;
		}
		case 0x02:      /* Only mipmaps */
		{
			int x = mipmap_coords[0][0] + (xpos / mipmap_divider[0]);
			int y = mipmap_coords[0][1] + (ypos / mipmap_divider[0]);
			int w = width / 2;
			int h = height / 2;

			int mipmap = 1;

			while (w >= 8 && h >= 8)
			{
				if (bitdepth)
				{
					write_texture16(x, y, w, h, page, (UINT16*)data);
				}
				else
				{
					//printf("write tex8: %08X, w %d, h %d, x %d, y %d, p %d, b %d\n", header, width, height, xpos, ypos, page, bitdepth);
					write_texture8(x, y, w, h, page, upper_byte, lower_byte, (UINT16*)data);
				}

				data += (w * h * (bitdepth ? 2 : 1)) / 4;
				w /= 2;
				h /= 2;

				x = mipmap_coords[mipmap][0] + (xpos / mipmap_divider[mipmap]);
				y = mipmap_coords[mipmap][1] + (ypos / mipmap_divider[mipmap]);
				mipmap++;
			}

			invalidate_texture(page, header & 0x3f, (header >> 7) & 0x1f, (header >> 14) & 0x7, (header >> 17) & 0x7);
			break;
		}
		case 0x80:      /* Gamma-table ? */
			break;
		default:
			fatalerror("Unknown texture type: %02X (%08X)\n", header >> 24, header);
			break;
	}
}

void model3_state::real3d_display_list_end()
{
	/* upload textures if there are any in the FIFO */
	if (m_texture_fifo_pos > 0)
	{
		int i = 0;
		while (i < m_texture_fifo_pos)
		{
			int length = (m_texture_fifo[i] / 2) + 2;
			UINT32 header = m_texture_fifo[i+1];
			real3d_upload_texture(header, &m_texture_fifo[i+2]);
			i += length;
		};
	}
	m_texture_fifo_pos = 0;

	m_renderer->clear_fb();

	reset_triangle_buffers();
	real3d_traverse_display_list();

	for (int i=0; i < 4; i++)
	{
		int ticount, tiacount;
		int ti = m_viewport_tri_index[i];
		int tia = m_viewport_tri_alpha_index[i];
		if (i < 3)
		{
			ticount = m_viewport_tri_index[i+1] - ti;
			tiacount = m_viewport_tri_alpha_index[i+1] - tia;
		}
		else
		{
			ticount = m_tri_buffer_ptr - ti;
			tiacount = m_tri_alpha_buffer_ptr - tia;
		}

		if (ticount > 0 || tiacount > 0)
		{
			m_renderer->clear_zb();
			m_renderer->draw_opaque_triangles(&m_tri_buffer[ti], ticount);
			m_renderer->draw_alpha_triangles(&m_tri_alpha_buffer[tia], tiacount);
			m_renderer->wait_for_polys();
		}
	}
}

void model3_state::real3d_display_list1_dma(UINT32 src, UINT32 dst, int length, int byteswap)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int d = (dst & 0xffffff) / 4;
	for (int i = 0; i < length; i += 4)
	{
		UINT32 w = space.read_dword(src);

		if (byteswap)
			w = BYTE_REVERSE32(w);

		m_display_list_ram[d++] = w;
		src += 4;
	}
}

void model3_state::real3d_display_list2_dma(UINT32 src, UINT32 dst, int length, int byteswap)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int d = (dst & 0xffffff) / 4;
	for (int i = 0; i < length; i += 4)
	{
		UINT32 w = space.read_dword(src);

		if (byteswap)
			w = BYTE_REVERSE32(w);

		m_culling_ram[d++] = w;
		src += 4;
	}
}

void model3_state::real3d_vrom_texture_dma(UINT32 src, UINT32 dst, int length, int byteswap)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	if ((dst & 0xff) == 0)
	{
		for (int i=0; i < length; i+=12)
		{
			UINT32 address = space.read_dword(src+i+0);
			UINT32 header = space.read_dword(src+i+4);

			if (byteswap)
			{
				address = BYTE_REVERSE32(address);
				header = BYTE_REVERSE32(header);
			}

			real3d_upload_texture(header, (UINT32*)&m_vrom[address]);
		}
	}
}

void model3_state::real3d_texture_fifo_dma(UINT32 src, int length, int byteswap)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	for (int i = 0; i < length; i += 4)
	{
		UINT32 w = space.read_dword(src);

		if (byteswap)
			w = BYTE_REVERSE32(w);

		m_texture_fifo[m_texture_fifo_pos] = w;
		m_texture_fifo_pos++;
		src += 4;
	}
}

void model3_state::real3d_polygon_ram_dma(UINT32 src, UINT32 dst, int length, int byteswap)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int d = (dst & 0xffffff) / 4;
	for (int i = 0; i < length; i += 4)
	{
		UINT32 w = space.read_dword(src);

		if (byteswap)
			w = BYTE_REVERSE32(w);

		m_polygon_ram[d++] = w;
		src += 4;
	}
}

WRITE64_MEMBER(model3_state::real3d_cmd_w)
{
	real3d_display_list_end();
}


/*****************************************************************************/
/* matrix and vector operations */

static inline float dot_product3(VECTOR3 a, VECTOR3 b)
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

/* multiplies a 4-element vector by a 4x4 matrix */
static void matrix_multiply_vector(MATRIX matrix, const VECTOR v, VECTOR *p)
{
	(*p)[0] = (v[0] * matrix[0][0]) + (v[1] * matrix[1][0]) + (v[2] * matrix[2][0]) + (v[3] * matrix[3][0]);
	(*p)[1] = (v[0] * matrix[0][1]) + (v[1] * matrix[1][1]) + (v[2] * matrix[2][1]) + (v[3] * matrix[3][1]);
	(*p)[2] = (v[0] * matrix[0][2]) + (v[1] * matrix[1][2]) + (v[2] * matrix[2][2]) + (v[3] * matrix[3][2]);
	(*p)[3] = (v[0] * matrix[0][3]) + (v[1] * matrix[1][3]) + (v[2] * matrix[2][3]) + (v[3] * matrix[3][3]);
}

/* multiplies a 4x4 matrix with another 4x4 matrix */
static void matrix_multiply(MATRIX a, MATRIX b, MATRIX *out)
{
	int i,j;
	MATRIX tmp;

	for( i=0; i < 4; i++ ) {
		for( j=0; j < 4; j++ ) {
			tmp[i][j] = (a[i][0] * b[0][j]) + (a[i][1] * b[1][j]) + (a[i][2] * b[2][j]) + (a[i][3] * b[3][j]);
		}
	}
	memcpy(out, &tmp, sizeof(MATRIX));
}

void model3_state::init_matrix_stack()
{
	MATRIX *matrix_stack;
	matrix_stack = m_matrix_stack = auto_alloc_array_clear(machine(), MATRIX, MATRIX_STACK_SIZE);

	/* initialize the first matrix as identity */
	matrix_stack[0][0][0] = 1.0f;
	matrix_stack[0][0][1] = 0.0f;
	matrix_stack[0][0][2] = 0.0f;
	matrix_stack[0][0][3] = 0.0f;
	matrix_stack[0][1][0] = 0.0f;
	matrix_stack[0][1][1] = 1.0f;
	matrix_stack[0][1][2] = 0.0f;
	matrix_stack[0][1][3] = 0.0f;
	matrix_stack[0][2][0] = 0.0f;
	matrix_stack[0][2][1] = 0.0f;
	matrix_stack[0][2][2] = 1.0f;
	matrix_stack[0][2][3] = 0.0f;
	matrix_stack[0][3][0] = 0.0f;
	matrix_stack[0][3][1] = 0.0f;
	matrix_stack[0][3][2] = 0.0f;
	matrix_stack[0][3][3] = 1.0f;

	m_matrix_stack_ptr = 0;
}

void model3_state::get_top_matrix(MATRIX *out)
{
	memcpy(out, &m_matrix_stack[m_matrix_stack_ptr], sizeof(MATRIX));
}

void model3_state::push_matrix_stack()
{
	m_matrix_stack_ptr++;
	if (m_matrix_stack_ptr >= MATRIX_STACK_SIZE)
		fatalerror("push_matrix_stack: matrix stack overflow\n");

	memcpy(&m_matrix_stack[m_matrix_stack_ptr], &m_matrix_stack[m_matrix_stack_ptr - 1], sizeof(MATRIX));
}

void model3_state::pop_matrix_stack()
{
	m_matrix_stack_ptr--;
	if (m_matrix_stack_ptr < 0)
		fatalerror("pop_matrix_stack: matrix stack underflow\n");
}

void model3_state::multiply_matrix_stack(MATRIX matrix)
{
	matrix_multiply(matrix, m_matrix_stack[m_matrix_stack_ptr], &m_matrix_stack[m_matrix_stack_ptr]);
}

void model3_state::translate_matrix_stack(float x, float y, float z)
{
	MATRIX tm;

	tm[0][0] = 1.0f;    tm[0][1] = 0.0f;    tm[0][2] = 0.0f;    tm[0][3] = 0.0f;
	tm[1][0] = 0.0f;    tm[1][1] = 1.0f;    tm[1][2] = 0.0f;    tm[1][3] = 0.0f;
	tm[2][0] = 0.0f;    tm[2][1] = 0.0f;    tm[2][2] = 1.0f;    tm[2][3] = 0.0f;
	tm[3][0] = x;       tm[3][1] = y;       tm[3][2] = z;       tm[3][3] = 1.0f;

	matrix_multiply(tm, m_matrix_stack[m_matrix_stack_ptr], &m_matrix_stack[m_matrix_stack_ptr]);
}

void model3_state::set_projection(float left, float right, float top, float bottom, float near, float far)
{
	float l = near * tanf(left * 0.5f);
	float r = near * tanf(right * 0.5f);
	float t = near * tanf(top * 0.5f );
	float b = near * tanf(bottom * 0.5f);

	m_projection_matrix[0][0] = (2.0f * near) / (l - r);
	m_projection_matrix[0][1] = 0.0f;
	m_projection_matrix[0][2] = (r + l) / (r - l);
	m_projection_matrix[0][3] = 0.0f;
	m_projection_matrix[1][0] = 0.0f;
	m_projection_matrix[1][1] = (2.0f * near) / (t - b);
	m_projection_matrix[1][2] = (t + b) / (t - b);
	m_projection_matrix[1][3] = 0.0f;
	m_projection_matrix[2][0] = 0.0f;
	m_projection_matrix[2][1] = 0.0f;
	m_projection_matrix[2][2] = -(far + near) / (far - near);
	m_projection_matrix[2][3] = -(2.0f * far * near) / (far - near);
	m_projection_matrix[3][0] = 0.0f;
	m_projection_matrix[3][1] = 0.0f;
	m_projection_matrix[3][2] = -1.0f;
	m_projection_matrix[3][3] = 0.0f;
}

/*****************************************************************************/
/* transformation and rasterizing */

void model3_state::reset_triangle_buffers()
{
	m_tri_buffer_ptr = 0;
	m_tri_alpha_buffer_ptr = 0;
}

m3_triangle *model3_state::push_triangle(bool alpha)
{
	if (!alpha)
	{
		int i = m_tri_buffer_ptr;

		if (m_tri_buffer_ptr >= TRI_BUFFER_SIZE)
		{
			return nullptr;
			//fatalerror("push_triangle: tri buffer max exceeded");
		}

		m_tri_buffer_ptr++;
		return &m_tri_buffer[i];
	}
	else
	{
		int i = m_tri_alpha_buffer_ptr;

		if (m_tri_alpha_buffer_ptr >= TRI_ALPHA_BUFFER_SIZE)
		{
			return nullptr;
			//fatalerror("push_triangle: tri alpha buffer max exceeded");
		}

		m_tri_alpha_buffer_ptr++;
		return &m_tri_alpha_buffer[i];
	}
}

void model3_state::draw_model(UINT32 addr)
{
	// Polygon RAM is mapped to the low 4MB of VROM
	UINT32 *model = (addr >= 0x100000) ? &m_vrom[addr] :  &m_polygon_ram[addr];

	UINT32 header[7];
	int index = 0;
	int last_polygon = FALSE, first_polygon = TRUE, back_face = FALSE;
	int num_vertices;
	int i, v, vi;
	float fixed_point_fraction;
	m3_vertex vertex[4];
	m3_vertex prev_vertex[4];
	m3_clip_vertex clip_vert[10];

	MATRIX transform_matrix;
	MATRIX vp_matrix;
	MATRIX coord_matrix;

	memset(&coord_matrix, 0, sizeof(coord_matrix));
	coord_matrix[0][0] = m_coordinate_system[0][1];
	coord_matrix[1][1] = m_coordinate_system[1][2];
	coord_matrix[2][2] = -m_coordinate_system[2][0];
	coord_matrix[3][3] = 1.0f;

	if (m_step < 0x15)      // position coordinates are 17.7 fixed-point in Step 1.0
		fixed_point_fraction = 1.0f / 128.0f;
	else                    // 13.11 fixed-point in other Steps
		fixed_point_fraction = 1.0f / 2048.0f;

	get_top_matrix(&transform_matrix);

	// make view-projection matrix
	matrix_multiply(transform_matrix, coord_matrix, &transform_matrix);
	matrix_multiply(transform_matrix, m_projection_matrix, &vp_matrix);

	memset(prev_vertex, 0, sizeof(prev_vertex));

	while (!last_polygon)
	{
		float texture_coord_scale;
		UINT32 color;
		VECTOR3 normal;
		VECTOR3 sn;
		VECTOR p[4];
		int polygon_transparency;

		for (i = 0; i < 7; i++)
			header[i] = model[index++];

		if (first_polygon && (header[0] & 0x0f) != 0)
			return;
		first_polygon = FALSE;

		if (header[6] == 0)
			return;

		if (header[1] & 0x4)
			last_polygon = TRUE;

		if ((header[0] & 0x300) == 0x300)       // TODO: broken polygons in srally2 have these bits set
			return;

		num_vertices = (header[0] & 0x40) ? 4 : 3;

		/* texture coordinates are 16.0 or 13.3 fixed-point */
		texture_coord_scale = (header[1] & 0x40) ? 1.0f : (1.0f / 8.0f);

		/* polygon normal (sign + 1.22 fixed-point) */
		normal[0] = (float)((INT32)header[1] >> 8) * (1.0f / 4194304.0f);
		normal[1] = (float)((INT32)header[2] >> 8) * (1.0f / 4194304.0f);
		normal[2] = (float)((INT32)header[3] >> 8) * (1.0f / 4194304.0f);

		/* load reused vertices */
		vi = 0;
		for (v = 0; v < 4; v++)
			if (header[0] & (1 << v))
				vertex[vi++] = prev_vertex[v];

		/* load new vertices */
		for ( ; vi < num_vertices; vi++)
		{
			UINT32 xw = model[index++];
			UINT32 yw = model[index++];
			UINT32 zw = model[index++];

			vertex[vi].x = (float)((INT32)(xw) >> 8) * fixed_point_fraction;
			vertex[vi].y = (float)((INT32)(yw) >> 8) * fixed_point_fraction;
			vertex[vi].z = (float)((INT32)(zw) >> 8) * fixed_point_fraction;
			vertex[vi].u = (UINT16)(model[index] >> 16);
			vertex[vi].v = (UINT16)(model[index++]);
//          vertex[vi].nx = normal[0] + ((float)((INT8)(xw)) / 127.0f);
//          vertex[vi].ny = normal[1] + ((float)((INT8)(yw)) / 127.0f);
//          vertex[vi].nz = normal[2] + ((float)((INT8)(zw)) / 127.0f);

			vertex[vi].nx = ((float)((INT8)(xw)) / 127.0f);
			vertex[vi].ny = ((float)((INT8)(yw)) / 127.0f);
			vertex[vi].nz = ((float)((INT8)(zw)) / 127.0f);
		}

		/* Copy current vertices as previous vertices */
		memcpy(prev_vertex, vertex, sizeof(m3_vertex) * 4);

		if (header[1] & 0x2)
		{
			color = (header[4] >> 8) & 0xffffff;
		}
		else
		{
			int ci = (header[4] >> 8) & 0x7ff;
			color = m_polygon_ram[0x400 + ci];
		}

		polygon_transparency =  (header[6] & 0x800000) ? 32 : ((header[6] >> 18) & 0x1f);

		/* transform polygon normal to view-space */
		sn[0] = (normal[0] * transform_matrix[0][0]) +
				(normal[1] * transform_matrix[0][1]) +
				(normal[2] * transform_matrix[0][2]);
		sn[1] = (normal[0] * transform_matrix[1][0]) +
				(normal[1] * transform_matrix[1][1]) +
				(normal[2] * transform_matrix[1][2]);
		sn[2] = (normal[0] * transform_matrix[2][0]) +
				(normal[1] * transform_matrix[2][1]) +
				(normal[2] * transform_matrix[2][2]);

		// TODO: depth bias
		// transform and light vertices
		for (i = 0; i < num_vertices; i++)
		{
			VECTOR vect;

			vect[0] = vertex[i].x;
			vect[1] = vertex[i].y;
			vect[2] = vertex[i].z;
			vect[3] = 1.0f;

			// transform to projection space
			matrix_multiply_vector(vp_matrix, vect, &p[i]);

			clip_vert[i].x = p[i][0];
			clip_vert[i].y = p[i][1];
			clip_vert[i].z = p[i][2];
			clip_vert[i].w = p[i][3];

			clip_vert[i].p[0] = vertex[i].u * texture_coord_scale * 256.0f;        // 8 bits of subtexel accuracy for bilinear filtering
			clip_vert[i].p[1] = vertex[i].v * texture_coord_scale * 256.0f;

			// transform vertex normal
			VECTOR3 n;
			n[0] = (vertex[i].nx * transform_matrix[0][0]) +
					(vertex[i].ny * transform_matrix[0][1]) +
					(vertex[i].nz * transform_matrix[0][2]);
			n[1] = (vertex[i].nx * transform_matrix[1][0]) +
					(vertex[i].ny * transform_matrix[1][1]) +
					(vertex[i].nz * transform_matrix[1][2]);
			n[2] = (vertex[i].nx * transform_matrix[2][0]) +
					(vertex[i].ny * transform_matrix[2][1]) +
					(vertex[i].nz * transform_matrix[2][2]);

			// lighting
			float intensity;
			if ((header[6] & 0x10000) == 0)
			{
				float dot = dot_product3(n, m_parallel_light);

				if (header[1] & 0x10)
					dot = fabs(dot);

				intensity = ((dot * m_parallel_light_intensity) + m_ambient_light_intensity) * 255.0f;
				if (intensity > 255.0f)
				{
					intensity = 255.0f;
				}
				if (intensity < 0.0f)
				{
					intensity = 0.0f;
				}
			}
			else
			{
				intensity = 255.0f;
			}

			clip_vert[i].p[2] = intensity;
		}

		/* clip against all edges of the view frustum */
		num_vertices = frustum_clip_all<float, 4>(clip_vert, num_vertices, clip_vert);

		/* divide by W, transform to screen coords */
		for(i=0; i < num_vertices; i++)
		{
			float oow = 1.0f / clip_vert[i].w;

			clip_vert[i].x *= oow;
			clip_vert[i].y *= oow;
			clip_vert[i].z *= oow;
			clip_vert[i].p[0] *= oow;
			clip_vert[i].p[1] *= oow;

			clip_vert[i].x = (((clip_vert[i].x * 0.5f) + 0.5f) * m_viewport_width) + m_viewport_x;
			clip_vert[i].y = (((clip_vert[i].y * 0.5f) + 0.5f) * m_viewport_height) + m_viewport_y;
			clip_vert[i].z = (((clip_vert[i].z * 0.5f) + 0.5f) * (m_viewport_far - m_viewport_near)) + m_viewport_near;
		}

		/* backface culling */
		if( (header[6] & 0x800000) && (!(header[1] & 0x0010)) )
		{
			if (sn[0]*clip_vert[0].x + sn[1]*clip_vert[0].y + sn[2]*clip_vert[0].z > 0)
				back_face = 1;
			else
				back_face = 0;
		}
		else
		{
			back_face = 0;  //no culling for transparent or two-sided polygons
		}

		back_face = 0;

		if (!back_face)
		{
			bool colormod = false;
			cached_texture* texture;

			if (header[6] & 0x0000400)
			{
				int tex_x = ((header[4] & 0x1f) << 1) | ((header[5] >> 7) & 0x1);
				int tex_y = (header[5] & 0x1f);
				int tex_width = ((header[3] >> 3) & 0x7);
				int tex_height = (header[3] & 0x7);
				int tex_format = (header[6] >> 7) & 0x7;

				if (tex_format != 0 && tex_format != 7)     // enable color modulation if this is not a color texture
					colormod = true;

				if (tex_width >= 6 || tex_height >= 6)      // srally2 poly ram has degenerate polys with 2k tex size (cpu bug or intended?)
					return;

				texture = get_texture((header[4] & 0x40) ? 1 : 0, tex_x, tex_y, tex_width, tex_height, tex_format);
			}
			else
			{
				texture = nullptr;
			}

			for (i=2; i < num_vertices; i++)
			{
				bool alpha = (header[6] & 0x1) || ((header[6] & 0x800000) == 0);        // put to alpha buffer if there's any transparency involved
				m3_triangle* tri = push_triangle(alpha);

				// bail out if tri buffer is maxed out (happens during harley boot)
				if (!tri)
					return;

				memcpy(&tri->v[0], &clip_vert[0], sizeof(m3_clip_vertex));
				memcpy(&tri->v[1], &clip_vert[i-1], sizeof(m3_clip_vertex));
				memcpy(&tri->v[2], &clip_vert[i], sizeof(m3_clip_vertex));

				tri->texture = texture;
				tri->transparency = polygon_transparency;
				tri->color = color;

				tri->param = 0;
				tri->param |= (header[4] & 0x40) ? TRI_PARAM_TEXTURE_PAGE : 0;
				tri->param |= (header[6] & 0x00000400) ? TRI_PARAM_TEXTURE_ENABLE : 0;
				tri->param |= (header[2] & 0x2) ? TRI_PARAM_TEXTURE_MIRROR_U : 0;
				tri->param |= (header[2] & 0x1) ? TRI_PARAM_TEXTURE_MIRROR_V : 0;
				tri->param |= (header[6] & 0x80000000) ? TRI_PARAM_ALPHA_TEST : 0;
				tri->param |= (colormod) ? TRI_PARAM_COLOR_MOD : 0;
			}
		}
	}
}


/*****************************************************************************/
/* display list parser */

UINT32 *model3_state::get_memory_pointer(UINT32 address)
{
	if (address & 0x800000)
	{
		if (address >= 0x840000) {
			fatalerror("get_memory_pointer: invalid display list memory address %08X\n", address);
		}
		return &m_display_list_ram[address & 0x7fffff];
	}
	else
	{
		if (address >= 0x100000) {
			fatalerror("get_memory_pointer: invalid node ram address %08X\n", address);
		}
		return &m_culling_ram[address];
	}
}

void model3_state::load_matrix(int matrix_num, MATRIX *out)
{
	float *matrix = (float *)get_memory_pointer(m_matrix_base_address + matrix_num * 12);

	(*out)[0][0] = matrix[3];   (*out)[0][1] = matrix[6];   (*out)[0][2] = matrix[9];   (*out)[0][3] = 0.0f;
	(*out)[1][0] = matrix[4];   (*out)[1][1] = matrix[7];   (*out)[1][2] = matrix[10];  (*out)[1][3] = 0.0f;
	(*out)[2][0] = matrix[5];   (*out)[2][1] = matrix[8];   (*out)[2][2] = matrix[11];  (*out)[2][3] = 0.0f;
	(*out)[3][0] = matrix[0];   (*out)[3][1] = matrix[1];   (*out)[3][2] = matrix[2];   (*out)[3][3] = 1.0f;
}

void model3_state::traverse_list4(int lod_num, UINT32 address)
{
	/* does something with the LOD selection */
	UINT32 *list = get_memory_pointer(address);
	UINT32 link = list[0];

	draw_model(link & 0xffffff);
}

void model3_state::traverse_list(UINT32 address)
{
	UINT32 *list = get_memory_pointer(address);
	int list_ptr = 0;

	if (m_list_depth > 2)
		return;

	m_list_depth++;

	/* find the end of the list */
	while (1)
	{
		address = list[list_ptr++];
		if (address & 0x02000000)
			break;
		if (address == 0 || (address >> 24) != 0)
		{
			list_ptr--;
			break;
		}
	}

	/* walk it backwards */
	while (list_ptr > 0)
	{
		address = list[--list_ptr] & 0xffffff;
		if (address != 0 && address != 0x800800)
		//if (address != 0)
			draw_block(address);
	}

	m_list_depth--;
}

inline void model3_state::process_link(UINT32 address, UINT32 link)
{
	if (link != 0 && link != 0x0fffffff && link != 0x00800800 && link != 0x01000000)
	{
		switch (link >> 24)
		{
			case 0x00:      /* link to another node */
				draw_block(link & 0xffffff);
				break;

			case 0x01:
			case 0x03:      /* both of these link to models, is there any difference ? */
				draw_model(link & 0xffffff);
				break;

			case 0x04:      /* list of links */
				traverse_list(link & 0xffffff);
				break;

			default:
				logerror("process_link %08X: link = %08X\n", address, link);
				break;
		}
	}
}

void model3_state::draw_block(UINT32 address)
{
	const UINT32 *node = get_memory_pointer(address);
	UINT32 link;
	int node_matrix;
	float x, y, z;
	MATRIX matrix;
	int offset;

	offset = (m_step < 0x15) ? 2 : 0;
	link = node[7 - offset];

	/* apply matrix and translation */
	node_matrix = node[3 - offset] & 0xfff;
	load_matrix(node_matrix, &matrix);

	push_matrix_stack();

	if (node[0] & 0x10)
	{
		x = *(float *)&node[4 - offset];
		y = *(float *)&node[5 - offset];
		z = *(float *)&node[6 - offset];
		translate_matrix_stack(x, y, z);
	}
	else if (node_matrix != 0)
		multiply_matrix_stack(matrix);

	/* bit 0x08 of word 0 indicates a pointer list */
	if (node[0] & 0x08)
		traverse_list4((node[3 - offset] >> 12) & 0x7f, link & 0xffffff);
	else
		process_link(address, link);

	pop_matrix_stack();

	/* handle the second link */
	if ((node[0] & 0x7) != 0x6)
	{
		link = node[8 - offset];
		process_link(address, link);
	}
}

void model3_state::draw_viewport(int pri, UINT32 address)
{
	const UINT32 *node = get_memory_pointer(address);
	UINT32 link_address;

	link_address = node[1];

	/* traverse to the link node before drawing this viewport */
	/* check this is correct as this affects the rendering order */
	if (link_address != 0x01000000 && link_address != 0)
		draw_viewport(pri, link_address);

	/* skip if this isn't the right priority */
	if (pri != ((node[0] >> 3) & 3))
		return;

	/* set viewport parameters */
	m_viewport_x      = (float)(node[26] & 0xffff) / 16.0f;         /* 12.4 fixed point */
	m_viewport_y      = (float)((node[26] >> 16) & 0xffff) / 16.0f;
	m_viewport_width  = (float)(node[20] & 0xffff) / 4.0f;          /* 14.2 fixed point */
	m_viewport_height = (float)((node[20] >> 16) & 0xffff) / 4.0f;

	m_viewport_near = 1.0f;
	m_viewport_far = 100000.0f;

	/* set up frustum */
	float frustum_left      = atan2(*(float *)&node[12], *(float *)&node[13]);
	float frustum_right     = -atan2(*(float *)&node[16], -*(float *)&node[17]);
	float frustum_top       = atan2(*(float *)&node[14], *(float *)&node[15]);
	float frustum_bottom    = -atan2(*(float *)&node[18], -*(float *)&node[19]);
//  float frustum_1         = atan2(*(float *)&node[9], *(float *)&node[8]);
//  float frustum_2         = atan2(*(float *)&node[11], *(float *)&node[10]);

	/*
	printf("%f\n", *(float *)&node[3]);
	printf("0: %f, 1: %f, 2: %f, 3: %f\n", *(float *)&node[8], *(float *)&node[9], *(float *)&node[10], *(float *)&node[11]);
	printf("4: %f, 5: %f, 6: %f, 7: %f\n", *(float *)&node[12], *(float *)&node[13], *(float *)&node[14], *(float *)&node[15]);
	printf("8: %f, 9: %f, A: %f, B: %f\n", *(float *)&node[16], *(float *)&node[17], *(float *)&node[18], *(float *)&node[19]);
	printf("fl = %f, fr = %f, ft = %f, fb = %f\n", RADIAN_TO_DEGREE(frustum_left), RADIAN_TO_DEGREE(frustum_right), RADIAN_TO_DEGREE(frustum_top), RADIAN_TO_DEGREE(frustum_bottom));
	printf("f1 = %f, f2 = %f\n", RADIAN_TO_DEGREE(frustum_1), RADIAN_TO_DEGREE(frustum_2));
	*/

	set_projection(frustum_left, frustum_right, frustum_top, frustum_bottom, m_viewport_near, m_viewport_far);


	m_matrix_base_address = node[22];
	/* TODO: where does node[23] point to ? LOD table ? */

	/* set lighting parameters */
	m_parallel_light[0] = *(float *)&node[5];
	m_parallel_light[1] = *(float *)&node[6];
	m_parallel_light[2] = *(float *)&node[4];
	m_parallel_light_intensity = *(float *)&node[7];
	m_ambient_light_intensity = (UINT8)(node[36] >> 8) / 256.0f;

	/* set coordinate system matrix */
	load_matrix(0, &m_coordinate_system);

	/* process a link */
	process_link(link_address, node[2]);
}


void model3_state::real3d_traverse_display_list()
{
	init_matrix_stack();

	m_list_depth = 0;

	for (int pri = 0; pri < 4; pri++)
	{
		m_viewport_tri_index[pri] = m_tri_buffer_ptr;
		m_viewport_tri_alpha_index[pri] = m_tri_alpha_buffer_ptr;
		draw_viewport(pri, 0x800000);
	}
}

void model3_renderer::draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, j;

	for (j = cliprect.min_y; j <= cliprect.max_y; ++j)
	{
		UINT32 *dst = &bitmap.pix32(j);
		UINT32 *src = &m_fb->pix32(j);

		for (i = cliprect.min_x; i <= cliprect.max_x; ++i)
		{
			if (src[i] & 0xff000000)
			{
				dst[i] = src[i];
			}
		}
	}
}

void model3_renderer::clear_fb()
{
	rectangle cliprect;
	cliprect.min_x = 0;
	cliprect.min_y = 0;
	cliprect.max_x = 495;
	cliprect.max_y = 383;

	m_fb->fill(0x00000000, cliprect);
}

void model3_renderer::clear_zb()
{
	rectangle cliprect;
	cliprect.min_x = 0;
	cliprect.min_y = 0;
	cliprect.max_x = 495;
	cliprect.max_y = 383;

	float zvalue = 10000000000.0f;
	m_zb->fill(*(int*)&zvalue, cliprect);
}

void model3_renderer::wait_for_polys()
{
	wait();
}

void model3_renderer::draw_opaque_triangles(const m3_triangle* tris, int num_tris)
{
	rectangle cliprect;
	cliprect.min_x = 0;
	cliprect.min_y = 0;
	cliprect.max_x = 495;
	cliprect.max_y = 383;

//  printf("draw opaque: %d\n", num_tris);

	vertex_t v[3];

	for (int t=0; t < num_tris; t++)
	{
		const m3_triangle* tri = &tris[t];

		if (tri->param & TRI_PARAM_TEXTURE_ENABLE)
		{
			for (int i=0; i < 3; i++)
			{
				v[i].x = tri->v[i].x;
				v[i].y = tri->v[i].y;
				v[i].p[0] = tri->v[i].w;
				v[i].p[1] = 1.0f / tri->v[i].w;
				v[i].p[2] = tri->v[i].p[0];
				v[i].p[3] = tri->v[i].p[1];
				v[i].p[4] = tri->v[i].p[2];
			}

			model3_polydata &extra = object_data_alloc();
			extra.texture = tri->texture;
			extra.transparency = tri->transparency;
			extra.texture_param = tri->param;
			extra.color = tri->color;

			if (tri->param & TRI_PARAM_ALPHA_TEST)
			{
				render_triangle(cliprect, render_delegate(FUNC(model3_renderer::draw_scanline_tex_contour), this), 5, v[0], v[1], v[2]);
			}
			else
			{
				if (tri->param & TRI_PARAM_COLOR_MOD)
					render_triangle(cliprect, render_delegate(FUNC(model3_renderer::draw_scanline_tex_colormod), this), 5, v[0], v[1], v[2]);
				else
					render_triangle(cliprect, render_delegate(FUNC(model3_renderer::draw_scanline_tex), this), 5, v[0], v[1], v[2]);
			}
		}
		else
		{
			for (int i=0; i < 3; i++)
			{
				v[i].x = tri->v[i].x;
				v[i].y = tri->v[i].y;
				v[i].p[0] = tri->v[i].w;
				v[i].p[1] = tri->v[i].p[2];
			}

			model3_polydata &extra = object_data_alloc();
			extra.color = tri->color;

			render_triangle(cliprect, render_delegate(FUNC(model3_renderer::draw_scanline_solid), this), 2, v[0], v[1], v[2]);
		}
	}
}

void model3_renderer::draw_alpha_triangles(const m3_triangle* tris, int num_tris)
{
	rectangle cliprect;
	cliprect.min_x = 0;
	cliprect.min_y = 0;
	cliprect.max_x = 495;
	cliprect.max_y = 383;

//  printf("draw alpha: %d\n", num_tris);

	vertex_t v[3];

	for (int t=num_tris-1; t >= 0; t--)
	{
		const m3_triangle* tri = &tris[t];

		if (tri->param & TRI_PARAM_TEXTURE_ENABLE)
		{
			for (int i=0; i < 3; i++)
			{
				v[i].x = tri->v[i].x;
				v[i].y = tri->v[i].y;
				v[i].p[0] = tri->v[i].w;
				v[i].p[1] = 1.0f / tri->v[i].w;
				v[i].p[2] = tri->v[i].p[0];
				v[i].p[3] = tri->v[i].p[1];
				v[i].p[4] = tri->v[i].p[2];
			}

			model3_polydata &extra = object_data_alloc();
			extra.texture = tri->texture;
			extra.transparency = tri->transparency;
			extra.texture_param = tri->param;

			render_triangle(cliprect, render_delegate(FUNC(model3_renderer::draw_scanline_tex_alpha), this), 5, v[0], v[1], v[2]);
		}
		else
		{
			for (int i=0; i < 3; i++)
			{
				v[i].x = tri->v[i].x;
				v[i].y = tri->v[i].y;
				v[i].p[0] = tri->v[i].w;
				v[i].p[1] = tri->v[i].p[2];
			}

			model3_polydata &extra = object_data_alloc();
			extra.color = tri->color;
			extra.transparency = tri->transparency;

			render_triangle(cliprect, render_delegate(FUNC(model3_renderer::draw_scanline_solid_trans), this), 2, v[0], v[1], v[2]);
		}
	}
}

void model3_renderer::draw_scanline_solid(INT32 scanline, const extent_t &extent, const model3_polydata &polydata, int threadid)
{
	UINT32 *fb = &m_fb->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);

	float z = extent.param[0].start;
	float dz = extent.param[0].dpdx;

	float in = extent.param[1].start;
	float inz = extent.param[1].dpdx;

	rgbaint_t color(polydata.color);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x])
		{
			rgbaint_t c(color);

			c.scale_imm_and_clamp((int)in);

			fb[x] = 0xff000000 | c.to_rgba_clamp();
			zb[x] = z;
		}

		in += inz;
		z += dz;
	}
}

void model3_renderer::draw_scanline_solid_trans(INT32 scanline, const extent_t &extent, const model3_polydata &polydata, int threadid)
{
	UINT32 *fb = &m_fb->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);

	float z = extent.param[0].start;
	float dz = extent.param[0].dpdx;

	float in = extent.param[1].start;
	float inz = extent.param[1].dpdx;

	rgbaint_t color(polydata.color);

	int trans = (polydata.transparency << 3) | (polydata.transparency >> 2);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x])
		{
			rgbaint_t c(color);

			c.scale_imm_and_clamp((int)in);

			if (trans != 0xff)
			{
				c.blend(rgbaint_t(fb[x]), trans);
			}

			fb[x] = 0xff000000 | c.to_rgba_clamp();
		}

		in += inz;
		z += dz;
	}
}

#define TEX_FETCH_NOFILTER()                                \
do {                                                        \
	float intz = 1.0f / ooz;                                \
	UINT32 u = uoz * intz;                                  \
	UINT32 v = voz * intz;                                  \
	UINT32 u1 = (u >> 8) & umask;                           \
	UINT32 v1 = (v >> 8) & vmask;                           \
	texel = texture->data[(v1 << width) + u1];              \
} while(0);

#define TEX_FETCH_BILINEAR()                                                    \
do {                                                                            \
	float intz = 1.0f / ooz;                                                    \
	UINT32 u = uoz * intz;                                                      \
	UINT32 v = voz * intz;                                                      \
	UINT32 u1 = (u >> 8) & umask;                                               \
	UINT32 v1 = (v >> 8) & vmask;                                               \
	UINT32 u2 = (u1 + 1) & umask;                                               \
	UINT32 v2 = (v1 + 1) & vmask;                                               \
	UINT32 pix00 = texture->data[(v1 << width) + u1];                           \
	UINT32 pix01 = texture->data[(v1 << width) + u2];                           \
	UINT32 pix10 = texture->data[(v2 << width) + u1];                           \
	UINT32 pix11 = texture->data[(v2 << width) + u2];                           \
	texel = rgbaint_t::bilinear_filter(pix00, pix01, pix10, pix11, u, v);       \
} while(0);

#if ENABLE_BILINEAR
#define TEX_FETCH() TEX_FETCH_BILINEAR()
#else
#define TEX_FETCH() TEX_FETCH_NOFILTER()
#endif

void model3_renderer::draw_scanline_tex(INT32 scanline, const extent_t &extent, const model3_polydata &polydata, int threadid)
{
	UINT32 *fb = &m_fb->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);
	const cached_texture *texture = polydata.texture;

	float z = extent.param[0].start;
	float dz = extent.param[0].dpdx;
	float ooz = extent.param[1].start;
	float dooz = extent.param[1].dpdx;
	float uoz = extent.param[2].start;
	float duoz = extent.param[2].dpdx;
	float voz = extent.param[3].start;
	float dvoz = extent.param[3].dpdx;
	float in = extent.param[4].start;
	float inz = extent.param[4].dpdx;

	UINT32 umask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_U) ? 64 : 32) << texture->width) - 1;
	UINT32 vmask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_V) ? 64 : 32) << texture->height) - 1;
	UINT32 width = 6 + texture->width;

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x])
		{
			UINT32 texel;
			TEX_FETCH();        // TODO fetch rgbaint_t instead

			rgbaint_t color(texel);

			color.scale_imm_and_clamp((int)in);

			fb[x] = 0xff000000 | color.to_rgba_clamp();
			zb[x] = z;
		}

		ooz += dooz;
		uoz += duoz;
		voz += dvoz;
		in += inz;
		z += dz;
	}
}

void model3_renderer::draw_scanline_tex_colormod(INT32 scanline, const extent_t &extent, const model3_polydata &polydata, int threadid)
{
	UINT32 *fb = &m_fb->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);
	const cached_texture *texture = polydata.texture;

	float z = extent.param[0].start;
	float dz = extent.param[0].dpdx;
	float ooz = extent.param[1].start;
	float dooz = extent.param[1].dpdx;
	float uoz = extent.param[2].start;
	float duoz = extent.param[2].dpdx;
	float voz = extent.param[3].start;
	float dvoz = extent.param[3].dpdx;
	float in = extent.param[4].start;
	float inz = extent.param[4].dpdx;

	UINT32 umask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_U) ? 64 : 32) << texture->width) - 1;
	UINT32 vmask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_V) ? 64 : 32) << texture->height) - 1;
	UINT32 width = 6 + texture->width;

	rgbaint_t polycolor(polydata.color);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x])
		{
			UINT32 texel;
			TEX_FETCH();        // TODO fetch rgbaint_t instead

			rgbaint_t color(texel);

			color.scale_and_clamp(polycolor);
			color.scale_imm_and_clamp((int)in);

			fb[x] = 0xff000000 | color.to_rgba_clamp();
			zb[x] = z;
		}

		ooz += dooz;
		uoz += duoz;
		voz += dvoz;
		in += inz;
		z += dz;
	}
}

void model3_renderer::draw_scanline_tex_contour(INT32 scanline, const extent_t &extent, const model3_polydata &polydata, int threadid)
{
	UINT32 *fb = &m_fb->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);
	const cached_texture *texture = polydata.texture;

	float z = extent.param[0].start;
	float dz = extent.param[0].dpdx;
	float ooz = extent.param[1].start;
	float dooz = extent.param[1].dpdx;
	float uoz = extent.param[2].start;
	float duoz = extent.param[2].dpdx;
	float voz = extent.param[3].start;
	float dvoz = extent.param[3].dpdx;
	float in = extent.param[4].start;
	float inz = extent.param[4].dpdx;

	UINT32 umask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_U) ? 64 : 32) << texture->width) - 1;
	UINT32 vmask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_V) ? 64 : 32) << texture->height) - 1;
	UINT32 width = 6 + texture->width;

	rgbaint_t polycolor(polydata.color);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x])
		{
			UINT32 texel;
			TEX_FETCH();

			UINT32 fa = texel >> 24;
			if (fa >= 0xf8)
			{
				rgbaint_t color(texel);

				color.scale_and_clamp(polycolor);
				color.scale_imm_and_clamp((int)in);
				color.blend(rgbaint_t(fb[x]), fa);

				fb[x] = 0xff000000 | color.to_rgba_clamp();
				zb[x] = z;
			}
		}

		ooz += dooz;
		uoz += duoz;
		voz += dvoz;
		in += inz;
		z += dz;
	}
}

void model3_renderer::draw_scanline_tex_trans(INT32 scanline, const extent_t &extent, const model3_polydata &polydata, int threadid)
{
	UINT32 *fb = &m_fb->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);
	const cached_texture *texture = polydata.texture;

	float z = extent.param[0].start;
	float dz = extent.param[0].dpdx;
	float ooz = extent.param[1].start;
	float dooz = extent.param[1].dpdx;
	float uoz = extent.param[2].start;
	float duoz = extent.param[2].dpdx;
	float voz = extent.param[3].start;
	float dvoz = extent.param[3].dpdx;
	float in = extent.param[4].start;
	float inz = extent.param[4].dpdx;

	int trans = (polydata.transparency << 3) | (polydata.transparency >> 2);

	UINT32 umask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_U) ? 64 : 32) << texture->width) - 1;
	UINT32 vmask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_V) ? 64 : 32) << texture->height) - 1;
	UINT32 width = 6 + texture->width;

	rgbaint_t polycolor(polydata.color);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x])
		{
			UINT32 texel;
			TEX_FETCH();

			rgbaint_t color(texel);

			color.scale_and_clamp(polycolor);
			color.scale_imm_and_clamp((int)in);
			color.blend(rgbaint_t(fb[x]), trans);

			fb[x] = 0xff000000 | color.to_rgba_clamp();
		}

		ooz += dooz;
		uoz += duoz;
		voz += dvoz;
		in += inz;
		z += dz;
	}
}

void model3_renderer::draw_scanline_tex_alpha(INT32 scanline, const extent_t &extent, const model3_polydata &polydata, int threadid)
{
	UINT32 *fb = &m_fb->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);
	const cached_texture *texture = polydata.texture;

	float z = extent.param[0].start;
	float dz = extent.param[0].dpdx;
	float ooz = extent.param[1].start;
	float dooz = extent.param[1].dpdx;
	float uoz = extent.param[2].start;
	float duoz = extent.param[2].dpdx;
	float voz = extent.param[3].start;
	float dvoz = extent.param[3].dpdx;
	float in = extent.param[4].start;
	float inz = extent.param[4].dpdx;

//  int srctrans = polydata.transparency;
//  int desttrans = 32 - polydata.transparency;

	UINT32 umask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_U) ? 64 : 32) << texture->width) - 1;
	UINT32 vmask = (((polydata.texture_param & TRI_PARAM_TEXTURE_MIRROR_V) ? 64 : 32) << texture->height) - 1;
	UINT32 width = 6 + texture->width;

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x])
		{
			UINT32 texel;
			TEX_FETCH();

			UINT32 fa = texel >> 24;
			if (fa != 0)
			{
				rgbaint_t color(texel);

				color.scale_imm_and_clamp((int)in);
				color.blend(rgbaint_t(fb[x]), fa);

				fb[x] = 0xff000000 | color.to_rgba_clamp();
			}
		}

		ooz += dooz;
		uoz += duoz;
		voz += dvoz;
		in += inz;
		z += dz;
	}
}
