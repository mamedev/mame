// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, hap, R. Belmont
/***************************************************************************

    Namco System22 / System Super22 hardware

***************************************************************************/

#ifndef MAME_INCLUDES_NAMCOS22_H
#define MAME_INCLUDES_NAMCOS22_H

#pragma once

#include "machine/eeprompar.h"
#include "machine/mb87078.h"
#include "machine/namcomcu.h"
#include "machine/timer.h"
#include "sound/c352.h"
#include "video/rgbutil.h"
#include "video/poly.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

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


struct namcos22_polyvertex
{
	float x, y, z;
	int u, v; /* 0..0xfff */
	int bri;  /* 0..0xff */
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
			float vx, vy;
			float vu, vd, vl, vr;
			int texturebank;
			int color;
			int cmode;
			int flags;
			int cz_adjust;
			int direct;
			namcos22_polyvertex v[4];
		} quad;

		struct
		{
			int tile, color;
			int prioverchar;
			int fade_enabled;
			int flipx, flipy;
			int linktype;
			int cols, rows;
			int xpos, ypos;
			int cx_min, cx_max;
			int cy_min, cy_max;
			int sizex, sizey;
			int translucency;
			int cz;
		} sprite;
	} data;
};


struct namcos22_object_data
{
	/* poly / sprites */
	rgbaint_t fogcolor;
	rgbaint_t fadecolor;
	rgbaint_t polycolor;
	const pen_t *pens;
	bitmap_rgb32 *destbase;
	bitmap_ind8 *primap;
	int bn;
	int flags;
	int prioverchar;
	int cmode;
	int fadefactor;
	int pfade_enabled;
	int fogfactor;
	int zfog_enabled;
	int cz_adjust;
	int cz_sdelta;
	const u8 *czram;

	/* sprites */
	const u8 *source;
	int alpha;
	int line_modulo;
	int flipx;
	int flipy;
};


class namcos22_state;

class namcos22_renderer : public poly_manager<float, namcos22_object_data, 4, 8000>
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

	float m_clipx;
	float m_clipy;
	rectangle m_cliprect;

	inline u8 nthbyte(const u32 *src, int n) { return (src[n / 4] << ((n & 3) * 8)) >> 24; }
	inline u16 nthword(const u32 *src, int n) { return (src[n / 2] << ((n & 1) * 16)) >> 16; }

	void render_scene_nodes(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node);
	void render_sprite(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node);
	void poly3d_drawquad(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node);
	void poly3d_drawsprite(screen_device &screen, bitmap_rgb32 &dest_bmp, u32 code, u32 color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, int cz_factor, int prioverchar, int fade_enabled, int alpha);

	void free_scenenode(struct namcos22_scenenode *node);
	struct namcos22_scenenode *alloc_scenenode(running_machine &machine, struct namcos22_scenenode *node);

	void renderscanline_uvi_full(int32_t scanline, const extent_t &extent, const namcos22_object_data &extra, int threadid);
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
		m_dsw(*this, "DSW"),
		m_inputs(*this, "INPUTS"),
		m_custom(*this, "CUSTOM.%u", 0),
		m_opt(*this, "OPT.%u", 0),
		m_mcuout(*this, "mcuout%u", 0U),
		m_cpuled(*this, "cpuled%u", 0U)
	{ }

	void cybrcomm(machine_config &config);
	void namcos22(machine_config &config);

	void init_acedrvr();
	void init_raveracw();
	void init_ridger2j();
	void init_victlap();
	void init_cybrcomm();
	void init_ridgeraj();

	// renderer
	int m_poly_translucency;
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
	int m_poly_fade_enabled;
	int m_poly_fade_r;
	int m_poly_fade_g;
	int m_poly_fade_b;
	u32 m_fog_colormask;
	int m_fog_r;
	int m_fog_g;
	int m_fog_b;
	std::unique_ptr<u8[]> m_recalc_czram[4];
	int m_fog_r_per_cztype[4];
	int m_fog_g_per_cztype[4];
	int m_fog_b_per_cztype[4];
	u16 m_czattr[8];

	required_device<palette_device> m_palette;
	optional_shared_ptr<u32> m_czram;
	optional_shared_ptr<u32> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
	virtual void device_post_load() override;

