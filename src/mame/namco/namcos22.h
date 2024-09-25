// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, hap, R. Belmont
/***************************************************************************

    Namco System22 / System Super22 hardware

***************************************************************************/

#ifndef MAME_NAMCO_NAMCOS22_H
#define MAME_NAMCO_NAMCOS22_H

#pragma once

#include "machine/eeprompar.h"
#include "machine/mb87078.h"
#include "namcomcu.h"
#include "machine/timer.h"
#include "sound/c352.h"
#include "video/rgbutil.h"
#include "video/poly.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class namcos22_state;

// higher precision for internal poly.h
typedef double poly3d_t;

enum
{
	NAMCOS22_AIR_COMBAT22,
	NAMCOS22_ALPINE_RACER,
	NAMCOS22_CYBER_COMMANDO,
	NAMCOS22_CYBER_CYCLES,
	NAMCOS22_PROP_CYCLE,
	NAMCOS22_RAVE_RACER,
	NAMCOS22_RIDGE_RACER,
	NAMCOS22_RIDGE_RACER2,
	NAMCOS22_TIME_CRISIS,
	NAMCOS22_VICTORY_LAP,
	NAMCOS22_ACE_DRIVER,
	NAMCOS22_ALPINE_RACER_2,
	NAMCOS22_ALPINE_SURFER,
	NAMCOS22_TOKYO_WARS,
	NAMCOS22_AQUA_JET,
	NAMCOS22_DIRT_DASH,
	NAMCOS22_ARMADILLO_RACING
};

enum namcos22_scenenode_type
{
	NAMCOS22_SCENENODE_NONLEAF,
	NAMCOS22_SCENENODE_QUAD,
	NAMCOS22_SCENENODE_SPRITE
};

#define NAMCOS22_RADIX_BITS 4
#define NAMCOS22_RADIX_BUCKETS (1 << NAMCOS22_RADIX_BITS)
#define NAMCOS22_RADIX_MASK (NAMCOS22_RADIX_BUCKETS - 1)


struct namcos22_polyvertex
{
	float x, y, z;
	int u, v; // 0..0xfff
	int bri;  // 0..0xff
};

struct namcos22_scenenode
{
	namcos22_scenenode_type type;
	struct namcos22_scenenode *next;
	union
	{
		struct
		{
			struct namcos22_scenenode *next[NAMCOS22_RADIX_BUCKETS];
		} nonleaf;

		struct
		{
			int vx, vy;
			int vu, vd, vl, vr;
			int texturebank;
			int color;
			int cmode;
			int cz_value;
			int cz_type;
			int cz_adjust;
			int objectflags;
			int direct;
			namcos22_polyvertex v[4];
		} quad;

		struct
		{
			int tile, color;
			int prioverchar;
			bool fade_enabled;
			int flipx, flipy;
			int linktype;
			int cols, rows;
			int xpos, ypos;
			int cx_min, cx_max;
			int cy_min, cy_max;
			int sizex, sizey;
			int alpha;
			int cz;
		} sprite;
	} data;
};


struct namcos22_object_data
{
	// poly / sprites
	rgbaint_t fogcolor;
	const pen_t *pens;
	bitmap_rgb32 *destbase;
	bitmap_ind8 *primap;
	int bn;
	int prioverchar;
	int cmode;
	bool shade_enabled;
	bool texture_enabled;
	int fogfactor;

	// ss22
	rgbaint_t polycolor;
	rgbaint_t fadecolor;
	int fadefactor;
	bool pfade_enabled;
	bool zfog_enabled;
	int cz_sdelta;
	const u8 *czram;
	bool alpha_enabled;
	int alpha;

	// sprites
	const u8 *source;
	int line_modulo;
	int flipx;
	int flipy;
};


class namcos22_renderer : public poly_manager<poly3d_t, namcos22_object_data, 4>
{
public:
	namcos22_renderer(namcos22_state &state);

