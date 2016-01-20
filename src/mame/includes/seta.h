// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Seta Hardware =-

***************************************************************************/

#include "sound/x1_010.h"
#include "video/seta001.h"

#define __uPD71054_TIMER    1

struct uPD71054_state
{
	emu_timer *timer[3];            // Timer
	UINT16  max[3];             // Max counter
	UINT16  write_select;       // Max counter write select
	UINT8   reg[4];             //
};

struct game_offset
{
	/* 2 values, for normal and flipped */
	const char *gamename;
	int sprite_offs[2];
	int tilemap_offs[2];
};

class seta_state : public driver_device
{
public:
	seta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this,"sub"),
		m_seta001(*this, "spritegen"),
		m_x1(*this, "x1snd"),
		m_sharedram(*this,"sharedram"),
		m_workram(*this,"workram"),
		m_vregs(*this,"vregs"),
		m_vram_0(*this,"vram_0"),
		m_vctrl_0(*this,"vctrl_0"),
		m_vram_2(*this,"vram_2"),
		m_vctrl_2(*this,"vctrl_2"),
		m_paletteram(*this,"paletteram"),
		m_paletteram2(*this,"paletteram2"),
		m_kiwame_nvram(*this,"kiwame_nvram"),
		m_inttoote_key_select(*this,"inttoote_keysel"),
		m_inttoote_700000(*this,"inttoote_700000"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	required_device<seta001_device> m_seta001;
	optional_device<x1_010_device> m_x1;

	optional_shared_ptr<UINT8> m_sharedram;
	optional_shared_ptr<UINT16> m_workram;
	optional_shared_ptr<UINT16> m_vregs;
	optional_shared_ptr<UINT16> m_vram_0;
	optional_shared_ptr<UINT16> m_vctrl_0;
	optional_shared_ptr<UINT16> m_vram_2;
	optional_shared_ptr<UINT16> m_vctrl_2;
	optional_shared_ptr<UINT16> m_paletteram;
	optional_shared_ptr<UINT16> m_paletteram2;
	optional_shared_ptr<UINT16> m_kiwame_nvram;
	optional_shared_ptr<UINT16> m_inttoote_key_select;
	optional_shared_ptr<UINT16> m_inttoote_700000;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_tiles_offset;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1; // Layer 0
	tilemap_t *m_tilemap_2;
	tilemap_t *m_tilemap_3; // Layer 1
	int m_tilemaps_flip;
	int m_samples_bank;
	int m_color_mode_shift;
	int m_current_tilemap_mode[2];

	uPD71054_state m_uPD71054;
	const game_offset *m_global_offsets;

	bool m_coin_lockout_initialized;
	int m_coin_lockout;

	int m_sub_ctrl_data;

	int m_gun_input_bit;
	int m_gun_input_src;
	int m_gun_bit_count;
	int m_gun_old_clock;

	UINT8 m_usclssic_port_select;
	int m_keroppi_prize_hop;
	int m_keroppi_protection_count;

	int m_wiggie_soundlatch;

	UINT8 m_jockeyc_key_select;

	UINT8 m_twineagl_xram[8];
	int m_twineagl_tilebank[4];

	UINT16 m_magspeed_lights[3];

	UINT16 m_pairslove_protram[0x200];
	UINT16 m_pairslove_protram_old[0x200];
	UINT16 m_downtown_protection[0x200/2];

	DECLARE_WRITE16_MEMBER(seta_vregs_w);
	DECLARE_WRITE16_MEMBER(seta_vram_0_w);
	DECLARE_WRITE16_MEMBER(seta_vram_2_w);
	DECLARE_WRITE16_MEMBER(twineagl_tilebank_w);
	DECLARE_WRITE16_MEMBER(timer_regs_w);
	DECLARE_READ16_MEMBER(sharedram_68000_r);
	DECLARE_WRITE16_MEMBER(sharedram_68000_w);
	DECLARE_WRITE16_MEMBER(sub_ctrl_w);
	DECLARE_READ16_MEMBER(seta_dsw_r);
	DECLARE_READ16_MEMBER(calibr50_ip_r);
	DECLARE_WRITE16_MEMBER(calibr50_soundlatch_w);
	DECLARE_READ16_MEMBER(usclssic_dsw_r);
	DECLARE_READ16_MEMBER(usclssic_trackball_x_r);
	DECLARE_READ16_MEMBER(usclssic_trackball_y_r);
	DECLARE_WRITE16_MEMBER(usclssic_lockout_w);
	DECLARE_READ16_MEMBER(zombraid_gun_r);
	DECLARE_WRITE16_MEMBER(zombraid_gun_w);
	DECLARE_READ16_MEMBER(zingzipbl_unknown_r);
	DECLARE_READ16_MEMBER(keroppi_protection_r);
	DECLARE_READ16_MEMBER(keroppi_protection_init_r);
	DECLARE_READ16_MEMBER(keroppi_coin_r);
	DECLARE_WRITE16_MEMBER(keroppi_prize_w);
	DECLARE_WRITE16_MEMBER(setaroul_spriteylow_w);
	DECLARE_WRITE16_MEMBER(setaroul_spritectrl_w);
	DECLARE_WRITE16_MEMBER(setaroul_spritecode_w);
	DECLARE_READ16_MEMBER(setaroul_spritecode_r);
	DECLARE_READ16_MEMBER(krzybowl_input_r);
	DECLARE_WRITE16_MEMBER(msgundam_vregs_w);
	DECLARE_READ16_MEMBER(kiwame_nvram_r);
	DECLARE_WRITE16_MEMBER(kiwame_nvram_w);
	DECLARE_READ16_MEMBER(kiwame_input_r);
	DECLARE_READ16_MEMBER(thunderl_protection_r);
	DECLARE_WRITE16_MEMBER(thunderl_protection_w);
	DECLARE_READ8_MEMBER(wiggie_soundlatch_r);
	DECLARE_WRITE16_MEMBER(wiggie_soundlatch_w);
	DECLARE_WRITE16_MEMBER(utoukond_soundlatch_w);
	DECLARE_READ16_MEMBER(pairlove_prot_r);
	DECLARE_WRITE16_MEMBER(pairlove_prot_w);
	DECLARE_READ16_MEMBER(inttoote_dsw_r);
	DECLARE_READ16_MEMBER(inttoote_key_r);
	DECLARE_READ16_MEMBER(inttoote_700000_r);
	DECLARE_READ16_MEMBER(jockeyc_mux_r);
	DECLARE_WRITE16_MEMBER(jockeyc_mux_w);
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_WRITE8_MEMBER(sub_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sub_bankswitch_lockout_w);
	DECLARE_READ8_MEMBER(ff_r);
	DECLARE_READ8_MEMBER(downtown_ip_r);
	DECLARE_WRITE8_MEMBER(calibr50_soundlatch2_w);
	DECLARE_READ16_MEMBER(twineagl_debug_r);
	DECLARE_READ16_MEMBER(twineagl_200100_r);
	DECLARE_WRITE16_MEMBER(twineagl_200100_w);
	DECLARE_READ16_MEMBER(downtown_protection_r);
	DECLARE_WRITE16_MEMBER(downtown_protection_w);
	DECLARE_READ16_MEMBER(arbalest_debug_r);
	DECLARE_WRITE16_MEMBER(magspeed_lights_w);
	DECLARE_READ8_MEMBER(dsw1_r);
	DECLARE_READ8_MEMBER(dsw2_r);
	DECLARE_READ16_MEMBER(extra_r);
	DECLARE_DRIVER_INIT(downtown);
	DECLARE_DRIVER_INIT(rezon);
	DECLARE_DRIVER_INIT(twineagl);
	DECLARE_DRIVER_INIT(zombraid);
	DECLARE_DRIVER_INIT(crazyfgt);
	DECLARE_DRIVER_INIT(inttoote);
	DECLARE_DRIVER_INIT(metafox);
	DECLARE_DRIVER_INIT(arbalest);
	DECLARE_DRIVER_INIT(inttootea);
	DECLARE_DRIVER_INIT(wiggie);
	DECLARE_DRIVER_INIT(blandia);
	DECLARE_DRIVER_INIT(kiwame);
	DECLARE_DRIVER_INIT(eightfrc);
	TILE_GET_INFO_MEMBER(twineagl_get_tile_info_0);
	TILE_GET_INFO_MEMBER(twineagl_get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);
	TILE_GET_INFO_MEMBER(get_tile_info_3);
	DECLARE_VIDEO_START(seta_no_layers);
	DECLARE_VIDEO_START(kyustrkr_no_layers);
	DECLARE_VIDEO_START(twineagl_1_layer);
	DECLARE_VIDEO_START(setaroul_1_layer);
	DECLARE_VIDEO_START(seta_1_layer);
	DECLARE_MACHINE_RESET(calibr50);
	DECLARE_PALETTE_INIT(usclssic);
	DECLARE_VIDEO_START(seta_2_layers);
	DECLARE_PALETTE_INIT(blandia);
	DECLARE_PALETTE_INIT(setaroul);
	DECLARE_PALETTE_INIT(zingzip);
	DECLARE_MACHINE_START(wrofaero);
	DECLARE_PALETTE_INIT(gundhara);
	DECLARE_PALETTE_INIT(jjsquawk);
	DECLARE_MACHINE_START(keroppi);
	DECLARE_VIDEO_START(oisipuzl_2_layers);
	DECLARE_PALETTE_INIT(inttoote);
	UINT32 screen_update_seta_no_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_seta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_usclssic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_setaroul(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_inttoote(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_seta_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_seta_buffer_sprites(screen_device &screen, bool state);
	void screen_eof_setaroul(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(wrofaero_interrupt);
	TIMER_CALLBACK_MEMBER(uPD71054_timer_callback);
	TIMER_CALLBACK_MEMBER(keroppi_prize_hop_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(seta_interrupt_1_and_2);
	TIMER_DEVICE_CALLBACK_MEMBER(seta_interrupt_2_and_4);
	TIMER_DEVICE_CALLBACK_MEMBER(seta_sub_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(tndrcade_sub_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(calibr50_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(setaroul_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(crazyfgt_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(inttoote_interrupt);
	void seta_coin_lockout_w(int data);
	inline void twineagl_tile_info( tile_data &tileinfo, int tile_index, int offset );
	inline void get_tile_info( tile_data &tileinfo, int tile_index, int layer, int offset );
	void set_pens();
	void usclssic_set_pens();
	void draw_tilemap_palette_effect(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tilemap, int scrollx, int scrolly, int gfxnum, int flipscreen);
	void seta_layers_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_bank_size, int sprite_setac );
	void uPD71054_timer_init(  );
	DECLARE_WRITE_LINE_MEMBER(pit_out0);
	DECLARE_WRITE_LINE_MEMBER(utoukond_ym3438_interrupt);
	SETA001_SPRITE_GFXBANK_CB_MEMBER(setac_gfxbank_callback);
};
