// license:BSD-3-Clause
// copyright-holders:R. Belmont, Acho A. Tang, Phil Stroffolino, Olivier Galibert

#ifndef MAME_INCLUDES_KONAMIGX_H
#define MAME_INCLUDES_KONAMIGX_H

#include "cpu/m68000/m68000.h"
#include "cpu/tms57002/tms57002.h"
#include "machine/adc083x.h"
#include "machine/eepromser.h"
#include "machine/k053252.h"
#include "screen.h"
#include "sound/k054539.h"
#include "sound/k056800.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053936.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k054338.h"
#include "video/k055555.h"

class konamigx_state : public driver_device
{
public:
	konamigx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_dasp(*this, "dasp"),
		m_video_timings(*this, "video_timings"),
		m_tilemap(*this, "tilemap"),
		m_sprites(*this, "sprites"),
		m_roz(*this, "roz"),
		m_mixer(*this, "mixer"),
		m_blender(*this, "blender"),
		m_k056800(*this, "k056800"),
		m_k054539_1(*this,"k054539_1"),
		m_k054539_2(*this,"k054539_2"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_workram(*this,"workram"),
		m_psacram(*this,"psacram"),
		m_subpaletteram32(*this,"subpaletteram"),
		m_k053936_0_ctrl(*this,"k053936_0_ctrl",32),
		m_k053936_0_linectrl(*this,"k053936_0_line",32),
		m_k053936_0_ctrl_16(*this,"k053936_0_ct16",16),
		m_k053936_0_linectrl_16(*this,"k053936_0_li16",16),
		m_konamigx_type3_psac2_bank(*this,"psac2_bank"),
		m_generic_paletteram_32(*this, "paletteram"),
		m_spriteram(*this, "spriteram"),
		m_an0(*this, "AN0"),
		m_an1(*this, "AN1"),
		m_light0_x(*this, "LIGHT0_X"),
		m_light0_y(*this, "LIGHT0_Y"),
		m_light1_x(*this, "LIGHT1_X"),
		m_light1_y(*this, "LIGHT1_Y"),
		m_use_68020_post_clock_hack(0)
		{ }

	enum {
		IRQ_LANC,
		IRQ_ESC,
		IRQ_SUBCPU,
		IRQ_OBJDMA,
		IRQ_EXTIO,
		IRQ_TRACKBALL,
		IRQ_EXTGR,
		IRQ_TIMER,
		IRQ_VBLANK,
		IRQ_COUNT
	};

	required_device<m68ec020_device> m_maincpu;
	required_device<m68000_device> m_soundcpu;
	required_device<tms57002_device> m_dasp;
	required_device<k053252_device> m_video_timings;
	required_device<k054156_056832_device> m_tilemap;
	required_device<k053246_055673_device> m_sprites;
	optional_device<k053936_device> m_roz;
	required_device<k055555_device> m_mixer;
	required_device<k054338_device> m_blender;
	required_device<k056800_device> m_k056800;
	required_device<k054539_device> m_k054539_1;
	required_device<k054539_device> m_k054539_2;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint32_t> m_workram;
	optional_shared_ptr<uint32_t> m_psacram;
	optional_shared_ptr<uint32_t> m_subpaletteram32;
	optional_shared_ptr<uint16_t> m_k053936_0_ctrl;
	optional_shared_ptr<uint16_t> m_k053936_0_linectrl;
	optional_shared_ptr<uint16_t> m_k053936_0_ctrl_16;
	optional_shared_ptr<uint16_t> m_k053936_0_linectrl_16;
	optional_shared_ptr<uint32_t> m_konamigx_type3_psac2_bank;
	optional_shared_ptr<uint32_t> m_generic_paletteram_32;
	required_shared_ptr<uint32_t> m_spriteram;

	optional_ioport m_an0, m_an1, m_light0_x, m_light0_y, m_light1_x, m_light1_y;

	void set_irq_level(int irq, int level);
	void update_irqs();

	DECLARE_WRITE_LINE_MEMBER(objdmairq_w);
	DECLARE_WRITE_LINE_MEMBER(objdmaact_w);
	DECLARE_WRITE_LINE_MEMBER(vblankirq_w);
	DECLARE_WRITE_LINE_MEMBER(htimerirq_w);
	DECLARE_WRITE16_MEMBER(port1_w);
	DECLARE_WRITE16_MEMBER(port2_w);
	DECLARE_CUSTOM_INPUT_MEMBER(rdport1_r);
	DECLARE_WRITE16_MEMBER(roz_banking_w);

	DECLARE_WRITE32_MEMBER(esc_w);
	DECLARE_WRITE32_MEMBER(eeprom_w);
	DECLARE_WRITE32_MEMBER(control_w);
	DECLARE_READ32_MEMBER(le2_gun_H_r);
	DECLARE_READ32_MEMBER(le2_gun_V_r);
	DECLARE_READ32_MEMBER(type1_roz_r1);
	DECLARE_READ32_MEMBER(type1_roz_r2);
	DECLARE_READ32_MEMBER(type3_sync_r);
	DECLARE_WRITE32_MEMBER(type4_prot_w);
	DECLARE_WRITE32_MEMBER(type1_cablamps_w);
	DECLARE_READ16_MEMBER(tms57002_data_word_r);
	DECLARE_WRITE16_MEMBER(tms57002_data_word_w);
	DECLARE_READ16_MEMBER(tms57002_status_word_r);
	DECLARE_WRITE16_MEMBER(tms57002_control_word_w);
	DECLARE_WRITE32_MEMBER(fantjour_dma_w);
	DECLARE_WRITE32_MEMBER(konamigx_type3_psac2_bank_w);
	DECLARE_WRITE32_MEMBER(konamigx_tilebank_w);
	DECLARE_WRITE32_MEMBER(konamigx_t1_psacmap_w);
	DECLARE_WRITE32_MEMBER(konamigx_t4_psacmap_w);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq_ack_w);
	DECLARE_WRITE_LINE_MEMBER(hblank_irq_ack_w);
	DECLARE_CUSTOM_INPUT_MEMBER(gx_rdport1_3_r);
	DECLARE_DRIVER_INIT(konamigx);
	TILE_GET_INFO_MEMBER(get_gx_psac_tile_info);
	TILE_GET_INFO_MEMBER(get_gx_psac3_tile_info);
	TILE_GET_INFO_MEMBER(get_gx_psac3_alt_tile_info);
	TILE_GET_INFO_MEMBER(get_gx_psac1a_tile_info);
	TILE_GET_INFO_MEMBER(get_gx_psac1b_tile_info);
	DECLARE_MACHINE_START(konamigx);
	DECLARE_MACHINE_RESET(konamigx);
	INTERRUPT_GEN_MEMBER(konamigx_type2_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(konamigx_type2_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(konamigx_type4_scanline);
	DECLARE_WRITE_LINE_MEMBER(k054539_irq_gen);
	TIMER_CALLBACK_MEMBER(dmaend_callback);
	TIMER_CALLBACK_MEMBER(boothack_callback);
	ADC083X_INPUT_CB(adc0834_callback);

	void sprites_wiring(uint32_t output, uint16_t &color, uint16_t &attr);
	void blender_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);
	void mixer_init(bitmap_ind16 **bitmaps);
	void mixer_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);
	void mixer_update_type1(bitmap_ind16 **bitmaps, const rectangle &cliprect);

private:
	static const int irq_to_level[];

