// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
#ifndef MAME_INCLUDES_HNG64_H
#define MAME_INCLUDES_HNG64_H

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

	float texCoords[4]{};     // Texture coordinates (U V 0 1.0) -> OpenGL style...

	float normal[4]{};        // Normal (X Y Z 1.0)
	float clipCoords[4]{};    // Homogeneous screen space coordinates (X Y Z W)

	float light[3]{};         // The intensity of the illumination at this point

	uint16_t colorIndex = 0;    // Flat shaded polygons, no texture, no lighting
};

struct polygon
{
	int n = 0;                      // Number of sides
	polyVert vert[10]{};          // Vertices (maximum number per polygon is 10 -> 3+6)

	float faceNormal[4]{};        // Normal of the face overall - for calculating visibility and flat-shading...
	bool visible = false;                // Polygon visibility in scene
	bool flatShade = false;              // Flat shaded polygon, no texture, no lighting

	uint8_t texIndex = 0;             // Which texture to draw from (0x00-0x0f)
	uint8_t texType = 0;              // How to index into the texture
	uint8_t texPageSmall = 0;         // Does this polygon use 'small' texture pages?
	uint8_t texPageHorizOffset = 0;   // If it does use small texture pages, how far is this page horizontally offset?
	uint8_t texPageVertOffset = 0;    // If it does use small texture pages, how far is this page vertically offset?

	uint32_t palOffset = 0;           // The base offset where this object's palette starts.
	uint32_t palPageSize = 0;         // The size of the palette page that is being pointed to.

	uint32_t debugColor = 0;          // Will go away someday.  Used to explicitly color polygons for debugging.
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
	uint8_t texType = 0;
	uint8_t texIndex = 0;
	uint8_t texPageSmall = 0;
	uint8_t texPageHorizOffset = 0;
	uint8_t texPageVertOffset = 0;
	int palOffset = 0;
	int palPageSize = 0;
	int debugColor = 0;
};

class hng64_state;

class hng64_poly_renderer : public poly_manager<float, hng64_poly_data, 7>
{
public:
	hng64_poly_renderer(hng64_state& state);

	void drawShaded(polygon *p);
	void render_texture_scanline(int32_t scanline, const extent_t& extent, const hng64_poly_data& renderData, int threadid);
	void render_flat_scanline(int32_t scanline, const extent_t& extent, const hng64_poly_data& renderData, int threadid);

	hng64_state& state() { return m_state; }
	bitmap_rgb32& colorBuffer3d() { return m_colorBuffer3d; }
	float* depthBuffer3d() { return m_depthBuffer3d.get(); }

private:
	hng64_state& m_state;

	// (Temporarily class members - someday they will live in the memory map)
	bitmap_rgb32 m_colorBuffer3d;
	std::unique_ptr<float[]> m_depthBuffer3d;
};


// TODO, this could become the IO board device emulation
class hng64_lamps_device : public device_t
{
public:
	hng64_lamps_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto lamps_out_cb() { return m_lamps_out_cb[N].bind(); }

	void lamps_w(offs_t offset, uint8_t data) { m_lamps_out_cb[offset](data); }

protected:
	virtual void device_start() override;

private:
	devcb_write8::array<8> m_lamps_out_cb;
};


