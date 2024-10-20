// license:BSD-3-Clause
// copyright-holders:R. Belmont, Acho A. Tang, Phil Stroffolino, Olivier Galibert
#ifndef MAME_KONAMI_KONAMIGX_H
#define MAME_KONAMI_KONAMIGX_H

#pragma once

#include "cpu/tms57002/tms57002.h"
#include "machine/adc083x.h"
#include "machine/k053252.h"
#include "machine/timer.h"
#include "sound/k056800.h"
#include "sound/k054539.h"
#include "k053246_k053247_k055673.h"
#include "k053250.h"
#include "video/k053936.h"
#include "k054156_k054157_k056832.h"
#include "k054338.h"
#include "k055555.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class konamigx_state : public driver_device
{
public:
	konamigx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this,"maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_dasp(*this, "dasp")
		, m_k053252(*this, "k053252")
		, m_k055673(*this, "k055673")
		, m_k055555(*this, "k055555")
		, m_k056832(*this, "k056832")
		, m_k054338(*this, "k054338")
		, m_k056800(*this, "k056800")
		, m_k054539_1(*this, "k054539_1")
		, m_k054539_2(*this, "k054539_2")
		, m_k053250_1(*this, "k053250_1")
		, m_k053250_2(*this, "k053250_2")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_workram(*this, "workram")
		, m_psacram(*this, "psacram")
		, m_subpaletteram32(*this, "subpaletteram")
		, m_k053936_0_ctrl(*this, "k053936_0_ctrl")
		, m_k053936_0_linectrl(*this, "k053936_0_line")
		, m_k053936_0_ctrl_16(*this, "k053936_0_ct16")
		, m_k053936_0_linectrl_16(*this, "k053936_0_li16")
		, m_generic_paletteram_32(*this, "paletteram")
		, m_an0(*this, "AN0")
		, m_an1(*this, "AN1")
		, m_light0_x(*this, "LIGHT0_X")
		, m_light0_y(*this, "LIGHT0_Y")
		, m_light1_x(*this, "LIGHT1_X")
		, m_light1_y(*this, "LIGHT1_Y")
		, m_eepromout(*this, "EEPROMOUT")
		, m_use_68020_post_clock_hack(0)
		, m_lamp(*this, "lamp0")
	{ }

	void esc_w(address_space &space, uint32_t data);
	void eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t le2_gun_H_r();
	uint32_t le2_gun_V_r();
	uint32_t type1_roz_r1(offs_t offset);
	uint32_t type1_roz_r2(offs_t offset);
	uint32_t type3_sync_r();
	void type4_prot_w(address_space &space, offs_t offset, uint32_t data);
	void type1_cablamps_w(uint32_t data);
	uint16_t tms57002_status_word_r();
	void tms57002_control_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t K055550_word_r(offs_t offset);
	void K055550_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void K053990_martchmp_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fantjour_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void type3_bank_w(offs_t offset, uint8_t data);
	[[maybe_unused]] void konamigx_555_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	[[maybe_unused]] void konamigx_555_palette2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void konamigx_tilebank_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void konamigx_t1_psacmap_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void konamigx_t4_psacmap_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vblank_irq_ack_w(int state);
	void hblank_irq_ack_w(int state);
	ioport_value gx_rdport1_3_r();
	void init_konamigx();
	TILE_GET_INFO_MEMBER(get_gx_psac_tile_info);
	TILE_GET_INFO_MEMBER(get_gx_psac3_tile_info);
	TILE_GET_INFO_MEMBER(get_gx_psac3_alt_tile_info);
	TILE_GET_INFO_MEMBER(get_gx_psac1a_tile_info);
	TILE_GET_INFO_MEMBER(get_gx_psac1b_tile_info);
	DECLARE_MACHINE_START(konamigx);
	DECLARE_MACHINE_RESET(konamigx);
	DECLARE_VIDEO_START(konamigx_5bpp);
	DECLARE_VIDEO_START(dragoonj);
	DECLARE_VIDEO_START(le2);
	DECLARE_VIDEO_START(konamigx_6bpp);
	DECLARE_VIDEO_START(opengolf);
	DECLARE_VIDEO_START(racinfrc);
	DECLARE_VIDEO_START(konamigx_type3);
	DECLARE_VIDEO_START(konamigx_type4);
	DECLARE_VIDEO_START(konamigx_type4_vsn);
	DECLARE_VIDEO_START(konamigx_type4_sd2);
	uint32_t screen_update_konamigx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_konamigx_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_konamigx_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(konamigx_type2_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(konamigx_type2_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(konamigx_type4_scanline);
	void k054539_irq_gen(int state);
	TIMER_CALLBACK_MEMBER(dmaend_callback);
	TIMER_CALLBACK_MEMBER(boothack_callback);
	double adc0834_callback(uint8_t input);
	K056832_CB_MEMBER(type2_tile_callback);
	K056832_CB_MEMBER(alpha_tile_callback);
	K055673_CB_MEMBER(type2_sprite_callback);
	K055673_CB_MEMBER(dragoonj_sprite_callback);
	K055673_CB_MEMBER(salmndr2_sprite_callback);
	K055673_CB_MEMBER(le2_sprite_callback);

	struct GX_OBJ { int order = 0, offs = 0, code = 0, color = 0; };

	void common_init();
	uint32_t k_6bpp_rom_long_r(offs_t offset, uint32_t mem_mask = ~0);
	void konamigx_mixer     (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,tilemap_t *sub1, int sub1flags,tilemap_t *sub2, int sub2flags,int mixerflags, bitmap_ind16 *extra_bitmap, int rushingheroes_hack);
	void konamigx_mixer_draw(screen_device &Screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,
						tilemap_t *sub1, int sub1flags,
						tilemap_t *sub2, int sub2flags,
						int mixerflags, bitmap_ind16 *extra_bitmap, int rushingheroes_hack,
						GX_OBJ *objpool,
						int *objbuf,
						int nobj
						);


	void gx_draw_basic_tilemaps(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, int code);
	void gx_draw_basic_extended_tilemaps_1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, int code, tilemap_t *sub1, int sub1flags, int rushingheroes_hack, int offs);
	void gx_draw_basic_extended_tilemaps_2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, int code, tilemap_t *sub2, int sub2flags, bitmap_ind16 *extra_bitmap, int offs);

	void konamigx_esc_alert(uint32_t *srcbase, int srcoffs, int count, int mode);
	void konamigx_precache_registers(void);

	void wipezbuf(int noshadow);

	void dmastart_callback(int data);

	void konamigx_mixer_init(screen_device &screen, int objdma);
	void konamigx_objdma(void);
	void generate_sprites(address_space &space, uint32_t src, uint32_t spr, int count);

	void fantjour_dma_install();

	void konamigx_mixer_primode(int mode);

	typedef void (konamigx_state::*esc_cb)(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);

	void tkmmpzdm_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void dragoonj_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void sal2_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void sexyparo_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void tbyahhoo_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void daiskiss_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);

	inline int K053247GX_combine_c18(int attrib);
	inline int K055555GX_decode_objcolor(int c18);
	inline int K055555GX_decode_inpri(int c18);
	int K055555GX_decode_vmixcolor(int layer, int *color);
	[[maybe_unused]] int K055555GX_decode_osmixcolor(int layer, int *color);

	void init_posthack();
	void konamigx_6bpp(machine_config &config);
	void gxtype4(machine_config &config);
	void gxtype4_vsn(machine_config &config);
	void racinfrc(machine_config &config);
	void gxtype4sd2(machine_config &config);
	void konamigx_bios(machine_config &config);
	void gxtype3(machine_config &config);
	void opengolf(machine_config &config);
	void winspike(machine_config &config);
	void le2(machine_config &config);
	void konamigx(machine_config &config);
	void dragoonj(machine_config &config);
	void salmndr2(machine_config &config);
	void tbyahhoo(machine_config &config);
	void gokuparo(machine_config &config);
	void sexyparo(machine_config &config);
	void gx_base_memmap(address_map &map) ATTR_COLD;
	void racinfrc_map(address_map &map) ATTR_COLD;
	void gx_type1_map(address_map &map) ATTR_COLD;
	void gx_type2_map(address_map &map) ATTR_COLD;
	void gx_type3_map(address_map &map) ATTR_COLD;
	void gx_type4_map(address_map &map) ATTR_COLD;
	void gxsndmap(address_map &map) ATTR_COLD;
	void gxtmsmap(address_map &map) ATTR_COLD;

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<tms57002_device> m_dasp;
	required_device<k053252_device> m_k053252;
	required_device<k055673_device> m_k055673;
	required_device<k055555_device> m_k055555;
	required_device<k056832_device> m_k056832;
	optional_device<k054338_device> m_k054338;
	optional_device<k056800_device> m_k056800;
	optional_device<k054539_device> m_k054539_1;
	optional_device<k054539_device> m_k054539_2;
	optional_device<k053250_device> m_k053250_1;
	optional_device<k053250_device> m_k053250_2;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint32_t> m_workram;
	optional_shared_ptr<uint32_t> m_psacram;
	optional_shared_ptr<uint32_t> m_subpaletteram32;
	optional_shared_ptr<uint32_t> m_k053936_0_ctrl;
	optional_shared_ptr<uint32_t> m_k053936_0_linectrl;
	optional_shared_ptr<uint16_t> m_k053936_0_ctrl_16;
	optional_shared_ptr<uint16_t> m_k053936_0_linectrl_16;
	optional_shared_ptr<uint32_t> m_generic_paletteram_32;

	optional_ioport m_an0, m_an1, m_light0_x, m_light0_y, m_light1_x, m_light1_y, m_eepromout;

	uint8_t m_sound_ctrl = 0;
	uint8_t m_sound_intck = 0;
	uint32_t m_fantjour_dma[8]{};
	int m_konamigx_current_frame = 0;
	int m_gx_objdma = 0, m_gx_primode = 0;
	emu_timer *m_dmadelay_timer = nullptr;
	emu_timer *m_boothack_timer = nullptr;
	int m_gx_rdport1_3 = 0, m_gx_syncen = 0;
	int m_gx_cfgport = 0;
	int m_suspension_active = 0, m_resume_trigger = 0;
	int m_last_prot_op = 0, m_last_prot_clk = 0;
	u16 m_last_prot_param = 0;
	uint8_t m_prev_pixel_clock = 0;

	uint8_t m_esc_program[4096]{};
	esc_cb m_esc_cb;

	uint16_t m_prot_data[0x20]{};

	uint16_t *m_gx_spriteram = nullptr;
	std::unique_ptr<uint16_t[]> m_gx_spriteram_alloc;

	// mirrored K054338 settings
	int *m_K054338_shdRGB = nullptr;

	// 1st-Tier GX/MW Variables
	// frequently used registers
	int m_k053247_vrcbk[4]{};
	int m_k053247_coreg = 0, m_k053247_coregshift = 0, m_k053247_opset = 0;
	int m_opri = 0, m_oinprion = 0;
	int m_vcblk[6]{}, m_ocblk = 0;
	int m_vinmix = 0, m_vmixon = 0, m_osinmix = 0, m_osmixon = 0;
	uint8_t  m_gx_wrport1_0 = 0, m_gx_wrport1_1 = 0;
	uint16_t m_gx_wrport2 = 0;

	// 2nd-Tier GX/MW Graphics Variables
	uint8_t *m_gx_objzbuf = nullptr;
	std::unique_ptr<uint8_t[]> m_gx_shdzbuf;
	int m_layer_colorbase[4]{};
	int32_t m_gx_tilebanks[8]{}, m_gx_oldbanks[8]{};
	int m_gx_tilemode = 0, m_gx_rozenable = 0, m_psac_colorbase = 0, m_last_psac_colorbase = 0;
	int m_gx_specialrozenable = 0; // type 1 roz, with voxel height-map, rendered from 2 source tilemaps (which include height data) to temp bitmap for further processing
	int m_gx_rushingheroes_hack = 0;

	tilemap_t *m_gx_psac_tilemap = nullptr, *m_gx_psac_tilemap2 = nullptr;
	std::unique_ptr<bitmap_ind16> m_type3_roz_temp_bitmap;
	tilemap_t *m_gx_psac_tilemap_alt = nullptr;
	int m_konamigx_has_dual_screen = 0;
	int m_konamigx_palformat = 0;
	std::unique_ptr<bitmap_rgb32> m_dualscreen_left_tempbitmap;
	std::unique_ptr<bitmap_rgb32> m_dualscreen_right_tempbitmap;

	/* On Type-1 the K053936 output is rendered to these temporary bitmaps as raw data
	the 'voxel' effect to give the pixels height is a post-process operation on the
	output of the K053936 (this can clearly be seen in videos as large chunks of
	scenary flicker when in the distance due to single pixels in the K053936 output
	becoming visible / invisible due to drawing precision.

	-- however, progress on this has stalled as our K053936 doesn't seem to give
	   the right output for post processing, I suspect the game is using some
	   unsupported flipping modes (probably due to the way it's hooked up to the
	   rest of the chips) which is causing entirely the wrong output.

	-- furthermore video\k053936.cpp contains an implementation of
	   the K053936_zoom_draw named K053936GP_zoom_draw that's only used in konamigx ...


	*/
	std::unique_ptr<bitmap_ind16> m_gxtype1_roz_dstbitmap;
	std::unique_ptr<bitmap_ind16> m_gxtype1_roz_dstbitmap2;
	rectangle m_gxtype1_roz_dstbitmapclip;

	std::unique_ptr<GX_OBJ[]> m_gx_objpool;

	u8 m_type3_psac2_bank = 0;
	u8 m_type3_spriteram_bank = 0;
	//int m_konamigx_type3_psac2_actual_last_bank = 0;
	int m_use_68020_post_clock_hack = 0;
	output_finder<> m_lamp;
};