	uint32_t m_irqin, m_irqmask;
	int m_objdma; 
	uint16_t m_port1, m_port2;
	uint8_t m_cur_irq;

public:

	void common_init();
	void konamigx_esc_alert(uint32_t *srcbase, int srcoffs, int count, int mode);
	void konamigx_precache_registers(void);

	void wipezbuf(int noshadow);

	void dmastart_callback(int data);

	void fantjour_dma_install();

	void konamigx_mixer_primode(int mode);

	typedef void (konamigx_state::*esc_cb)(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);

	void tkmmpzdm_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void dragoonj_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void sal2_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void sexyparo_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void tbyahhoo_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
	void daiskiss_esc(address_space &space, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);

	uint8_t m_sound_ctrl;
	uint8_t m_sound_intck;
	uint32_t m_fantjour_dma[8];
	int m_konamigx_current_frame;
	int m_gx_objdma, m_gx_primode;
	emu_timer *m_dmadelay_timer;
	emu_timer *m_boothack_timer;
	int m_gx_rdport1_3, m_gx_syncen;
	int m_gx_cfgport;
	int m_suspension_active, m_resume_trigger;
	int m_last_prot_op, m_last_prot_clk;
	uint8_t m_prev_pixel_clock;

	uint8_t m_esc_program[4096];
	esc_cb m_esc_cb;

	uint16_t *m_gx_spriteram;

	// mirrored K054338 settings
	int *m_K054338_shdRGB;

	// 1st-Tier GX/MW Variables
	// frequently used registers
	int m_k053247_vrcbk[4];
	int m_k053247_coreg, m_k053247_coregshift, m_k053247_opset;
	int m_opri, m_oinprion;
	int m_vcblk[6], m_ocblk;
	int m_vinmix, m_vmixon, m_osinmix, m_osmixon;
	uint8_t  m_gx_wrport1_0, m_gx_wrport1_1;
	uint16_t m_gx_wrport2;

	// 2nd-Tier GX/MW Graphics Variables
	uint8_t *m_gx_objzbuf;
	std::unique_ptr<uint8_t[]> m_gx_shdzbuf;
	int m_layer_colorbase[4];
	int32_t m_gx_tilebanks[8], m_gx_oldbanks[8];
	int m_gx_tilemode, m_gx_rozenable, m_psac_colorbase, m_last_psac_colorbase;
	int m_gx_specialrozenable; // type 1 roz, with voxel height-map, rendered from 2 source tilemaps (which include height data) to temp bitmap for further processing
	int m_gx_rushingheroes_hack;

	tilemap_t *m_gx_psac_tilemap, *m_gx_psac_tilemap2;
	std::unique_ptr<bitmap_ind16> m_type3_roz_temp_bitmap;
	tilemap_t *m_gx_psac_tilemap_alt;
	int m_konamigx_has_dual_screen;
	int m_konamigx_palformat;
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

	-- furthermore video\k053936.c contains an implementation of
	   the K053936_zoom_draw named K053936GP_zoom_draw that's only used in konamigx ...


	*/
	std::unique_ptr<bitmap_ind16> m_gxtype1_roz_dstbitmap;
	std::unique_ptr<bitmap_ind16> m_gxtype1_roz_dstbitmap2;
	rectangle m_gxtype1_roz_dstbitmapclip;

	int m_konamigx_type3_psac2_actual_bank;
	//int m_konamigx_type3_psac2_actual_last_bank = 0;

	DECLARE_DRIVER_INIT(posthack);
	int m_use_68020_post_clock_hack;

	void generate_sprites(address_space &space, uint32_t src, uint32_t spr, int count, int prislot, int offx);
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

#endif // MAME_INCLUDES_KONAMIGX_H