class hng64_state : public driver_device
{
public:
	hng64_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
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
		m_rombase(*this, "user1"),
		m_spriteram(*this, "spriteram"),
		m_spriteregs(*this, "spriteregs"),
		m_videoram(*this, "videoram"),
		m_videoregs(*this, "videoregs"),
		m_tcram(*this, "tcram"),
		m_fbtable(*this, "fbtable"),
		m_comhack(*this, "comhack"),
		m_fbram1(*this, "fbram1"),
		m_fbram2(*this, "fbram2"),
		m_idt7133_dpram(*this, "com_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_in(*this, "IN%u", 0U),
		m_samsho64_3d_hack(0),
		m_roadedge_3d_hack(0)
	{ }

	void hng64(machine_config &config);
	void hng64_default(machine_config &config);
	void hng64_drive(machine_config &config);
	void hng64_shoot(machine_config &config);
	void hng64_fight(machine_config &config);

	void init_roadedge();
	void init_hng64_drive();
	void init_hng64();
	void init_hng64_shoot();
	void init_ss64();
	void init_hng64_fght();

	uint8_t *m_texturerom = nullptr;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

private:
	static constexpr int HNG64_MASTER_CLOCK = 50'000'000;

	/* TODO: NOT measured! */
	static constexpr int PIXEL_CLOCK = (HNG64_MASTER_CLOCK*2)/4; // x 2 is due to the interlaced screen ...

	static constexpr int HTOTAL = 0x200+0x100;
	static constexpr int HBEND = 0;
	static constexpr int HBSTART = 0x200;

	static constexpr int VTOTAL = 264*2;
	static constexpr int VBEND = 0;
	static constexpr int VBSTART = 224*2;

	enum hng64trans_t
	{
		HNG64_TILEMAP_NORMAL = 1,
		HNG64_TILEMAP_ADDITIVE,
		HNG64_TILEMAP_ALPHA
	};

	struct blit_parameters
	{
		bitmap_rgb32 *      bitmap = nullptr;
		rectangle           cliprect;
		uint32_t            tilemap_priority_code = 0;
		uint8_t             mask = 0;
		uint8_t             value = 0;
		uint8_t             alpha = 0;
		hng64trans_t        drawformat;
	};

	required_device<mips3_device> m_maincpu;
	required_device<v53a_device> m_audiocpu;
	required_device<tmp87ph40an_device> m_iomcu;
	required_device<hng64_lamps_device> m_lamps;
	required_device<idt71321_device> m_dt71321_dpram;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<cpu_device> m_comm;
	required_device<msm6242_device> m_rtc;

	required_shared_ptr<uint32_t> m_mainram;
	required_region_ptr<uint32_t> m_cart;
	required_shared_ptr<uint32_t> m_sysregs;
	required_region_ptr<uint32_t> m_rombase;
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr<uint32_t> m_spriteregs;
	required_shared_ptr<uint32_t> m_videoram;
	required_shared_ptr<uint32_t> m_videoregs;
	required_shared_ptr<uint32_t> m_tcram;

	std::unique_ptr<uint16_t[]> m_dl;
	required_shared_ptr<uint32_t> m_fbtable;
	required_shared_ptr<uint32_t> m_comhack;
	required_shared_ptr<uint32_t> m_fbram1;
	required_shared_ptr<uint32_t> m_fbram2;

	required_shared_ptr<uint32_t> m_idt7133_dpram;
	//required_shared_ptr<uint8_t> m_com_mmu_mem;

	required_device<gfxdecode_device> m_gfxdecode;

	required_ioport_array<8> m_in;

	template <unsigned N> void hng64_default_lamps_w(uint8_t data) { logerror("lamps%u %02x\n", N, data); }

	void hng64_drive_lamps7_w(uint8_t data);
	void hng64_drive_lamps6_w(uint8_t data);
	void hng64_drive_lamps5_w(uint8_t data);

	void hng64_shoot_lamps7_w(uint8_t data);
	void hng64_shoot_lamps6_w(uint8_t data);

	void hng64_fight_lamps6_w(uint8_t data);

	int m_samsho64_3d_hack;
	int m_roadedge_3d_hack;

	uint8_t m_fbcontrol[4]{};

	std::unique_ptr<uint16_t[]> m_soundram;
	std::unique_ptr<uint16_t[]> m_soundram2;

	/* Communications stuff */
	std::unique_ptr<uint8_t[]> m_com_op_base;
	std::unique_ptr<uint8_t[]> m_com_virtual_mem;
	uint8_t m_com_shared[8]{};

	int32_t m_dma_start = 0;
	int32_t m_dma_dst = 0;
	int32_t m_dma_len = 0;

	uint16_t m_mcu_en = 0;

	uint32_t m_activeDisplayList = 0U;
	uint32_t m_no_machine_error_code = 0U;

	uint32_t m_unk_vreg_toggle = 0U;
	uint32_t m_p1_trig = 0U;

	//uint32_t *q2 = nullptr;

	std::vector< std::pair <int, uint32_t *> > m_spritelist;

	uint8_t m_screen_dis = 0U;

	struct hng64_tilemap {
		tilemap_t *m_tilemap_8x8 = nullptr;
		tilemap_t *m_tilemap_16x16 = nullptr;
		tilemap_t *m_tilemap_16x16_alt = nullptr;
	};

	hng64_tilemap m_tilemap[4]{};

	uint8_t m_additive_tilemap_debug = 0U;

	uint32_t m_old_animmask = 0U;
	uint32_t m_old_animbits = 0U;
	uint16_t m_old_tileflags[4]{};

	// 3d State
	int m_paletteState3d = 0;
	float m_projectionMatrix[16]{};
	float m_modelViewMatrix[16]{};
	float m_cameraMatrix[16]{};

	float m_lightStrength = 0;
	float m_lightVector[3]{};

	uint32_t hng64_com_r(offs_t offset, uint32_t mem_mask = ~0);
	void hng64_com_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void hng64_com_share_w(offs_t offset, uint8_t data);
	uint8_t hng64_com_share_r(offs_t offset);
	void hng64_com_share_mips_w(offs_t offset, uint8_t data);
	uint8_t hng64_com_share_mips_r(offs_t offset);
	uint32_t hng64_sysregs_r(offs_t offset, uint32_t mem_mask = ~0);
	void hng64_sysregs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t hng64_rtc_r(offs_t offset, uint32_t mem_mask = ~0);
	void hng64_rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t hng64_dmac_r(offs_t offset, uint32_t mem_mask = ~0);
	void hng64_dmac_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t hng64_irqc_r(offs_t offset, uint32_t mem_mask = ~0);
	void hng64_irqc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void hng64_mips_to_iomcu_irq_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t hng64_dualport_r(offs_t offset);
	void hng64_dualport_w(offs_t offset, uint8_t data);

	uint8_t hng64_fbcontrol_r(offs_t offset);
	void hng64_fbcontrol_w(offs_t offset, uint8_t data);

	void hng64_fbunkpair_w(offs_t offset, uint16_t data);
	void hng64_fbscroll_w(offs_t offset, uint16_t data);

	void hng64_fbunkbyte_w(offs_t offset, uint8_t data);

	uint32_t hng64_fbtable_r(offs_t offset, uint32_t mem_mask = ~0);
	void hng64_fbtable_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t hng64_fbram1_r(offs_t offset);
	void hng64_fbram1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t hng64_fbram2_r(offs_t offset);
	void hng64_fbram2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void dl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	//uint32_t dl_r();
	void dl_control_w(uint32_t data);
	void dl_upload_w(uint32_t data);
	void dl_unk_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dl_vreg_r();

	void tcram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t tcram_r(offs_t offset);

	void hng64_soundram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t hng64_soundram_r(offs_t offset);
	void hng64_vregs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// not actually used, but left in code so you can turn it and see the (possibly undesired?) behavior, see notes in memory map
	void hng64_soundram2_w(uint32_t data);
	uint32_t hng64_soundram2_r();

	void hng64_soundcpu_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void hng64_sprite_clear_even_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void hng64_sprite_clear_odd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void hng64_videoram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// shared ram access
	uint8_t ioport0_r();
	void ioport0_w(uint8_t data);
	void ioport7_w(uint8_t data);

	// input port access
	uint8_t ioport3_r();
	void ioport3_w(uint8_t data);
	void ioport1_w(uint8_t data);

	// unknown access
	void ioport4_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( sio0_w );

	uint8_t m_port7 = 0;
	uint8_t m_port1 = 0;

	int m_ex_ramaddr = 0;
	int m_ex_ramaddr_upper = 0;

	TIMER_CALLBACK_MEMBER(tempio_irqon_callback);
	TIMER_CALLBACK_MEMBER(tempio_irqoff_callback);
	emu_timer *m_tempio_irqon_timer = nullptr;
	emu_timer *m_tempio_irqoff_timer = nullptr;
	void init_io();

	void init_hng64_reorder_gfx();

	void set_irq(uint32_t irq_vector);
	uint32_t m_irq_pending = 0;

	TIMER_CALLBACK_MEMBER(comhack_callback);
	emu_timer *m_comhack_timer = nullptr;


	int m_irq_level = 0;
	TILE_GET_INFO_MEMBER(get_hng64_tile0_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile0_16x16_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile1_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile1_16x16_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile2_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile2_16x16_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile3_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile3_16x16_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_hng64(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_hng64);
	TIMER_DEVICE_CALLBACK_MEMBER(hng64_irq);
	void do_dma(address_space &space);

	void hng64_mark_all_tiles_dirty(int tilemap);
	void hng64_mark_tile_dirty(int tilemap, int tile_index);
	void hng64_drawtilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tm);

	void hng64_tilemap_draw_roz_core(screen_device &screen, tilemap_t *tmap, const blit_parameters *blit,
		uint32_t startx, uint32_t starty, int incxx, int incxy, int incyx, int incyy, int wraparound);

	void hng64_tilemap_draw_roz(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, tilemap_t *tmap,
		uint32_t startx, uint32_t starty, int incxx, int incxy, int incyx, int incyy,
		int wraparound, uint32_t flags, uint8_t priority, hng64trans_t drawformat);

	void hng64_tilemap_draw_roz_primask(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, tilemap_t *tmap,
		uint32_t startx, uint32_t starty, int incxx, int incxy, int incyx, int incyy,
		int wraparound, uint32_t flags, uint8_t priority, uint8_t priority_mask, hng64trans_t drawformat);

	static void hng64_configure_blit_parameters(blit_parameters *blit, tilemap_t *tmap, bitmap_rgb32 &dest, const rectangle &cliprect, uint32_t flags, uint8_t priority, uint8_t priority_mask, hng64trans_t drawformat);



	std::unique_ptr<hng64_poly_renderer> m_poly_renderer;

	TIMER_CALLBACK_MEMBER(hng64_3dfifo_processed);
	emu_timer *m_3dfifo_timer = nullptr;

	uint16_t* m_vertsrom = nullptr;
	int m_vertsrom_size = 0;
	std::vector<polygon> m_polys;  // HNG64_MAX_POLYGONS

	void clear3d();
	void hng64_command3d(const uint16_t* packet);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void transition_control(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void setCameraTransformation(const uint16_t* packet);
	void setLighting(const uint16_t* packet);
	void set3dFlags(const uint16_t* packet);
	void setCameraProjectionMatrix(const uint16_t* packet);
	void recoverPolygonBlock(const uint16_t* packet, int& numPolys);
	void printPacket(const uint16_t* packet, int hex);
	float uToF(uint16_t input);
	void matmul4(float *product, const float *a, const float *b);
	void vecmatmul4(float *product, const float *a, const float *b);
	float vecDotProduct(const float *a, const float *b);
	void setIdentity(float *matrix);
	void normalize(float* x);

	void reset_sound();
	void reset_net();

	DECLARE_WRITE_LINE_MEMBER(dma_hreq_cb);
	uint8_t dma_memr_cb(offs_t offset);
	void dma_iow3_cb(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(tcu_tm0_cb);
	DECLARE_WRITE_LINE_MEMBER(tcu_tm1_cb);
	DECLARE_WRITE_LINE_MEMBER(tcu_tm2_cb);



	uint16_t hng64_sound_port_0008_r(offs_t offset, uint16_t mem_mask = ~0);
	void hng64_sound_port_0008_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void hng64_sound_port_000a_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void hng64_sound_port_000c_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void hng64_sound_port_0080_w(uint16_t data);

	void hng64_sound_bank_w(offs_t offset, uint16_t data);
	uint16_t main_sound_comms_r(offs_t offset);
	void main_sound_comms_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sound_comms_r(offs_t offset);
	void sound_comms_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t main_latch[2]{};
	uint16_t sound_latch[2]{};
	void hng64_audio(machine_config &config);
	void hng64_network(machine_config &config);
	void hng_comm_io_map(address_map &map);
	void hng_comm_map(address_map &map);
	void hng_map(address_map &map);
	void hng_sound_io(address_map &map);
	void hng_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_HNG64_H
