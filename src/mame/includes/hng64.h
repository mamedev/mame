// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
#ifndef MAME_INCLUDES_HNG64_H
#define MAME_INCLUDES_HNG64_H

#pragma once

#include "machine/msm6242.h"
#include "machine/timer.h"
#include "cpu/mips/mips3.h"
#include "cpu/nec/v5x.h"
#include "sound/l7a1045_l6028_dsp_a.h"
#include "video/poly.h"
#include "cpu/tlcs870/tlcs870.h"
#include "machine/mb8421.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

enum hng64trans_t
{
	HNG64_TILEMAP_NORMAL = 1,
	HNG64_TILEMAP_ADDITIVE,
	HNG64_TILEMAP_ALPHA
};

struct blit_parameters
{
	bitmap_rgb32 *      bitmap;
	rectangle           cliprect;
	uint32_t              tilemap_priority_code;
	uint8_t               mask;
	uint8_t               value;
	uint8_t               alpha;
	hng64trans_t        drawformat;
};

#define HNG64_MASTER_CLOCK 50000000


/////////////////
/// 3d Engine ///
/////////////////

struct polyVert
{
	float worldCoords[4];   // World space coordinates (X Y Z 1.0)

	float texCoords[4];     // Texture coordinates (U V 0 1.0) -> OpenGL style...

	float normal[4];        // Normal (X Y Z 1.0)
	float clipCoords[4];    // Homogeneous screen space coordinates (X Y Z W)

	float light[3];         // The intensity of the illumination at this point
};

struct polygon
{
	int n;                      // Number of sides
	polyVert vert[10];          // Vertices (maximum number per polygon is 10 -> 3+6)

	float faceNormal[4];        // Normal of the face overall - for calculating visibility and flat-shading...
	int visible;                // Polygon visibility in scene

	uint8_t texIndex;             // Which texture to draw from (0x00-0x0f)
	uint8_t texType;              // How to index into the texture
	uint8_t texPageSmall;         // Does this polygon use 'small' texture pages?
	uint8_t texPageHorizOffset;   // If it does use small texture pages, how far is this page horizontally offset?
	uint8_t texPageVertOffset;    // If it does use small texture pages, how far is this page vertically offset?

	uint32_t palOffset;           // The base offset where this object's palette starts.
	uint32_t palPageSize;         // The size of the palette page that is being pointed to.

	uint32_t debugColor;          // Will go away someday.  Used to explicitly color polygons for debugging.
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
	uint8_t texType;
	uint8_t texIndex;
	uint8_t texPageSmall;
	uint8_t texPageHorizOffset;
	uint8_t texPageVertOffset;
	int palOffset;
	int palPageSize;
	int debugColor;
};

class hng64_state;

class hng64_poly_renderer : public poly_manager<float, hng64_poly_data, 7, HNG64_MAX_POLYGONS>
{
public:
	hng64_poly_renderer(hng64_state& state);

	void drawShaded(polygon *p);
	void render_scanline(int32_t scanline, const extent_t& extent, const hng64_poly_data& renderData, int threadid);

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

	auto lamps0_out_cb() { return m_lamps_out_cb[0].bind(); }
	auto lamps1_out_cb() { return m_lamps_out_cb[1].bind(); }
	auto lamps2_out_cb() { return m_lamps_out_cb[2].bind(); }
	auto lamps3_out_cb() { return m_lamps_out_cb[3].bind(); }
	auto lamps4_out_cb() { return m_lamps_out_cb[4].bind(); }
	auto lamps5_out_cb() { return m_lamps_out_cb[5].bind(); }
	auto lamps6_out_cb() { return m_lamps_out_cb[6].bind(); }
	auto lamps7_out_cb() { return m_lamps_out_cb[7].bind(); }

	DECLARE_WRITE8_MEMBER(lamps_w) { m_lamps_out_cb[offset](data); }

protected:
	virtual void device_start() override;

private:
	devcb_write8  m_lamps_out_cb[8];
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
		m_cart(*this, "cart"),
		m_sysregs(*this, "sysregs"),
		m_rombase(*this, "rombase"),
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
		m_an_in(*this, "AN%u", 0U),
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

