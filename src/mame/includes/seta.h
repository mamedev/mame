// license:BSD-3-Clause
// copyright-holders:Luca Elia

/***************************************************************************

                            -= Seta Hardware =-

***************************************************************************/

#include "machine/gen_latch.h"
#include "sound/x1_010.h"
#include "video/seta001.h"

#define __uPD71054_TIMER    1

struct uPD71054_state
{
	emu_timer *timer[3];            // Timer
	uint16_t  max[3];             // Max counter
	uint16_t  write_select;       // Max counter write select
	uint8_t   reg[4];             //
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
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_dsw(*this, "DSW"),
		m_rot(*this, {"ROT1", "ROT2"}),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_coins(*this, "COINS"),
		m_extra_port(*this, "EXTRA"),
		m_track1_x(*this, "TRACK1_X"),
		m_track1_y(*this, "TRACK1_Y"),
		m_track2_x(*this, "TRACK2_X"),
		m_track2_y(*this, "TRACK2_Y"),
		m_dsw1(*this, "DSW1"),
		m_dsw2_3(*this, "DSW2_3"),
		m_bet(*this, {"BET0", "BET1", "BET2", "BET3", "BET4"}),
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
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<generic_latch_8_device> m_soundlatch2;

	optional_ioport m_dsw;
	optional_ioport_array<2> m_rot;
	optional_ioport m_p1;
	optional_ioport m_p2;
	optional_ioport m_coins;
	optional_ioport m_extra_port;
	optional_ioport m_track1_x;
	optional_ioport m_track1_y;
	optional_ioport m_track2_x;
	optional_ioport m_track2_y;
	optional_ioport m_dsw1;
	optional_ioport m_dsw2_3;
	optional_ioport_array<5> m_bet;

	optional_shared_ptr<uint8_t> m_sharedram;
	optional_shared_ptr<uint16_t> m_workram;
	optional_shared_ptr<uint16_t> m_vregs;
	optional_shared_ptr<uint16_t> m_vram_0;
	optional_shared_ptr<uint16_t> m_vctrl_0;
	optional_shared_ptr<uint16_t> m_vram_2;
	optional_shared_ptr<uint16_t> m_vctrl_2;
	optional_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint16_t> m_paletteram2;
	optional_shared_ptr<uint16_t> m_kiwame_nvram;
	optional_shared_ptr<uint16_t> m_inttoote_key_select;
	optional_shared_ptr<uint16_t> m_inttoote_700000;

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

	uint8_t m_usclssic_port_select;
	int m_keroppi_prize_hop;
	int m_keroppi_protection_count;

	int m_wiggie_soundlatch;

	uint8_t m_jockeyc_key_select;

	uint8_t m_twineagl_xram[8];
	int m_twineagl_tilebank[4];

	uint16_t m_magspeed_lights[3];

	uint16_t m_pairslove_protram[0x200];
	uint16_t m_pairslove_protram_old[0x200];
	uint16_t m_downtown_protection[0x200/2];

	void seta_vregs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void seta_vram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void seta_vram_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void twineagl_tilebank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void timer_regs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sharedram_68000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sharedram_68000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sub_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t seta_dsw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t calibr50_ip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void calibr50_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t usclssic_dsw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t usclssic_trackball_x_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t usclssic_trackball_y_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void usclssic_lockout_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t zombraid_gun_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void zombraid_gun_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t zingzipbl_unknown_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t keroppi_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t keroppi_protection_init_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t keroppi_coin_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void keroppi_prize_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void setaroul_spriteylow_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void setaroul_spritectrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void setaroul_spritecode_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t setaroul_spritecode_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t krzybowl_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void msgundam_vregs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t kiwame_nvram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void kiwame_nvram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t kiwame_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t thunderl_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void thunderl_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t wiggie_soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wiggie_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void utoukond_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pairlove_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pairlove_prot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t inttoote_dsw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t inttoote_key_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t inttoote_700000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t jockeyc_mux_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void jockeyc_mux_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t unk_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sub_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sub_bankswitch_lockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ff_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t downtown_ip_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void calibr50_soundlatch2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t twineagl_debug_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t twineagl_200100_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void twineagl_200100_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t downtown_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void downtown_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t arbalest_debug_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void magspeed_lights_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t dsw1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsw2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint16_t extra_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void init_downtown();
	void init_rezon();
	void init_twineagl();
	void init_zombraid();
	void init_crazyfgt();
	void init_inttoote();
	void init_metafox();
	void init_arbalest();
	void init_inttootea();
	void init_wiggie();
	void init_blandia();
	void init_kiwame();
	void init_eightfrc();
	void twineagl_get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void twineagl_get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_seta_no_layers();
	void video_start_kyustrkr_no_layers();
	void video_start_twineagl_1_layer();
	void video_start_setaroul_1_layer();
	void video_start_seta_1_layer();
	void machine_reset_calibr50();
	void palette_init_usclssic(palette_device &palette);
	void video_start_seta_2_layers();
	void palette_init_blandia(palette_device &palette);
	void palette_init_setaroul(palette_device &palette);
	void palette_init_zingzip(palette_device &palette);
	void machine_start_wrofaero();
	void palette_init_gundhara(palette_device &palette);
	void palette_init_jjsquawk(palette_device &palette);
	void machine_start_keroppi();
	void video_start_oisipuzl_2_layers();
	void palette_init_inttoote(palette_device &palette);
	uint32_t screen_update_seta_no_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_seta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_usclssic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_setaroul(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_inttoote(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_seta_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_seta_buffer_sprites(screen_device &screen, bool state);
	void screen_eof_setaroul(screen_device &screen, bool state);
	void wrofaero_interrupt(device_t &device);
	void uPD71054_timer_callback(void *ptr, int32_t param);
	void keroppi_prize_hop_callback(void *ptr, int32_t param);
	void seta_interrupt_1_and_2(timer_device &timer, void *ptr, int32_t param);
	void seta_interrupt_2_and_4(timer_device &timer, void *ptr, int32_t param);
	void seta_sub_interrupt(timer_device &timer, void *ptr, int32_t param);
	void tndrcade_sub_interrupt(timer_device &timer, void *ptr, int32_t param);
	void calibr50_interrupt(timer_device &timer, void *ptr, int32_t param);
	void setaroul_interrupt(timer_device &timer, void *ptr, int32_t param);
	void crazyfgt_interrupt(timer_device &timer, void *ptr, int32_t param);
	void inttoote_interrupt(timer_device &timer, void *ptr, int32_t param);
	void seta_coin_lockout_w(int data);
	inline void twineagl_tile_info( tile_data &tileinfo, int tile_index, int offset );
	inline void get_tile_info( tile_data &tileinfo, int tile_index, int layer, int offset );
	void set_pens();
	void usclssic_set_pens();
	void draw_tilemap_palette_effect(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tilemap, int scrollx, int scrolly, int gfxnum, int flipscreen);
	void seta_layers_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_bank_size, int sprite_setac );
	void uPD71054_timer_init(  );
	void pit_out0(int state);
	void utoukond_ym3438_interrupt(int state);
	SETA001_SPRITE_GFXBANK_CB_MEMBER(setac_gfxbank_callback);
};
