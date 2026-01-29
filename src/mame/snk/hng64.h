// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
#ifndef MAME_SNK_HNG64_H
#define MAME_SNK_HNG64_H

#pragma once

#include "cpu/mips/mips3.h"
#include "cpu/nec/v5x.h"
#include "cpu/tlcs870/tlcs870.h"
#include "machine/mb8421.h"
#include "machine/msm6242.h"
#include "machine/timer.h"
#include "sound/l7a1045_l6028_dsp_a.h"
#include "video/poly.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

/////////////////
/// 3d Engine ///
/////////////////

struct polyVert
{
	float worldCoords[4]{};   // World space coordinates (X Y Z 1.0)

	float texCoords[2]{};     // Texture coordinates (U V 0 1.0) -> OpenGL style...

	float normal[4]{};        // Normal (X Y Z 1.0)
	float clipCoords[4]{};    // Homogeneous screen space coordinates (X Y Z W)

	float light;         // The intensity of the illumination at this point

	//u16 colorIndex = 0;    // Flat shaded polygons, no texture, no lighting
};

struct polygon
{
	int n = 0;                      // Number of sides
	polyVert vert[10]{};          // Vertices (maximum number per polygon is 10 -> 3+6)

	float faceNormal[4]{};        // Normal of the face overall - for calculating visibility and flat-shading...
	bool visible = false;                // Polygon visibility in scene
	bool flatShade = false;              // Flat shaded polygon, no texture, no lighting
	bool blend = false;

	u8 texIndex = 0;             // Which texture to draw from (0x00-0x0f)
	u8 tex4bpp = 0;              // How to index into the texture
	u16 texPageSmall = 0;         // Does this polygon use 'small' texture pages?
	u32 palOffset = 0;           // The base offset where this object's palette starts.
	u16 colorIndex = 0;

	u16 texscrollx = 0;
	u16 texscrolly = 0;

	u16 tex_mask_x = 1024;
	u16 tex_mask_y = 1024;
};


/////////////////////////
/// polygon rendering ///
/////////////////////////

// Refer to the clipping planes as numbers
#define HNG64_LEFT   0
#define HNG64_RIGHT  1
#define HNG64_TOP    2
#define HNG64_BOTTOM 3
#define HNG64_NEAR   4
#define HNG64_FAR    5


////////////////////////////////////
/// Polygon rasterizer interface ///
////////////////////////////////////

const int HNG64_MAX_POLYGONS = 10000;

typedef frustum_clip_vertex<float, 5> hng64_clip_vertex;

struct hng64_poly_data
{
	u8 tex4bpp = 0;
	u8 texIndex = 0;
	u16 texPageSmall = 0;
	u32 palOffset = 0;
	u16 colorIndex = 0;
	bool blend = false;
	u16 texscrollx = 0;
	u16 texscrolly = 0;

	u16 tex_mask_x = 1024;
	u16 tex_mask_y = 1024;
};

class hng64_state;

class hng64_poly_renderer : public poly_manager<float, hng64_poly_data, 7>
{
public:
	hng64_poly_renderer(hng64_state &state);

	void drawShaded(polygon *p);
	void render_texture_scanline(s32 scanline, const extent_t &extent, const hng64_poly_data &renderData, int threadid);
	void render_flat_scanline(s32 scanline, const extent_t &extent, const hng64_poly_data &renderData, int threadid);

	hng64_state &state() { return m_state; }
	float *depthBuffer3d() { return m_depthBuffer3d.get(); }
	u16 *colorBuffer3d() { return m_colorBuffer3d.get(); }

private:
	hng64_state &m_state;

	// (Temporarily class members - someday they will live in the memory map)
	std::unique_ptr<float[]> m_depthBuffer3d;
	std::unique_ptr<u16[]> m_colorBuffer3d;
};