	uint8_t *m_texturerom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

private:
	/* TODO: NOT measured! */
	const int PIXEL_CLOCK = (HNG64_MASTER_CLOCK*2)/4; // x 2 is due to the interlaced screen ...

	const int HTOTAL = 0x200+0x100;
	const int HBEND = 0;
	const int HBSTART = 0x200;

	const int VTOTAL = 264*2;
	const int VBEND = 0;
	const int VBSTART = 224*2;


	required_device<mips3_device> m_maincpu;
	required_device<v53a_device> m_audiocpu;
	required_device<tmp87ph40an_device> m_iomcu;
	required_device<hng64_lamps_device> m_lamps;
	required_device<idt71321_device> m_dt71321_dpram;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<cpu_device> m_comm;
	required_device<msm6242_device> m_rtc;

	required_shared_ptr<uint32_t> m_mainram;
	required_shared_ptr<uint32_t> m_cart;
	required_shared_ptr<uint32_t> m_sysregs;
	required_shared_ptr<uint32_t> m_rombase;
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
	required_ioport_array<8> m_an_in;


	DECLARE_WRITE8_MEMBER(hng64_default_lamps0_w) { logerror("lamps0 %02x\n", data); }
	DECLARE_WRITE8_MEMBER(hng64_default_lamps1_w) { logerror("lamps1 %02x\n", data); }
	DECLARE_WRITE8_MEMBER(hng64_default_lamps2_w) { logerror("lamps2 %02x\n", data); }
	DECLARE_WRITE8_MEMBER(hng64_default_lamps3_w) { logerror("lamps3 %02x\n", data); }
	DECLARE_WRITE8_MEMBER(hng64_default_lamps4_w) { logerror("lamps4 %02x\n", data); }
	DECLARE_WRITE8_MEMBER(hng64_default_lamps5_w) { logerror("lamps5 %02x\n", data); }
	DECLARE_WRITE8_MEMBER(hng64_default_lamps6_w) { logerror("lamps6 %02x\n", data); }
	DECLARE_WRITE8_MEMBER(hng64_default_lamps7_w) { logerror("lamps7 %02x\n", data); }

	DECLARE_WRITE8_MEMBER(hng64_drive_lamps7_w);
	DECLARE_WRITE8_MEMBER(hng64_drive_lamps6_w);
	DECLARE_WRITE8_MEMBER(hng64_drive_lamps5_w);

	DECLARE_WRITE8_MEMBER(hng64_shoot_lamps7_w);
	DECLARE_WRITE8_MEMBER(hng64_shoot_lamps6_w);

	DECLARE_WRITE8_MEMBER(hng64_fight_lamps6_w);

	int m_samsho64_3d_hack;
	int m_roadedge_3d_hack;

	uint8_t m_fbcontrol[4];

	std::unique_ptr<uint16_t[]> m_soundram;
	std::unique_ptr<uint16_t[]> m_soundram2;

	/* Communications stuff */
	std::unique_ptr<uint8_t[]> m_com_op_base;
	std::unique_ptr<uint8_t[]> m_com_virtual_mem;
	uint8_t m_com_shared[8];

	int32_t m_dma_start;
	int32_t m_dma_dst;
	int32_t m_dma_len;

	uint16_t m_mcu_en;

	uint32_t m_activeDisplayList;
	uint32_t m_no_machine_error_code;

	uint32_t m_unk_vreg_toggle;
	uint32_t m_p1_trig;

	//uint32_t *q2;

	uint8_t m_screen_dis;

	struct hng64_tilemap {
		tilemap_t *m_tilemap_8x8;
		tilemap_t *m_tilemap_16x16;
		tilemap_t *m_tilemap_16x16_alt;
	};

	hng64_tilemap m_tilemap[4];

	uint8_t m_additive_tilemap_debug;

	uint32_t m_old_animmask;
	uint32_t m_old_animbits;
	uint16_t m_old_tileflags[4];