// Sprite Callbacks

/* callbacks should return color codes in this format:
    fedcba9876543210fedcba9876543210
    ----------------xxxxxxxxxxxxxxxx (bit 00-15: color)
    --------------xx---------------- (bit 16-17: blend code)
    ------------xx------------------ (bit 18-19: brightness code)
    -x------------------------------ (bit 30   : skip shadow)
    x------------------------------- (bit 31   : full shadow)
*/
#define K055555_COLORMASK   0x0000ffff
#define K055555_MIXSHIFT    16
#define K055555_BRTSHIFT    18
#define K055555_SKIPSHADOW  0x40000000
#define K055555_FULLSHADOW  0x80000000



// Centralized Sprites and Layer Blitter

/* Mixer Flags
    fedcba9876543210fedcba9876543210
    --------------------FFEEDDCCBBAA (layer A-F blend modes)
    ----------------DCBA------------ (layer A-D line/row scroll disables)
    ----FFEEDDCCBBAA---------------- (layer A-F mix codes in forced blending)
    ---x---------------------------- (disable shadows)
    --x----------------------------- (disable z-buffering)
*/
#define GXMIX_BLEND_AUTO    0           // emulate all blend effects
#define GXMIX_BLEND_NONE    1           // disable all blend effects
#define GXMIX_BLEND_FAST    2           // simulate translucency
#define GXMIX_BLEND_FORCE   3           // force mix code on selected layer(s)
#define GXMIX_NOLINESCROLL  0x1000      // disable linescroll on selected layer(s)
#define GXMIX_NOSHADOW      0x10000000  // disable all shadows (shadow pens will be skipped)
#define GXMIX_NOZBUF        0x20000000  // disable z-buffering (shadow pens will be drawn as solid)

// Sub Layer Flags
#define GXSUB_K053250   0x10    // chip type: 0=K053936 ROZ+, 1=K053250 LVC
#define GXSUB_4BPP      0x04    //  16 colors
#define GXSUB_5BPP      0x05    //  32 colors
#define GXSUB_8BPP      0x08    // 256 colors

#endif // MAME_KONAMI_KONAMIGX_H