	void render_scene(screen_device &screen, bitmap_rgb32 &bitmap);
	struct namcos22_scenenode *new_scenenode(running_machine &machine, u32 zsort, namcos22_scenenode_type type);

	void init();

private:
	namcos22_state &m_state;

	struct namcos22_scenenode m_scenenode_root;
	struct namcos22_scenenode *m_scenenode_cur;
	std::list<namcos22_scenenode> m_scenenode_alloc;
	rectangle m_cliprect;

	static u8 nthbyte(const u32 *src, int n) { return util::big_endian_cast<u8>(src)[n]; }
	static u16 nthword(const u32 *src, int n) { return util::big_endian_cast<u16>(src)[n]; }

	void render_scene_nodes(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node);
	void render_sprite(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node);
	void poly3d_drawquad(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node);
	void poly3d_drawsprite(screen_device &screen, bitmap_rgb32 &dest_bmp, u32 code, u32 color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int cz_factor, int prioverchar, bool fade_enabled, int alpha);

	void free_scenenode(struct namcos22_scenenode *node);
	struct namcos22_scenenode *alloc_scenenode(running_machine &machine, struct namcos22_scenenode *node);

	void renderscanline_poly(int32_t scanline, const extent_t &extent, const namcos22_object_data &extra, int threadid);
	void renderscanline_poly_ss22(int32_t scanline, const extent_t &extent, const namcos22_object_data &extra, int threadid);
	void renderscanline_sprite(int32_t scanline, const extent_t &extent, const namcos22_object_data &extra, int threadid);
};


enum namcos22_dsp_upload_state
{
	NAMCOS22_DSP_UPLOAD_READY,
	NAMCOS22_DSP_UPLOAD_DEST,
	NAMCOS22_DSP_UPLOAD_DATA
};

#define NAMCOS22_MAX_LIT_SURFACES 0x80
#define NAMCOS22_MAX_RENDER_CMD_SEQ 0x1c