// TODO, this could become the IO board device emulation
class hng64_lamps_device : public device_t
{
public:
	hng64_lamps_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <unsigned N> auto lamps_out_cb() { return m_lamps_out_cb[N].bind(); }

	void lamps_w(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write8::array<8> m_lamps_out_cb;
};


class hng64_state : public driver_device
{
public:
	hng64_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_texturerom(*this, "textures0"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_palette_fade0(*this, "palette0"),
		m_palette_fade1(*this, "palette1"),
		m_palette_3d(*this, "palette3d"),
		m_paletteram(*this, "paletteram"),
		m_vblank(*this, "VBLANK"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_iomcu(*this, "iomcu"),
		m_lamps(*this, "lamps"),
		m_dt71321_dpram(*this, "dt71321_dpram"),
		m_dsp(*this, "l7a1045"),
		m_comm(*this, "network"),
		m_rtc(*this, "rtc"),
		m_mainram(*this, "mainram"),
		m_cart(*this, "gameprg"),
		m_sysregs(*this, "sysregs"),
		m_rombase(*this, "bios"),
		m_spriteram(*this, "spriteram"),
		m_spriteregs(*this, "spriteregs"),
		m_videoram(*this, "videoram"),
		m_videoregs(*this, "videoregs"),
		m_tcram(*this, "tcram"),
		m_comhack(*this, "comhack"),
		m_fbram1(*this, "fbram1"),
		m_fbram2(*this, "fbram2"),
		m_fbscale(*this, "fbscale"),
		m_fbscroll(*this, "fbscroll"),
		m_fbunk(*this, "fbunk"),
		m_idt7133_dpram(*this, "com_ram"),
		m_com_bank(*this, "com_bank"),
		m_audio_bank(*this, "audio_bank_%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_in(*this, "IN%u", 0U),
		m_samsho64_3d_hack(0),
		m_roadedge_3d_hack(0),
		m_vertsrom(*this, "verts"),
		m_wheel_motor(*this, "wheel_motor"),
		m_lamps_out(*this, "lamp%u", 0U)
	{
	}

	void hng64(machine_config &config) ATTR_COLD;
	void hng64_default(machine_config &config) ATTR_COLD;
	void hng64_drive(machine_config &config) ATTR_COLD;
	void hng64_shoot(machine_config &config) ATTR_COLD;
	void hng64_fight(machine_config &config) ATTR_COLD;

	void init_roadedge() ATTR_COLD;
	void init_hng64_drive() ATTR_COLD;
	void init_hng64() ATTR_COLD;
	void init_hng64_shoot() ATTR_COLD;
	void init_ss64() ATTR_COLD;
	void init_hng64_fght() ATTR_COLD;

	required_region_ptr<u8> m_texturerom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_palette_fade0;
	required_device<palette_device> m_palette_fade1;
	required_device<palette_device> m_palette_3d;
	required_shared_ptr<u32> m_paletteram;
	required_ioport m_vblank;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	static constexpr int HNG64_MASTER_CLOCK = 50'000'000;

	/* TODO: NOT measured! */
	static constexpr int PIXEL_CLOCK = (HNG64_MASTER_CLOCK * 2) / 4; // x 2 is due to the interlaced screen ...

	static constexpr int HTOTAL = 0x200 + 0x100;
	static constexpr int HBEND = 0;
	static constexpr int HBSTART = 0x200;

	static constexpr int VTOTAL = 264 * 2;
	static constexpr int VBEND = 0;
	static constexpr int VBSTART = 224 * 2;

	required_device<mips3_device> m_maincpu;
	required_device<v53a_device> m_audiocpu;
	required_device<tmp87ph40an_device> m_iomcu;
	required_device<hng64_lamps_device> m_lamps;
	required_device<idt71321_device> m_dt71321_dpram;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<cpu_device> m_comm;
	required_device<msm6242_device> m_rtc;

	required_shared_ptr<u32> m_mainram;
	required_region_ptr<u32> m_cart;
	required_shared_ptr<u32> m_sysregs;
	required_region_ptr<u32> m_rombase;
	required_shared_ptr<u32> m_spriteram;
	required_shared_ptr<u32> m_spriteregs;
	required_shared_ptr<u32> m_videoram;
	required_shared_ptr<u32> m_videoregs;
	required_shared_ptr<u32> m_tcram;

	std::unique_ptr<u16[]> m_dl;
	required_shared_ptr<u32> m_comhack;
	required_shared_ptr<u32> m_fbram1;
	required_shared_ptr<u32> m_fbram2;
	required_shared_ptr<u32> m_fbscale;
	required_shared_ptr<u32> m_fbscroll;
	required_shared_ptr<u32> m_fbunk;

	required_shared_ptr<u32> m_idt7133_dpram;
	//required_shared_ptr<u8> m_com_mmu_mem;
	required_memory_bank m_com_bank;

	required_memory_bank_array<16> m_audio_bank;

	required_device<gfxdecode_device> m_gfxdecode;

	required_ioport_array<8> m_in;

	bool m_samsho64_3d_hack = false;
	bool m_roadedge_3d_hack = false;

	u8 m_fbcontrol[4]{};
	u8 m_texture_wrapsize_table[0x20]{};

	std::unique_ptr<u16[]> m_soundram;
	std::unique_ptr<u16[]> m_soundram2;

	/* Communications stuff */
	std::unique_ptr<u8[]> m_com_op_base;
	std::unique_ptr<u8[]> m_com_virtual_mem;
	u8 m_com_shared[8]{};

	s32 m_dma_start = 0;
	s32 m_dma_dst = 0;
	s32 m_dma_len = 0;

	u16 m_mcu_en = 0;

	u32 m_activeDisplayList = 0U;
	u32 m_no_machine_error_code = 0U;

	u32 m_unk_vreg_toggle = 0U;
	u32 m_p1_trig = 0U;

	//u32 *q2 = nullptr;

	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_sprite_zbuffer;

	u8 m_irq_pos_half;
	u32 m_raster_irq_pos[2];

	u8 m_screen_dis = 0U;

	struct hng64_tilemap {
		tilemap_t *m_tilemap_8x8 = nullptr;
		tilemap_t *m_tilemap_16x16 = nullptr;
		tilemap_t *m_tilemap_16x16_alt = nullptr;
	};

	hng64_tilemap m_tilemap[4]{};

	u32 m_old_animmask = 0U;
	u32 m_old_animbits = 0U;
	u16 m_old_tileflags[4]{};

	// 3d State
	u16 m_texturescrollx = 0;
	u16 m_texturescrolly = 0;
	u16 m_paletteState3d = 0;
	u16 m_modelscalex = 0;
	u16 m_modelscaley = 0;
	u16 m_modelscalez = 0;

	float m_projectionMatrix[16]{};
	float m_modelViewMatrix[16]{};
	float m_cameraMatrix[16]{};

	float m_lightStrength = 0;
	float m_lightVector[3]{};

	u8 m_port7 = 0;
	u8 m_port1 = 0;

	u16 m_ex_ramaddr = 0;
	u16 m_ex_ramaddr_upper = 0;

	emu_timer *m_tempio_irqon_timer = nullptr;
	emu_timer *m_tempio_irqoff_timer = nullptr;

	u32 m_irq_pending = 0;

	emu_timer *m_comhack_timer = nullptr;

	u32 m_irq_level = 0;
	std::unique_ptr<hng64_poly_renderer> m_poly_renderer;

	emu_timer *m_3dfifo_timer = nullptr;

	required_region_ptr<u16> m_vertsrom;
	std::vector<polygon> m_polys;  // HNG64_MAX_POLYGONS

	output_finder<> m_wheel_motor;
	output_finder<8> m_lamps_out;

	u16 main_latch[2]{};
	u16 sound_latch[2]{};

	template <unsigned N> void default_lamps_w(u8 data) { logerror("lamps%u %02x\n", N, data); }

	void drive_lamps7_w(u8 data);
	void drive_lamps6_w(u8 data);
	void drive_lamps5_w(u8 data);

	void shoot_lamps7_w(u8 data);
	void shoot_lamps6_w(u8 data);

	void fight_lamps6_w(u8 data);

	u32 com_r(offs_t offset, u32 mem_mask = ~0);
	void com_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void com_share_w(offs_t offset, u8 data);
	u8 com_share_r(offs_t offset);
	void com_bank_w(u8 data);
	void com_share_mips_w(offs_t offset, u8 data);
	u8 com_share_mips_r(offs_t offset);
	u32 sysregs_r(offs_t offset, u32 mem_mask = ~0);
	void sysregs_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 rtc_r(offs_t offset, u32 mem_mask = ~0);
	void rtc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 dmac_r(offs_t offset, u32 mem_mask = ~0);
	void dmac_w(address_space &space, offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 irqc_r(offs_t offset, u32 mem_mask = ~0);
	void irqc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void mips_to_iomcu_irq_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void raster_irq_pos_w(u32 data);

	u8 dualport_r(offs_t offset);
	void dualport_w(offs_t offset, u8 data);

	u8 fbcontrol_r(offs_t offset);
	void fbcontrol_w(offs_t offset, u8 data);

	void fbscale_w(offs_t offset, u32 data, u32 mem_mask);
	void fbscroll_w(offs_t offset, u32 data, u32 mem_mask);

	void fbunkbyte_w(offs_t offset, u32 data, u32 mem_mask);

	u8 texture_wrapsize_table_r(offs_t offset);
	void texture_wrapsize_table_w(offs_t offset, u8 data);

	u32 fbram1_r(offs_t offset);
	void fbram1_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 fbram2_r(offs_t offset);
	void fbram2_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void dl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	//u32 dl_r();
	void dl_control_w(u32 data);
	void dl_upload_w(u32 data);
	void dl_unk_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 dl_vreg_r();

	void set_palette_entry_with_faderegs(int entry, u8 r, u8 g, u8 b, u32 rgbfade, u8 r_mode, u8 g_mode, u8 b_mode, palette_device *palette);
	void set_single_palette_entry(int entry, u8 r, u8 g, u8 b);
	void update_palette_entry(int entry);
	void pal_w(offs_t offset, u32 data, u32 mem_mask);
	void tcram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 tcram_r(offs_t offset);

	void soundram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 soundram_r(offs_t offset);
	void vregs_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	// not actually used, but left in code so you can turn it and see the (possibly undesired?) behavior, see notes in memory map
	void soundram2_w(u32 data);
	u32 soundram2_r();

	void soundcpu_enable_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void sprite_clear_even_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void sprite_clear_odd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void videoram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	// shared ram access
	u8 ioport0_r();
	void ioport0_w(u8 data);
	void ioport7_w(u8 data);

	// input port access
	u8 ioport3_r();
	void ioport3_w(u8 data);
	void ioport1_w(u8 data);

	// unknown access
	void ioport4_w(u8 data);

	void sio0_w(int state);

	TIMER_CALLBACK_MEMBER(tempio_irqon_callback);
	TIMER_CALLBACK_MEMBER(tempio_irqoff_callback);
	void init_io();

	void init_reorder_gfx();

	void set_irq(u32 irq_vector);
	TIMER_CALLBACK_MEMBER(comhack_callback);
	template <u8 Which, u8 Size> TILE_GET_INFO_MEMBER(get_tile_info);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(hng64_irq);
	void do_dma(address_space &space);

	void mark_all_tiles_dirty(int tilemap);
	void mark_tile_dirty(int tilemap, int tile_index);

	u16 get_tileregs(int tm);
	u16 get_scrollbase(int tm);

	int get_blend_mode(int tm);

	void draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tm, int flags, int line);

	void tilemap_draw_roz_core_line(
			screen_device &screen, bitmap_rgb32 &destbitmap, const rectangle &cliprect, tilemap_t *tmap,
			int wraparound, u8 drawformat, u8 alpha, u8 mosaic, u8 tm, int splitside);

	TIMER_CALLBACK_MEMBER(_3dfifo_processed);
	void clear3d();
	bool command3d(const u16* packet);

	void mixsprites_test(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u16 priority, int y);

	void get_tile_details(bool chain, u16 spritenum, u8 xtile, u8 ytile, u8 xsize, u8 ysize, bool xflip, bool yflip, u32 &tileno, u16 &pal, u8 &gfxregion);
	void draw_sprites_buffer(screen_device &screen, const rectangle &cliprect);
	void draw_sprite_line(
			screen_device &screen, const rectangle &cliprect,
			s32 curyy, s16 cury, s16 xpos, int chainx, s32 dx, s32 dy, int ytileblock, int chaini, int currentsprite, int chainy,
			int xflip, int yflip, u16 zval, bool zsort, bool blend, u16 group, bool checkerboard, u8 mosaic);

	void drawline(
			bitmap_ind16 &dest, bitmap_ind16 &destz, const rectangle &cliprect,
			gfx_element *gfx, u32 code, u32 color,
			int flipy, s32 xpos, s32 dx, s32 dy,
			u32 trans_pen, u32 zval, bool zrev, bool blend, bool checkerboard, u8 mosaic, u8 &mosaic_count_x,
			s32 ypos, const u8 *srcdata, s32 srcx, u32 leftovers, int line, u16 &srcpix);

	void zoom_transpen(
			bitmap_ind16 &dest, bitmap_ind16 &destz, const rectangle &cliprect,
			gfx_element *gfx, u32 code, u32 color,
			int flipx, int flipy, s32 xpos, s32 ypos, s32 dx, s32 dy, u32 dstwidth,
			u32 trans_pen, u32 zval, bool zrev, bool blend, u16 group, bool checkerboard, u8 mosaic, u8 &mosaic_count_x,
			int line, u16 &srcpix);

	void setCameraTransformation(const u16 *packet);
	void setLighting(const u16 *packet);
	void set3dFlags(const u16 *packet);
	void setCameraProjectionMatrix(const u16 *packet);
	void recoverStandardVerts(polygon &currentPoly, int m, const u16 *chunkOffset_verts, int &counter, const u16 *packet);
	void recoverPolygonBlock(const u16 *packet, int &numPolys);
	void printPacket(const u16 *packet, int hex);
	float uToF(u16 input);
	void matmul4(float *product, const float *a, const float *b);
	void vecmatmul4(float *product, const float *a, const float *b);
	float vecDotProduct(const float *a, const float *b);
	void setIdentity(float *matrix);
	void normalize(float* x);

	void init_sound();

	void reset_sound();
	void reset_net();

	void dma_hreq_cb(int state);
	u8 dma_memr_cb(offs_t offset);
	void dma_iow3_cb(u8 data);
	void tcu_tm0_cb(int state);
	void tcu_tm1_cb(int state);
	void tcu_tm2_cb(int state);

	void sound_port_0080_w(u16 data);

	void sound_bank_w(offs_t offset, u16 data);
	u16 main_sound_comms_r(offs_t offset);
	void main_sound_comms_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sound_comms_r(offs_t offset);
	void sound_comms_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void hng64_audio_base(machine_config &config) ATTR_COLD;
	void hng64_audio(machine_config &config) ATTR_COLD;
	void hng64_audio_bbust2(machine_config &config) ATTR_COLD;
	void hng64_network(machine_config &config) ATTR_COLD;
	void comm_io_map(address_map &map) ATTR_COLD;
	void comm_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void dsp_map(address_map &map) ATTR_COLD;
	void dsp_ram_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SNK_HNG64_H
