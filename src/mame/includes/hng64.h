// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
#include "machine/msm6242.h"
#include "cpu/mips/mips3.h"
#include "cpu/nec/v53.h"
#include "sound/l7a1045_l6028_dsp_a.h"
#include "video/poly.h"

enum
{
	FIGHT_MCU = 1,
	SHOOT_MCU,
	RACING_MCU,
	SAMSHO_MCU,
	BURIKI_MCU
};

enum hng64trans_t
{
	HNG64_TILEMAP_NORMAL = 1,
	HNG64_TILEMAP_ADDITIVE,
	HNG64_TILEMAP_ALPHA
};

struct blit_parameters
{
	bitmap_rgb32 *          bitmap;
	rectangle           cliprect;
	UINT32              tilemap_priority_code;
	UINT8               mask;
	UINT8               value;
	UINT8               alpha;
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
	struct polyVert vert[10];   // Vertices (maximum number per polygon is 10 -> 3+6)

	float faceNormal[4];        // Normal of the face overall - for calculating visibility and flat-shading...
	int visible;                // Polygon visibility in scene

	UINT8 texIndex;             // Which texture to draw from (0x00-0x0f)
	UINT8 texType;              // How to index into the texture
	UINT8 texPageSmall;         // Does this polygon use 'small' texture pages?
	UINT8 texPageHorizOffset;   // If it does use small texture pages, how far is this page horizontally offset?
	UINT8 texPageVertOffset;    // If it does use small texture pages, how far is this page vertically offset?

	UINT32 palOffset;           // The base offset where this object's palette starts.
	UINT32 palPageSize;         // The size of the palette page that is being pointed to.

	UINT32 debugColor;          // Will go away someday.  Used to explicitly color polygons for debugging.
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

struct hng64_poly_data
{
	UINT8 texType;
	UINT8 texIndex;
	UINT8 texPageSmall;
	UINT8 texPageHorizOffset;
	UINT8 texPageVertOffset;
	int palOffset;
	int palPageSize;
	int debugColor;
};

class hng64_state;

class hng64_poly_renderer : public poly_manager<float, hng64_poly_data, 7, HNG64_MAX_POLYGONS>
{
public:
    hng64_poly_renderer(hng64_state& state);
    
    void drawShaded(struct polygon *p);
    void render_scanline(INT32 scanline, const extent_t& extent, const hng64_poly_data& renderData, int threadid);

    hng64_state& state() { return m_state; }
    bitmap_rgb32& colorBuffer3d() { return m_colorBuffer3d; }
    float* depthBuffer3d() { return m_depthBuffer3d; }
    
private:
    hng64_state& m_state;
    