	// 3d State
	int m_paletteState3d;
	float m_projectionMatrix[16];
	float m_modelViewMatrix[16];
	float m_cameraMatrix[16];

	float m_lightStrength;
	float m_lightVector[3];

	DECLARE_READ32_MEMBER(hng64_com_r);
	DECLARE_WRITE32_MEMBER(hng64_com_w);
	DECLARE_WRITE8_MEMBER(hng64_com_share_w);
	DECLARE_READ8_MEMBER(hng64_com_share_r);
	DECLARE_WRITE8_MEMBER(hng64_com_share_mips_w);
	DECLARE_READ8_MEMBER(hng64_com_share_mips_r);
	DECLARE_READ32_MEMBER(hng64_sysregs_r);
	DECLARE_WRITE32_MEMBER(hng64_sysregs_w);
	DECLARE_READ32_MEMBER(hng64_rtc_r);
	DECLARE_WRITE32_MEMBER(hng64_rtc_w);
	DECLARE_READ32_MEMBER(hng64_dmac_r);
	DECLARE_WRITE32_MEMBER(hng64_dmac_w);
	DECLARE_READ32_MEMBER(hng64_irqc_r);
	DECLARE_WRITE32_MEMBER(hng64_irqc_w);
	DECLARE_WRITE32_MEMBER(hng64_mips_to_iomcu_irq_w);

	DECLARE_READ8_MEMBER(hng64_dualport_r);
	DECLARE_WRITE8_MEMBER(hng64_dualport_w);

	DECLARE_READ8_MEMBER(hng64_fbcontrol_r);
	DECLARE_WRITE8_MEMBER(hng64_fbcontrol_w);

	DECLARE_WRITE16_MEMBER(hng64_fbunkpair_w);
	DECLARE_WRITE16_MEMBER(hng64_fbscroll_w);

	DECLARE_WRITE8_MEMBER(hng64_fbunkbyte_w);

	DECLARE_READ32_MEMBER(hng64_fbtable_r);
	DECLARE_WRITE32_MEMBER(hng64_fbtable_w);

	DECLARE_READ32_MEMBER(hng64_fbram1_r);
	DECLARE_WRITE32_MEMBER(hng64_fbram1_w);

	DECLARE_READ32_MEMBER(hng64_fbram2_r);
	DECLARE_WRITE32_MEMBER(hng64_fbram2_w);

	DECLARE_WRITE16_MEMBER(dl_w);
	//DECLARE_READ32_MEMBER(dl_r);
	DECLARE_WRITE32_MEMBER(dl_control_w);
	DECLARE_WRITE32_MEMBER(dl_upload_w);
	DECLARE_WRITE32_MEMBER(dl_unk_w);
	DECLARE_READ32_MEMBER(dl_vreg_r);

	DECLARE_WRITE32_MEMBER(tcram_w);
	DECLARE_READ32_MEMBER(tcram_r);

	DECLARE_WRITE32_MEMBER(hng64_soundram_w);
	DECLARE_READ32_MEMBER(hng64_soundram_r);
	DECLARE_WRITE32_MEMBER(hng64_vregs_w);

	// not actually used, but left in code so you can turn it and see the (possibly undesired?) behavior, see notes in memory map
	DECLARE_WRITE32_MEMBER(hng64_soundram2_w);
	DECLARE_READ32_MEMBER(hng64_soundram2_r);

	DECLARE_WRITE32_MEMBER(hng64_soundcpu_enable_w);

	DECLARE_WRITE32_MEMBER(hng64_sprite_clear_even_w);
	DECLARE_WRITE32_MEMBER(hng64_sprite_clear_odd_w);
	DECLARE_WRITE32_MEMBER(hng64_videoram_w);
	DECLARE_READ8_MEMBER(hng64_comm_space_r);
	DECLARE_WRITE8_MEMBER(hng64_comm_space_w);
	DECLARE_READ8_MEMBER(hng64_comm_mmu_r);
	DECLARE_WRITE8_MEMBER(hng64_comm_mmu_w);