class namcos22_state : public driver_device
{
public:
	namcos22_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_czram(*this, "czram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_maincpu(*this, "maincpu"),
		m_master(*this, "master"),
		m_slave(*this, "slave"),
		m_mcu(*this, "mcu"),
		m_iomcu(*this, "iomcu"),
		m_eeprom(*this, "eeprom"),
		m_mb87078(*this, "mb87078"),
		m_c352(*this, "c352"),
		m_shareram(*this, "shareram"),
		m_slave_extram(*this, "slaveextram"),
		m_master_extram(*this, "masterextram"),
		m_paletteram(*this, "paletteram"),
		m_cgram(*this, "cgram"),
		m_textram(*this, "textram"),
		m_polygonram(*this, "polygonram"),
		m_mixer(*this, "video_mixer"),
		m_gamma_proms(*this, "gamma_proms"),
		m_vics_data(*this, "vics_data"),
		m_vics_control(*this, "vics_control"),
		m_screen(*this, "screen"),
		m_adc_ports(*this, "ADC.%u", 0),
		m_inputs(*this, "INPUTS"),
		m_custom(*this, "CUSTOM.%u", 0),
		m_opt(*this, "OPT.%u", 0),
		m_mcu_out(*this, "mcuout%u", 0U),
		m_cpuled_out(*this, "cpuled%u", 0U)
	{ }

	void cybrcomm(machine_config &config);
	void namcos22(machine_config &config);

	void init_acedrive();
	void init_raverace();
	void init_ridgera2();
	void init_victlap();
	void init_cybrcomm();
	void init_ridgerac();

	// renderer
	u16 *m_texture_tilemap;
	std::unique_ptr<u8[]> m_texture_tileattr;
	u8 *m_texture_tiledata;
	std::unique_ptr<u8[]> m_texture_ayx_to_pixel;
	int m_is_ss22;
	int m_mixer_flags;
	int m_screen_fade_factor;
	int m_screen_fade_r;
	int m_screen_fade_g;
	int m_screen_fade_b;
	bool m_poly_fade_enabled;
	int m_poly_fade_r;
	int m_poly_fade_g;
	int m_poly_fade_b;
	int m_poly_alpha_color;
	int m_poly_alpha_pen;
	int m_poly_alpha_factor;
	u32 m_fog_colormask;
	int m_fog_r;
	int m_fog_g;
	int m_fog_b;
	std::unique_ptr<u8[]> m_recalc_czram[4];
	int m_fog_r_per_cztype[4];
	int m_fog_g_per_cztype[4];
	int m_fog_b_per_cztype[4];
	u16 m_czattr[8] = { };

	required_device<palette_device> m_palette;
	optional_shared_ptr<u32> m_czram;
	optional_shared_ptr<u32> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load() override;

	void namcos22_textram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u16 namcos22_tilemapattr_r(offs_t offset);
	void namcos22_tilemapattr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 namcos22_dspram_r(offs_t offset);
	void namcos22_dspram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void namcos22_cgram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void namcos22_paletteram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void namcos22_dspram16_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 namcos22_dspram16_r(offs_t offset);
	void namcos22_dspram16_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pdp_status_r();
	u16 pdp_begin_r();
	u16 dsp_hold_signal_r();
	void dsp_hold_ack_w(u16 data);
	void dsp_xf_output_w(u16 data);
	void point_address_w(u16 data);
	void point_loword_iw(u16 data);
	void point_hiword_w(u16 data);
	u16 point_loword_r();
	u16 point_hiword_ir();
	void dsp_unk2_w(u16 data);
	u16 dsp_unk_port3_r();
	void upload_code_to_slave_dsp_w(u16 data);
	u16 dsp_unk8_r();
	u16 custom_ic_status_r();
	u16 dsp_upload_status_r();
	void slave_serial_io_w(u16 data);
	u16 master_serial_io_r();
	void dsp_unk_porta_w(u16 data);
	void dsp_led_w(u16 data);
	void dsp_unk8_w(u16 data);
	void master_render_device_w(u16 data);
	u16 dsp_slave_bioz_r();
	u16 dsp_slave_port3_r();
	u16 dsp_slave_port4_r();
	u16 dsp_slave_port5_r();
	u16 dsp_slave_port6_r();
	void dsp_slave_portc_w(u16 data);
	u16 dsp_slave_port8_r();
	u16 dsp_slave_portb_r();
	void dsp_slave_portb_w(u16 data);
	u16 namcos22_sci_r(offs_t offset);
	void namcos22_sci_w(offs_t offset, u16 data);
	u16 namcos22_shared_r(offs_t offset);
	void namcos22_shared_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 namcos22_keycus_r(offs_t offset);
	void namcos22_keycus_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 namcos22_portbit_r(offs_t offset);
	void namcos22_portbit_w(offs_t offset, u16 data);
	void namcos22_cpuleds_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u8 mcu_port4_s22_r();
	u8 iomcu_port4_s22_r();
	u16 mcuc74_speedup_r();
	void mcu_speedup_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	static u8 nthbyte(const u32 *src, int n) { return util::big_endian_cast<u8>(src)[n]; }
	static u16 nthword(const u32 *src, int n) { return util::big_endian_cast<u16>(src)[n]; }

	static constexpr s32 signed12(s32 val) { return util::sext(val, 12); }
	static constexpr s32 signed18(s32 val) { return util::sext(val, 18); }
	static constexpr s32 signed24(s32 val) { return util::sext(val, 24); }

	static constexpr float dspfixed_to_nativefloat(s16 val) { return val / (float)0x7fff; }
	static float dspfloat_to_nativefloat(u32 val);

	void handle_driving_io();
	void handle_coinage(u16 flags);
	void handle_cybrcomm_io();
	void pdp_handle_commands(u16 offs);
	inline u32 pdp_polygonram_read(offs_t offs) { return m_polygonram[offs & 0x7fff]; }
	inline void pdp_polygonram_write(offs_t offs, u32 data) { m_polygonram[offs & 0x7fff] = data; }
	void point_write(offs_t offs, u32 data);
	s32 pointram_read(offs_t offs);
	inline s32 point_read(offs_t offs) { offs &= 0x00ffffff; return (offs < m_pointrom_size) ? m_pointrom[offs] : pointram_read(offs); }
	void master_enable(bool enable);
	void slave_enable(bool enable);

	void syscon_irqlevel(offs_t offset, u8 data);
	void syscon_irqack(offs_t offset, u8 data);
	void syscon_dspcontrol(offs_t offset, u8 data);
	void syscon_mcucontrol(offs_t offset, u8 data);
	u8 syscon_r(offs_t offset);
	void ss22_syscon_w(offs_t offset, u8 data);
	void s22_syscon_w(offs_t offset, u8 data);

	void posirq_update();
	emu_timer *m_posirq_timer;
	TIMER_CALLBACK_MEMBER(posirq_callback);

	void matrix3d_multiply(float a[4][4], float b[4][4]);
	void matrix3d_identity(float m[4][4]);
	void matrix3d_apply_reflection(float m[4][4]);
	void transform_point(float *vx, float *vy, float *vz, float m[4][4]);
	void transform_normal(float *nx, float *ny, float *nz, float m[4][4]);
	void register_normals(int addr, float m[4][4]);

	void blit_single_quad(u32 color, u32 addr, float m[4][4], int polyshift, int flags, int packetformat);
	void blit_quads(int addr, int len, float m[4][4]);
	void blit_polyobject(int code, float m[4][4]);

	void slavesim_handle_bb0003(const s32 *src);
	void slavesim_handle_200002(const s32 *src, int code);
	void slavesim_handle_300000(const s32 *src);
	void slavesim_handle_233002(const s32 *src);
	void simulate_slavedsp();

	virtual void init_tables();
	void update_mixer();
	void update_palette();
	void draw_direct_poly(const u16 *src);
	void draw_polygons();
	void draw_sprites();
	void draw_sprite_group(const u32 *src, const u32 *attr, int num_sprites, int deltax, int deltay, int y_lowres);
	void namcos22_mix_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void install_c74_speedup();

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	virtual void draw_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_text_rowscroll();
	void apply_text_scroll();
	u32 screen_update_namcos22(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(namcos22_interrupt);
	INTERRUPT_GEN_MEMBER(dsp_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(dsp_serial_pulse);

	void iomcu_s22_program(address_map &map) ATTR_COLD;
	void master_dsp_data(address_map &map) ATTR_COLD;
	void master_dsp_io(address_map &map) ATTR_COLD;
	void master_dsp_program(address_map &map) ATTR_COLD;
	void mcu_s22_program(address_map &map) ATTR_COLD;
	void namcos22_am(address_map &map) ATTR_COLD;
	void slave_dsp_data(address_map &map) ATTR_COLD;
	void slave_dsp_io(address_map &map) ATTR_COLD;
	void slave_dsp_program(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_device<m37710_cpu_device> m_mcu;
	optional_device<m37710_cpu_device> m_iomcu;
	required_device<eeprom_parallel_28xx_device> m_eeprom;
	optional_device<mb87078_device> m_mb87078;
	required_device<c352_device> m_c352;
	required_shared_ptr<u16> m_shareram;
	required_shared_ptr<u16> m_slave_extram;
	required_shared_ptr<u16> m_master_extram;
	required_shared_ptr<u32> m_paletteram;
	required_shared_ptr<u32> m_cgram;
	required_shared_ptr<u32> m_textram;
	required_shared_ptr<u32> m_polygonram;
	required_shared_ptr<u32> m_mixer;
	optional_region_ptr<u8> m_gamma_proms;
	optional_shared_ptr<u32> m_vics_data;
	optional_shared_ptr<u32> m_vics_control;
	required_device<screen_device> m_screen;
	optional_ioport_array<8> m_adc_ports;
	required_ioport m_inputs;
	optional_ioport_array<2> m_custom;
	optional_ioport_array<2> m_opt;
	output_finder<16> m_mcu_out;
	output_finder<8> m_cpuled_out;

	u8 m_syscontrol[0x20] = { };
	bool m_dsp_irq_enabled = false;
	u16 m_dsp_master_bioz = 0;
	std::unique_ptr<u32[]> m_pointram;
	int m_old_coin_state = 0;
	u32 m_credits1 = 0;
	u32 m_credits2 = 0;
	u32 m_point_address = 0;
	u32 m_point_data = 0;
	u16 m_SerialDataSlaveToMasterNext = 0;
	u16 m_SerialDataSlaveToMasterCurrent = 0;
	int m_RenderBufSize = 0;
	u16 m_RenderBufData[NAMCOS22_MAX_RENDER_CMD_SEQ] = { };
	u16 m_portbits[2] = { };
	int m_irq_state = 0;
	int m_irq_enabled = 0;
	namcos22_dsp_upload_state m_dsp_upload_state;
	int m_UploadDestIdx = 0;
	u32 m_cpuled_data = 0;
	u16 m_su_82 = 0;
	u16 m_keycus_id = 0;
	u16 m_keycus_rng = 0;
	int m_gametype = 0;
	int m_cz_adjust = 0;
	int m_objectflags = 0;
	std::unique_ptr<namcos22_renderer> m_poly;
	u16 m_dspram_bank = 0;
	u16 m_dspram16_latch = 0;
	bool m_slave_simulation_active = false;
	int m_absolute_priority = 0;
	int m_objectshift = 0;
	float m_viewmatrix[4][4] = { };
	u8 m_reflection = 0;
	bool m_cullflip = false;
	u8 m_LitSurfaceInfo[NAMCOS22_MAX_LIT_SURFACES] = { };
	int m_SurfaceNormalFormat = 0;
	unsigned m_LitSurfaceCount = 0;
	unsigned m_LitSurfaceIndex = 0;
	int m_pointrom_size = 0;
	std::unique_ptr<s32[]> m_pointrom;
	std::unique_ptr<u8[]> m_dirtypal;
	std::unique_ptr<bitmap_ind16> m_mix_bitmap;

	tilemap_t *m_bgtilemap;
	u16 m_tilemapattr[8] = { };
	u16 m_rowscroll[480] = { };
	u16 m_lastrow = 0;
	u64 m_rs_frame = 0;
	int m_spot_factor = 0;
	int m_text_palbase = 0;
	int m_bg_palbase = 0;

	int m_camera_vx = 0;
	int m_camera_vy = 0;
	int m_camera_vu = 0;
	int m_camera_vd = 0;
	int m_camera_vl = 0;
	int m_camera_vr = 0;
	float m_camera_zoom = 0.0f;
	float m_camera_lx = 0.0f; // unit vector for light direction
	float m_camera_ly = 0.0f; // "
	float m_camera_lz = 0.0f; // "
	int m_camera_ambient = 0; // 0.0..1.0
	int m_camera_power = 0;   // 0.0..1.0

	bool m_skipped_this_frame = false;
	void render_frame_active();
	void screen_vblank(int state);
	bool m_pdp_render_done = false;
	bool m_render_refresh = false;
	u64 m_pdp_frame = 0;
	u16 m_pdp_base = 0;
};

class namcos22s_state : public namcos22_state
{
public:
	namcos22s_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos22_state(mconfig, type, tag)
	{ }

	void namcos22s(machine_config &config);
	void dirtdash(machine_config &config);
	void airco22b(machine_config &config);
	void cybrcycc(machine_config &config);
	void tokyowar(machine_config &config);

	void init_aquajet();
	void init_cybrcycc();
	void init_tokyowar();
	void init_dirtdash();
	void init_airco22();

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void init_tables() override;
	virtual void draw_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	void install_130_speedup();
	void install_141_speedup();

	void recalc_czram();
	void namcos22s_mix_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int prival);
	u32 screen_update_namcos22s(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void namcos22s_czattr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 namcos22s_czattr_r(offs_t offset);
	void namcos22s_czram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 namcos22s_czram_r(offs_t offset);
	u32 namcos22s_vics_control_r(offs_t offset);
	void namcos22s_vics_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u16 spotram_r(offs_t offset);
	void spotram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void mb87078_gain_changed(offs_t offset, u8 data);
	void namcos22s_chipselect_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void mcu_port4_w(u8 data);
	u8 mcu_port4_r();
	void mcu_port5_w(u8 data);
	u8 mcu_port5_r();
	void mcu_port6_w(u8 data);
	u8 mcu_port6_r();
	template <int Channel> u16 mcu_adc_r();
	u16 mcu130_speedup_r();
	u16 mcu141_speedup_r();

	INTERRUPT_GEN_MEMBER(namcos22s_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq);

	void mcu_program(address_map &map) ATTR_COLD;
	void namcos22s_am(address_map &map) ATTR_COLD;

	int m_spotram_enable = 0;
	int m_spotram_address = 0;
	std::unique_ptr<u16[]> m_spotram;
	std::unique_ptr<u16[]> m_banked_czram[4];
	u32 m_cz_was_written[4];

	u8 m_mcu_iocontrol = 0;
	u8 m_mcu_outdata = 0;
	int m_chipselect = 0;
};

class alpine_state : public namcos22s_state
{
public:
	alpine_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos22s_state(mconfig, type, tag),
		m_motor_timer(*this, "motor_timer")
	{ }

	void alpine(machine_config &config);
	void init_alpiner();
	void init_alpiner2();

	template <int N> int alpine_motor_r();

protected:
	required_device<timer_device> m_motor_timer;

	virtual void machine_start() override ATTR_COLD;

	void alpine_mcu_port4_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(alpine_steplock_callback);

	int m_motor_status = 2;
};

class alpines_state : public alpine_state
{
public:
	alpines_state(const machine_config &mconfig, device_type type, const char *tag) :
		alpine_state(mconfig, type, tag),
		m_rombank(*this, "rombank")
	{ }

	void alpines(machine_config &config);
	void init_alpines();

private:
	required_memory_bank m_rombank;

	void rombank_w(u32 data);
	void alpines_am(address_map &map) ATTR_COLD;
};

class timecris_state : public namcos22s_state
{
public:
	timecris_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos22s_state(mconfig, type, tag)
	{ }

	void timecris(machine_config &config);
	void init_timecris();

private:
	u16 gun_r(offs_t offset);
	void timecris_am(address_map &map) ATTR_COLD;
};

class propcycl_state : public namcos22s_state
{
public:
	propcycl_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos22s_state(mconfig, type, tag),
		m_pedal_interrupt(*this, "pedal_int")
	{ }

	void propcycl(machine_config &config);
	void init_propcycl();
	void init_propcyclj();

private:
	required_device<timer_device> m_pedal_interrupt;

	TIMER_DEVICE_CALLBACK_MEMBER(pedal_update);
	TIMER_DEVICE_CALLBACK_MEMBER(pedal_interrupt);
};

class adillor_state : public namcos22s_state
{
public:
	adillor_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos22s_state(mconfig, type, tag),
		m_trackball_interrupt(*this, "trackball_int%u", 0),
		m_config_switches(*this, "DEV")
	{ }

	void adillor(machine_config &config);
	void init_adillor();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device_array<timer_device, 2> m_trackball_interrupt;
	required_ioport m_config_switches;

	u32 m_trackball_count[2] = { };
	s32 m_trackball_residual[2] = { };

	TIMER_DEVICE_CALLBACK_MEMBER(trackball_update);
	TIMER_DEVICE_CALLBACK_MEMBER(trackball_interrupt);
};

#endif // MAME_NAMCO_NAMCOS22_H