//private:
	DECLARE_WRITE32_MEMBER(namcos22_textram_w);
	DECLARE_READ16_MEMBER(namcos22_tilemapattr_r);
	DECLARE_WRITE16_MEMBER(namcos22_tilemapattr_w);
	DECLARE_READ32_MEMBER(namcos22_dspram_r);
	DECLARE_WRITE32_MEMBER(namcos22_dspram_w);
	DECLARE_WRITE32_MEMBER(namcos22_cgram_w);
	DECLARE_WRITE32_MEMBER(namcos22_paletteram_w);
	DECLARE_WRITE16_MEMBER(namcos22_dspram16_bank_w);
	DECLARE_READ16_MEMBER(namcos22_dspram16_r);
	DECLARE_WRITE16_MEMBER(namcos22_dspram16_w);
	DECLARE_READ16_MEMBER(pdp_status_r);
	DECLARE_READ16_MEMBER(pdp_begin_r);
	DECLARE_READ16_MEMBER(dsp_hold_signal_r);
	DECLARE_WRITE16_MEMBER(dsp_hold_ack_w);
	DECLARE_WRITE16_MEMBER(dsp_xf_output_w);
	DECLARE_WRITE16_MEMBER(point_address_w);
	DECLARE_WRITE16_MEMBER(point_loword_iw);
	DECLARE_WRITE16_MEMBER(point_hiword_w);
	DECLARE_READ16_MEMBER(point_loword_r);
	DECLARE_READ16_MEMBER(point_hiword_ir);
	DECLARE_WRITE16_MEMBER(dsp_unk2_w);
	DECLARE_READ16_MEMBER(dsp_unk_port3_r);
	DECLARE_WRITE16_MEMBER(upload_code_to_slave_dsp_w);
	DECLARE_READ16_MEMBER(dsp_unk8_r);
	DECLARE_READ16_MEMBER(custom_ic_status_r);
	DECLARE_READ16_MEMBER(dsp_upload_status_r);
	DECLARE_WRITE16_MEMBER(slave_serial_io_w);
	DECLARE_READ16_MEMBER(master_serial_io_r);
	DECLARE_WRITE16_MEMBER(dsp_unk_porta_w);
	DECLARE_WRITE16_MEMBER(dsp_led_w);
	DECLARE_WRITE16_MEMBER(dsp_unk8_w);
	DECLARE_WRITE16_MEMBER(master_render_device_w);
	DECLARE_READ16_MEMBER(dsp_slave_bioz_r);
	DECLARE_READ16_MEMBER(dsp_slave_port3_r);
	DECLARE_READ16_MEMBER(dsp_slave_port4_r);
	DECLARE_READ16_MEMBER(dsp_slave_port5_r);
	DECLARE_READ16_MEMBER(dsp_slave_port6_r);
	DECLARE_WRITE16_MEMBER(dsp_slave_portc_w);
	DECLARE_READ16_MEMBER(dsp_slave_port8_r);
	DECLARE_READ16_MEMBER(dsp_slave_portb_r);
	DECLARE_WRITE16_MEMBER(dsp_slave_portb_w);
	DECLARE_READ32_MEMBER(namcos22_sci_r);
	DECLARE_WRITE32_MEMBER(namcos22_sci_w);
	DECLARE_READ16_MEMBER(namcos22_shared_r);
	DECLARE_WRITE16_MEMBER(namcos22_shared_w);
	DECLARE_READ16_MEMBER(namcos22_keycus_r);
	DECLARE_WRITE16_MEMBER(namcos22_keycus_w);
	DECLARE_READ16_MEMBER(namcos22_portbit_r);
	DECLARE_WRITE16_MEMBER(namcos22_portbit_w);
	DECLARE_READ16_MEMBER(namcos22_dipswitch_r);
	DECLARE_WRITE16_MEMBER(namcos22_cpuleds_w);
	DECLARE_READ8_MEMBER(mcu_port4_s22_r);
	DECLARE_READ8_MEMBER(iomcu_port4_s22_r);
	DECLARE_READ16_MEMBER(mcuc74_speedup_r);
	DECLARE_WRITE16_MEMBER(mcu_speedup_w);

	inline u8 nthbyte(const u32 *src, int n) { return (src[n / 4] << ((n & 3) * 8)) >> 24; }
	inline u16 nthword(const u32 *src, int n) { return (src[n / 2] << ((n & 1) * 16)) >> 16; }

	inline s32 signed18(s32 val) { return (val & 0x00020000) ? (s32)(val | 0xfffc0000) : val & 0x0001ffff; }
	inline s32 signed24(s32 val) { return (val & 0x00800000) ? (s32)(val | 0xff000000) : val & 0x007fffff; }

	inline float dspfixed_to_nativefloat(s16 val) { return val / (float)0x7fff; }
	float dspfloat_to_nativefloat(u32 val);

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
	DECLARE_READ8_MEMBER(syscon_r);
	DECLARE_WRITE8_MEMBER(ss22_syscon_w);
	DECLARE_WRITE8_MEMBER(s22_syscon_w);

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
	u32 screen_update_namcos22(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(namcos22_interrupt);
	INTERRUPT_GEN_MEMBER(dsp_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(dsp_serial_pulse);

	void iomcu_s22_program(address_map &map);
	void master_dsp_data(address_map &map);
	void master_dsp_io(address_map &map);
	void master_dsp_program(address_map &map);
	void mcu_s22_program(address_map &map);
	void namcos22_am(address_map &map);
	void slave_dsp_data(address_map &map);
	void slave_dsp_io(address_map &map);
	void slave_dsp_program(address_map &map);

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
	required_ioport m_dsw;
	required_ioport m_inputs;
	optional_ioport_array<2> m_custom;
	optional_ioport_array<2> m_opt;
	output_finder<16> m_mcuout;
	output_finder<8> m_cpuled;

	u8 m_syscontrol[0x20];
	bool m_dsp_irq_enabled;
	emu_timer *m_ar_tb_interrupt[2];
	u16 m_dsp_master_bioz;
	std::unique_ptr<u32[]> m_pointram;
	int m_old_coin_state;
	u32 m_credits1;
	u32 m_credits2;
	u32 m_point_address;
	u32 m_point_data;
	u16 m_SerialDataSlaveToMasterNext;
	u16 m_SerialDataSlaveToMasterCurrent;
	int m_RenderBufSize;
	u16 m_RenderBufData[NAMCOS22_MAX_RENDER_CMD_SEQ];
	u16 m_portbits[2];
	int m_irq_state;
	int m_irq_enabled;
	namcos22_dsp_upload_state m_dsp_upload_state;
	int m_UploadDestIdx;
	u16 m_su_82;
	u16 m_keycus_id;
	u16 m_keycus_rng;
	int m_gametype;
	int m_cz_adjust;
	namcos22_renderer *m_poly;
	u16 m_dspram_bank;
	u16 m_dspram16_latch;
	bool m_slave_simulation_active;
	int m_absolute_priority;
	int m_objectshift;
	float m_viewmatrix[4][4];
	u8 m_reflection;
	bool m_cullflip;
	u8 m_LitSurfaceInfo[NAMCOS22_MAX_LIT_SURFACES];
	int m_SurfaceNormalFormat;
	unsigned m_LitSurfaceCount;
	unsigned m_LitSurfaceIndex;
	int m_pointrom_size;
	s32 *m_pointrom;
	std::unique_ptr<u8[]> m_dirtypal;
	std::unique_ptr<bitmap_ind16> m_mix_bitmap;
	tilemap_t *m_bgtilemap;
	u16 m_tilemapattr[8];

	int m_spot_factor;
	int m_text_palbase;
	int m_bg_palbase;

	float m_camera_zoom;
	float m_camera_vx;
	float m_camera_vy;
	float m_camera_vu;
	float m_camera_vd;
	float m_camera_vl;
	float m_camera_vr;
	float m_camera_lx; // unit vector for light direction
	float m_camera_ly; // "
	float m_camera_lz; // "
	int m_camera_ambient; // 0.0..1.0
	int m_camera_power;   // 0.0..1.0

	bool m_skipped_this_frame;
	void render_frame_active();
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	bool m_pdp_render_done;
	bool m_render_refresh;
	uint64_t m_pdp_frame;
	u16 m_pdp_base;
};

class namcos22s_state : public namcos22_state
{
public:
	namcos22s_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos22_state(mconfig, type, tag),
		m_motor_timer(*this, "motor_timer"),
		m_pc_pedal_interrupt(*this, "pc_p_int")
	{ }

	void namcos22s(machine_config &config);
	void propcycl(machine_config &config);
	void dirtdash(machine_config &config);
	void airco22b(machine_config &config);
	void cybrcycc(machine_config &config);
	void tokyowar(machine_config &config);
	void alpine(machine_config &config);
	void alpinesa(machine_config &config);
	void adillor(machine_config &config);
	void timecris(machine_config &config);

	void init_aquajet();
	void init_adillor();
	void init_cybrcyc();
	void init_timecris();
	void init_tokyowar();
	void init_propcycl();
	void init_alpiner2();
	void init_dirtdash();
	void init_airco22();
	void init_alpiner();
	void init_alpinesa();

	template <int N> DECLARE_READ_LINE_MEMBER(alpine_motor_r);

protected:
	virtual void machine_start() override;

	virtual void init_tables() override;
	virtual void draw_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

private:
	DECLARE_MACHINE_START(adillor);

	void install_130_speedup();
	void install_141_speedup();

	void recalc_czram();
	void namcos22s_mix_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int prival);
	u32 screen_update_namcos22s(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE16_MEMBER(namcos22s_czattr_w);
	DECLARE_READ16_MEMBER(namcos22s_czattr_r);
	DECLARE_WRITE32_MEMBER(namcos22s_czram_w);
	DECLARE_READ32_MEMBER(namcos22s_czram_r);
	DECLARE_READ32_MEMBER(namcos22s_vics_control_r);
	DECLARE_WRITE32_MEMBER(namcos22s_vics_control_w);
	DECLARE_READ16_MEMBER(spotram_r);
	DECLARE_WRITE16_MEMBER(spotram_w);

	DECLARE_READ32_MEMBER(alpinesa_prot_r);
	DECLARE_WRITE32_MEMBER(alpinesa_prot_w);
	DECLARE_READ16_MEMBER(timecris_gun_r);
	DECLARE_WRITE8_MEMBER(mb87078_gain_changed);
	DECLARE_WRITE32_MEMBER(namcos22s_chipselect_w);

	DECLARE_WRITE8_MEMBER(mcu_port4_w);
	DECLARE_READ8_MEMBER(mcu_port4_r);
	DECLARE_WRITE8_MEMBER(mcu_port5_w);
	DECLARE_READ8_MEMBER(mcu_port5_r);
	DECLARE_WRITE8_MEMBER(mcu_port6_w);
	DECLARE_READ8_MEMBER(mcu_port6_r);
	template <int Channel> u16 mcu_adc_r();
	DECLARE_WRITE8_MEMBER(alpine_mcu_port4_w);
	DECLARE_READ16_MEMBER(mcu130_speedup_r);
	DECLARE_READ16_MEMBER(mcu141_speedup_r);

	INTERRUPT_GEN_MEMBER(namcos22s_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(adillor_trackball_update);
	TIMER_CALLBACK_MEMBER(adillor_trackball_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(propcycl_pedal_update);
	TIMER_DEVICE_CALLBACK_MEMBER(propcycl_pedal_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(alpine_steplock_callback);

	void alpinesa_am(address_map &map);
	void mcu_program(address_map &map);
	void namcos22s_am(address_map &map);
	void timecris_am(address_map &map);

	optional_device<timer_device> m_motor_timer;
	optional_device<timer_device> m_pc_pedal_interrupt;

	int m_spotram_enable;
	int m_spotram_address;
	std::unique_ptr<u16[]> m_spotram;
	std::unique_ptr<u16[]> m_banked_czram[4];
	u32 m_cz_was_written[4];

	u32 m_alpinesa_protection;
	int m_motor_status;
	u8 m_mcu_iocontrol;
	u8 m_mcu_outdata;
	int m_chipselect;
};

#endif // MAME_INCLUDES_NAMCOS22_H