	// shared ram access
	DECLARE_READ8_MEMBER(ioport0_r);
	DECLARE_WRITE8_MEMBER(ioport0_w);
	DECLARE_WRITE8_MEMBER(ioport7_w);

	// input port access
	DECLARE_READ8_MEMBER(ioport3_r);
	DECLARE_WRITE8_MEMBER(ioport3_w);
	DECLARE_WRITE8_MEMBER(ioport1_w);

	// unknown access
	DECLARE_WRITE8_MEMBER(ioport4_w);

	// analog input access
	DECLARE_READ8_MEMBER(anport0_r);
	DECLARE_READ8_MEMBER(anport1_r);
	DECLARE_READ8_MEMBER(anport2_r);
	DECLARE_READ8_MEMBER(anport3_r);
	DECLARE_READ8_MEMBER(anport4_r);
	DECLARE_READ8_MEMBER(anport5_r);
	DECLARE_READ8_MEMBER(anport6_r);
	DECLARE_READ8_MEMBER(anport7_r);

	DECLARE_WRITE_LINE_MEMBER( sio0_w );

	uint8_t m_port7;
	uint8_t m_port1;

	int m_ex_ramaddr;
	int m_ex_ramaddr_upper;

	TIMER_CALLBACK_MEMBER(tempio_irqon_callback);
	TIMER_CALLBACK_MEMBER(tempio_irqoff_callback);
	emu_timer *m_tempio_irqon_timer;
	emu_timer *m_tempio_irqoff_timer;
	void init_io();

	void init_hng64_reorder_gfx();

	void set_irq(uint32_t irq_vector);
	uint32_t m_irq_pending;
	uint8_t *m_comm_rom;
	std::unique_ptr<uint8_t[]> m_comm_ram;
	uint8_t m_mmu_regs[8];
	uint32_t m_mmua[6];
	uint16_t m_mmub[6];
	uint8_t read_comm_data(uint32_t offset);
	void write_comm_data(uint32_t offset,uint8_t data);
	TIMER_CALLBACK_MEMBER(comhack_callback);
	emu_timer *m_comhack_timer;


	int m_irq_level;
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




	std::unique_ptr<hng64_poly_renderer> m_poly_renderer;

	TIMER_CALLBACK_MEMBER(hng64_3dfifo_processed);
	emu_timer *m_3dfifo_timer;

	uint16_t* m_vertsrom;
	int m_vertsrom_size;
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
	DECLARE_READ8_MEMBER(dma_memr_cb);
	DECLARE_WRITE8_MEMBER(dma_iow3_cb);
	DECLARE_WRITE_LINE_MEMBER(tcu_tm0_cb);
	DECLARE_WRITE_LINE_MEMBER(tcu_tm1_cb);
	DECLARE_WRITE_LINE_MEMBER(tcu_tm2_cb);



	DECLARE_READ16_MEMBER(hng64_sound_port_0008_r);
	DECLARE_WRITE16_MEMBER(hng64_sound_port_0008_w);

	DECLARE_WRITE16_MEMBER(hng64_sound_port_000a_w);
	DECLARE_WRITE16_MEMBER(hng64_sound_port_000c_w);

	DECLARE_WRITE16_MEMBER(hng64_sound_port_0080_w);

	DECLARE_WRITE16_MEMBER(hng64_sound_bank_w);
	DECLARE_READ16_MEMBER(main_sound_comms_r);
	DECLARE_WRITE16_MEMBER(main_sound_comms_w);
	DECLARE_READ16_MEMBER(sound_comms_r);
	DECLARE_WRITE16_MEMBER(sound_comms_w);
	uint16_t main_latch[2];
	uint16_t sound_latch[2];
	void hng64_audio(machine_config &config);
	void hng64_network(machine_config &config);
	void hng_comm_io_map(address_map &map);
	void hng_comm_map(address_map &map);
	void hng_map(address_map &map);
	void hng_sound_io(address_map &map);
	void hng_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_HNG64_H