    // (Temporarily class members - someday they will live in the memory map)
    bitmap_rgb32 m_colorBuffer3d;
    float* m_depthBuffer3d;
};



class hng64_state : public driver_device
{
public:
	hng64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dsp(*this, "l7a1045"),
		m_comm(*this, "network"),
		m_rtc(*this, "rtc"),
		m_mainram(*this, "mainram"),
		m_cart(*this, "cart"),
		m_sysregs(*this, "sysregs"),
		m_dualport(*this, "dualport"),
		m_rombase(*this, "rombase"),
		m_spriteram(*this, "spriteram"),
		m_spriteregs(*this, "spriteregs"),
		m_videoram(*this, "videoram"),
		m_videoregs(*this, "videoregs"),
		m_tcram(*this, "tcram"),
		m_3dregs(*this, "3dregs"),
		m_3d_1(*this, "3d_1"),
		m_3d_2(*this, "3d_2"),
		m_com_ram(*this, "com_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<mips3_device> m_maincpu;
	required_device<v53a_device> m_audiocpu;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<cpu_device> m_comm;
	required_device<msm6242_device> m_rtc;

	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_cart;
	required_shared_ptr<UINT32> m_sysregs;
	required_shared_ptr<UINT32> m_dualport;
	required_shared_ptr<UINT32> m_rombase;
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_spriteregs;
	required_shared_ptr<UINT32> m_videoram;
	required_shared_ptr<UINT32> m_videoregs;
	required_shared_ptr<UINT32> m_tcram;

    /* 3D stuff */
	UINT16* m_dl;

	required_shared_ptr<UINT32> m_3dregs;
	required_shared_ptr<UINT32> m_3d_1;
	required_shared_ptr<UINT32> m_3d_2;

	required_shared_ptr<UINT32> m_com_ram;
	//required_shared_ptr<UINT8> m_com_mmu_mem;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	int m_mcu_type;

	UINT16 *m_soundram;
	UINT16 *m_soundram2;

	/* Communications stuff */
	UINT8  *m_com_op_base;
	UINT8  *m_com_virtual_mem;
	UINT8 m_com_shared[8];

	INT32 m_dma_start;
	INT32 m_dma_dst;
	INT32 m_dma_len;

	UINT32 m_mcu_fake_time;
	UINT16 m_mcu_en;

	UINT32 m_activeBuffer;
	UINT32 m_no_machine_error_code;

	UINT32 m_unk_vreg_toggle;
	UINT32 m_p1_trig;

	//UINT32 *q2;

	UINT8 m_screen_dis;

	struct hng64_tilemap {
		tilemap_t *m_tilemap_8x8;
		tilemap_t *m_tilemap_16x16;
		tilemap_t *m_tilemap_16x16_alt;
	};

	hng64_tilemap m_tilemap[4];

	UINT8 m_additive_tilemap_debug;

	UINT32 m_old_animmask;
	UINT32 m_old_animbits;
	UINT16 m_old_tileflags[4];

	// 3d State
	int m_paletteState3d;
	float m_projectionMatrix[16];
	float m_modelViewMatrix[16];
	float m_cameraMatrix[16];

	float m_lightStrength;
	float m_lightVector[3];

	DECLARE_READ32_MEMBER(hng64_random_read);
	DECLARE_READ32_MEMBER(hng64_com_r);
	DECLARE_WRITE32_MEMBER(hng64_com_w);
	DECLARE_WRITE8_MEMBER(hng64_com_share_w);
	DECLARE_READ8_MEMBER(hng64_com_share_r);
	DECLARE_WRITE8_MEMBER(hng64_com_share_mips_w);
	DECLARE_READ8_MEMBER(hng64_com_share_mips_r);
	DECLARE_READ32_MEMBER(hng64_sysregs_r);
	DECLARE_WRITE32_MEMBER(hng64_sysregs_w);
	DECLARE_READ32_MEMBER(fight_io_r);
	DECLARE_READ32_MEMBER(samsho_io_r);
	DECLARE_READ32_MEMBER(shoot_io_r);
	DECLARE_READ32_MEMBER(racing_io_r);
	DECLARE_READ32_MEMBER(hng64_dualport_r);
	DECLARE_WRITE32_MEMBER(hng64_dualport_w);
	DECLARE_READ32_MEMBER(hng64_3d_1_r);
	DECLARE_READ32_MEMBER(hng64_3d_2_r);
	DECLARE_WRITE32_MEMBER(hng64_3d_1_w);
	DECLARE_WRITE32_MEMBER(hng64_3d_2_w);
	DECLARE_WRITE16_MEMBER(dl_w);
	//DECLARE_READ32_MEMBER(dl_r);
	DECLARE_WRITE32_MEMBER(dl_control_w);
	DECLARE_WRITE32_MEMBER(dl_upload_w);
	DECLARE_WRITE32_MEMBER(tcram_w);
	DECLARE_READ32_MEMBER(tcram_r);
	DECLARE_READ32_MEMBER(unk_vreg_r);
	DECLARE_WRITE32_MEMBER(hng64_soundram_w);
	DECLARE_READ32_MEMBER(hng64_soundram_r);
	DECLARE_WRITE32_MEMBER(hng64_vregs_w);

	// not actually used, but left in code so you can turn it and see the (possibly undesired?) behavior, see notes in memory map
	DECLARE_WRITE32_MEMBER(hng64_soundram2_w);
	DECLARE_READ32_MEMBER(hng64_soundram2_r);

	DECLARE_WRITE32_MEMBER(hng64_soundcpu_enable_w);

	DECLARE_WRITE32_MEMBER(hng64_sprite_clear_even_w);
	DECLARE_WRITE32_MEMBER(hng64_sprite_clear_odd_w);
	DECLARE_WRITE32_MEMBER(trap_write);
	DECLARE_WRITE32_MEMBER(activate_3d_buffer);
	DECLARE_READ8_MEMBER(hng64_comm_shared_r);
	DECLARE_WRITE8_MEMBER(hng64_comm_shared_w);
	DECLARE_WRITE32_MEMBER(hng64_videoram_w);
	DECLARE_READ8_MEMBER(hng64_comm_space_r);
	DECLARE_WRITE8_MEMBER(hng64_comm_space_w);
	DECLARE_READ8_MEMBER(hng64_comm_mmu_r);
	DECLARE_WRITE8_MEMBER(hng64_comm_mmu_w);
	DECLARE_DRIVER_INIT(hng64_race);
	DECLARE_DRIVER_INIT(fatfurwa);
	DECLARE_DRIVER_INIT(buriki);
	DECLARE_DRIVER_INIT(hng64);
	DECLARE_DRIVER_INIT(hng64_shoot);
	DECLARE_DRIVER_INIT(ss64);
	DECLARE_DRIVER_INIT(hng64_fght);
	DECLARE_DRIVER_INIT(hng64_reorder_gfx);

	void m_set_irq(UINT32 irq_vector);
	UINT32 m_irq_pending;
	UINT8 *m_comm_rom;
	UINT8 *m_comm_ram;
	UINT8 m_mmu_regs[8];
	UINT32 m_mmua[6];
	UINT16 m_mmub[6];
	UINT8 read_comm_data(UINT32 offset);
	void write_comm_data(UINT32 offset,UINT8 data);
	int m_irq_level;
	TILE_GET_INFO_MEMBER(get_hng64_tile0_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile0_16x16_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile1_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile1_16x16_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile2_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile2_16x16_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile3_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile3_16x16_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_hng64(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_hng64(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(hng64_irq);
	void do_dma(address_space &space);


	DECLARE_CUSTOM_INPUT_MEMBER(left_handle_r);
	DECLARE_CUSTOM_INPUT_MEMBER(right_handle_r);
	DECLARE_CUSTOM_INPUT_MEMBER(acc_down_r);
	DECLARE_CUSTOM_INPUT_MEMBER(brake_down_r);

    hng64_poly_renderer* m_poly_renderer;
    
    void clear3d();
	TIMER_CALLBACK_MEMBER(hng64_3dfifo_processed);

	void hng64_command3d(const UINT16* packet);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void transition_control(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void hng64_tilemap_draw_roz_core(screen_device &screen, tilemap_t *tmap, const blit_parameters *blit,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound);
	void hng64_drawtilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tm);
	void setCameraTransformation(const UINT16* packet);
	void setLighting(const UINT16* packet);
	void set3dFlags(const UINT16* packet);
	void setCameraProjectionMatrix(const UINT16* packet);
	void recoverPolygonBlock(const UINT16* packet, int* numPolys);
	void hng64_mark_all_tiles_dirty(int tilemap);
	void hng64_mark_tile_dirty(int tilemap, int tile_index);

	void hng64_tilemap_draw_roz(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, tilemap_t *tmap,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		int wraparound, UINT32 flags, UINT8 priority, hng64trans_t drawformat);

	void hng64_tilemap_draw_roz_primask(screen_device &screen, bitmap_rgb32 &dest, const rectangle &cliprect, tilemap_t *tmap,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy,
		int wraparound, UINT32 flags, UINT8 priority, UINT8 priority_mask, hng64trans_t drawformat);

	void printPacket(const UINT16* packet, int hex);
	void matmul4(float *product, const float *a, const float *b);
	void vecmatmul4(float *product, const float *a, const float *b);
	float vecDotProduct(const float *a, const float *b);
	void setIdentity(float *matrix);
	float uToF(UINT16 input);
	void normalize(float* x);
	int Inside(struct polyVert *v, int plane);
	void Intersect(struct polyVert *input0, struct polyVert *input1, struct polyVert *output, int plane);
	void performFrustumClip(struct polygon *p);
	UINT8 *m_texturerom;
	UINT16* m_vertsrom;
	int m_vertsrom_size;
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

	DECLARE_WRITE16_MEMBER(hng64_sound_port_0100_w);
	DECLARE_WRITE16_MEMBER(hng64_sound_port_0102_w);
	DECLARE_READ16_MEMBER(hng64_sound_port_0104_r);
	DECLARE_READ16_MEMBER(hng64_sound_port_0106_r);
	DECLARE_WRITE16_MEMBER(hng64_sound_port_0108_w);
	DECLARE_WRITE16_MEMBER(hng64_sound_port_010a_w);

	DECLARE_WRITE16_MEMBER(hng64_sound_bank_w);
	DECLARE_READ16_MEMBER(main_sound_comms_r);
	DECLARE_WRITE16_MEMBER(main_sound_comms_w);
	DECLARE_READ16_MEMBER(sound_comms_r);
	DECLARE_WRITE16_MEMBER(sound_comms_w);
	UINT16 main_latch[2],sound_latch[2];

	std::vector<polygon> polys;//(1024*5);
};
