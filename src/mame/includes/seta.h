// license:BSD-3-Clause
// copyright-holders:Luca Elia

/***************************************************************************

                            -= Seta Hardware =-

***************************************************************************/

#include "machine/74157.h"
#include "machine/adc083x.h"
#include "machine/gen_latch.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/upd4701.h"
#include "machine/upd4992.h"
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
	seta_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this,"sub"),
		m_seta001(*this, "spritegen"),
		m_x1(*this, "x1snd"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_upd4701(*this, "upd4701"),
		m_buttonmux(*this, "buttonmux"),
		m_adc(*this, "adc"),
		m_dsw(*this, "DSW"),
		m_rot(*this, {"ROT1", "ROT2"}),
		m_gun_inputs(*this, {"GUNX1", "GUNY1", "GUNX2", "GUNY2"}),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_coins(*this, "COINS"),
		m_extra_port(*this, "EXTRA"),
		m_track1_x(*this, "TRACK1_X"),
		m_track1_y(*this, "TRACK1_Y"),
		m_track2_x(*this, "TRACK2_X"),
		m_track2_y(*this, "TRACK2_Y"),
		m_sharedram(*this,"sharedram"),
		m_vregs(*this,"vregs"),
		m_vram_0(*this,"vram_0"),
		m_vctrl_0(*this,"vctrl_0"),
		m_vram_2(*this,"vram_2"),
		m_vctrl_2(*this,"vctrl_2"),
		m_paletteram(*this,"paletteram"),
		m_paletteram2(*this,"paletteram2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	required_device<seta001_device> m_seta001;
	optional_device<x1_010_device> m_x1;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<generic_latch_8_device> m_soundlatch2;
	optional_device<upd4701_device> m_upd4701;
	optional_device<hc157_device> m_buttonmux;
	optional_device<adc083x_device> m_adc;

	optional_ioport m_dsw;
	optional_ioport_array<2> m_rot;
	optional_ioport_array<4> m_gun_inputs;
	optional_ioport m_p1;
	optional_ioport m_p2;
	optional_ioport m_coins;
	optional_ioport m_extra_port;
	optional_ioport m_track1_x;
	optional_ioport m_track1_y;
	optional_ioport m_track2_x;
	optional_ioport m_track2_y;

	optional_shared_ptr<uint8_t> m_sharedram;
	optional_shared_ptr<uint16_t> m_vregs;
	optional_shared_ptr<uint16_t> m_vram_0;
	optional_shared_ptr<uint16_t> m_vctrl_0;
	optional_shared_ptr<uint16_t> m_vram_2;
	optional_shared_ptr<uint16_t> m_vctrl_2;
	optional_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint16_t> m_paletteram2;

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

	uint8_t m_usclssic_port_select;
	int m_keroppi_prize_hop;
	int m_keroppi_protection_count;

	uint8_t m_twineagl_xram[8];
	int m_twineagl_tilebank[4];

	uint16_t m_magspeed_lights[3];

	uint16_t m_pairslove_protram[0x200];
	uint16_t m_pairslove_protram_old[0x200];
	uint16_t m_downtown_protection[0x200/2];

	uint16_t m_kiwame_row_select;

	DECLARE_READ16_MEMBER(metafox_protection_r);
	DECLARE_WRITE16_MEMBER(seta_vregs_w);
	DECLARE_WRITE16_MEMBER(seta_vram_0_w);
	DECLARE_WRITE16_MEMBER(seta_vram_2_w);
	DECLARE_WRITE16_MEMBER(twineagl_tilebank_w);
	DECLARE_WRITE16_MEMBER(timer_regs_w);
	DECLARE_READ16_MEMBER(sharedram_68000_r);
	DECLARE_WRITE16_MEMBER(sharedram_68000_w);
	DECLARE_WRITE16_MEMBER(sub_ctrl_w);
	DECLARE_READ16_MEMBER(seta_dsw_r);
	DECLARE_READ16_MEMBER(usclssic_dsw_r);
	DECLARE_CUSTOM_INPUT_MEMBER(usclssic_trackball_x_r);
	DECLARE_CUSTOM_INPUT_MEMBER(usclssic_trackball_y_r);
	DECLARE_WRITE8_MEMBER(usclssic_lockout_w);
	ADC083X_INPUT_CB(zombraid_adc_cb);
	DECLARE_READ16_MEMBER(zombraid_gun_r);
	DECLARE_WRITE16_MEMBER(zombraid_gun_w);
	DECLARE_READ16_MEMBER(zingzipbl_unknown_r);
	DECLARE_READ16_MEMBER(keroppi_protection_r);
	DECLARE_READ16_MEMBER(keroppi_protection_init_r);
	DECLARE_READ16_MEMBER(keroppi_coin_r);
	DECLARE_WRITE16_MEMBER(keroppi_prize_w);
	DECLARE_WRITE16_MEMBER(msgundam_vregs_w);
	DECLARE_WRITE16_MEMBER(kiwame_row_select_w);
	DECLARE_READ16_MEMBER(kiwame_input_r);
	DECLARE_READ16_MEMBER(thunderl_protection_r);
	DECLARE_WRITE16_MEMBER(thunderl_protection_w);
	DECLARE_WRITE8_MEMBER(utoukond_sound_control_w);
	DECLARE_READ16_MEMBER(pairlove_prot_r);
	DECLARE_WRITE16_MEMBER(pairlove_prot_w);
	DECLARE_WRITE8_MEMBER(sub_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sub_bankswitch_lockout_w);
	DECLARE_READ8_MEMBER(ff_r);
	DECLARE_READ8_MEMBER(downtown_ip_r);
	DECLARE_WRITE8_MEMBER(calibr50_sub_bankswitch_w);
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
	DECLARE_DRIVER_INIT(crazyfgt);
	DECLARE_DRIVER_INIT(metafox);
	DECLARE_DRIVER_INIT(arbalest);
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
	DECLARE_VIDEO_START(seta_1_layer);
	DECLARE_MACHINE_RESET(calibr50);
	DECLARE_PALETTE_INIT(palette_init_RRRRRGGGGGBBBBB_proms);
	DECLARE_PALETTE_INIT(usclssic);
	DECLARE_MACHINE_START(usclssic);
	DECLARE_VIDEO_START(seta_2_layers);
	DECLARE_PALETTE_INIT(blandia);
	DECLARE_PALETTE_INIT(zingzip);
	DECLARE_MACHINE_START(wrofaero);
	DECLARE_PALETTE_INIT(gundhara);
	DECLARE_PALETTE_INIT(jjsquawk);
	DECLARE_MACHINE_START(keroppi);
	DECLARE_VIDEO_START(oisipuzl_2_layers);
	uint32_t screen_update_seta_no_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_seta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_usclssic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_seta_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_seta_buffer_sprites);
	DECLARE_READ16_MEMBER(ipl0_ack_r);
	DECLARE_WRITE16_MEMBER(ipl0_ack_w);
	DECLARE_READ16_MEMBER(ipl1_ack_r);
	DECLARE_WRITE16_MEMBER(ipl1_ack_w);
	DECLARE_READ16_MEMBER(ipl2_ack_r);
	DECLARE_WRITE16_MEMBER(ipl2_ack_w);
	void uPD71054_update_timer(device_t *cpu, int no);
	INTERRUPT_GEN_MEMBER(wrofaero_interrupt);
	TIMER_CALLBACK_MEMBER(uPD71054_timer_callback);
	TIMER_CALLBACK_MEMBER(keroppi_prize_hop_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(seta_interrupt_1_and_2);
	TIMER_DEVICE_CALLBACK_MEMBER(seta_interrupt_2_and_4);
	TIMER_DEVICE_CALLBACK_MEMBER(seta_sub_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(tndrcade_sub_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(calibr50_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(crazyfgt_interrupt);
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
	void keroppij(machine_config &config);
	void madshark(machine_config &config);
	void jjsquawb(machine_config &config);
	void oisipuzl(machine_config &config);
	void zingzipbl(machine_config &config);
	void eightfrc(machine_config &config);
	void gundhara(machine_config &config);
	void triplfun(machine_config &config);
	void calibr50(machine_config &config);
	void blandiap(machine_config &config);
	void wits(machine_config &config);
	void msgundam(machine_config &config);
	void extdwnhl(machine_config &config);
	void pairlove(machine_config &config);
	void zingzip(machine_config &config);
	void wiggie(machine_config &config);
	void umanclub(machine_config &config);
	void tndrcade(machine_config &config);
	void daioh(machine_config &config);
	void atehate(machine_config &config);
	void usclssic(machine_config &config);
	void zombraid(machine_config &config);
	void thunderlbl(machine_config &config);
	void blockcarb(machine_config &config);
	void wrofaero(machine_config &config);
	void downtown(machine_config &config);
	void blockcar(machine_config &config);
	void crazyfgt(machine_config &config);
	void keroppi(machine_config &config);
	void drgnunit(machine_config &config);
	void orbs(machine_config &config);
	void daiohp(machine_config &config);
	void magspeed(machine_config &config);
	void krzybowl(machine_config &config);
	void kiwame(machine_config &config);
	void qzkklgy2(machine_config &config);
	void kamenrid(machine_config &config);
	void superbar(machine_config &config);
	void jjsquawk(machine_config &config);
	void twineagl(machine_config &config);
	void blandia(machine_config &config);
	void thunderl(machine_config &config);
	void metafox(machine_config &config);
	void utoukond(machine_config &config);
	void rezon(machine_config &config);
	void atehate_map(address_map &map);
	void blandia_map(address_map &map);
	void blandiap_map(address_map &map);
	void blockcar_map(address_map &map);
	void blockcarb_map(address_map &map);
	void blockcarb_sound_map(address_map &map);
	void blockcarb_sound_portmap(address_map &map);
	void calibr50_map(address_map &map);
	void calibr50_sub_map(address_map &map);
	void crazyfgt_map(address_map &map);
	void daioh_map(address_map &map);
	void daiohp_map(address_map &map);
	void downtown_map(address_map &map);
	void downtown_sub_map(address_map &map);
	void drgnunit_map(address_map &map);
	void extdwnhl_map(address_map &map);
	void jjsquawb_map(address_map &map);
	void kamenrid_map(address_map &map);
	void keroppi_map(address_map &map);
	void kiwame_map(address_map &map);
	void krzybowl_map(address_map &map);
	void madshark_map(address_map &map);
	void magspeed_map(address_map &map);
	void metafox_sub_map(address_map &map);
	void msgundam_map(address_map &map);
	void oisipuzl_map(address_map &map);
	void orbs_map(address_map &map);
	void pairlove_map(address_map &map);
	void thunderl_map(address_map &map);
	void thunderlbl_map(address_map &map);
	void thunderlbl_sound_map(address_map &map);
	void thunderlbl_sound_portmap(address_map &map);
	void tndrcade_map(address_map &map);
	void tndrcade_sub_map(address_map &map);
	void triplfun_map(address_map &map);
	void twineagl_sub_map(address_map &map);
	void umanclub_map(address_map &map);
	void usclssic_map(address_map &map);
	void utoukond_map(address_map &map);
	void utoukond_sound_io_map(address_map &map);
	void utoukond_sound_map(address_map &map);
	void wiggie_map(address_map &map);
	void wiggie_sound_map(address_map &map);
	void wrofaero_map(address_map &map);
	void zingzipbl_map(address_map &map);
	void zombraid_map(address_map &map);
};

class setaroul_state : public seta_state
{
public:
	setaroul_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_rtc(*this, "rtc"),
		m_hopper(*this, "hopper"),
		m_bet(*this, "BET.%02X", 0),
		m_mux(0),
		m_pay(0),
		m_led(0),
		m_coin_start_cycles(0)
	{ }

	DECLARE_WRITE16_MEMBER(rtc_w);
	DECLARE_READ16_MEMBER(rtc_r);

	DECLARE_READ16_MEMBER(inputs_r);
	DECLARE_WRITE16_MEMBER(mux_w);

	DECLARE_INPUT_CHANGED_MEMBER(coin_drop_start);
	DECLARE_CUSTOM_INPUT_MEMBER(coin_sensors_r);
	DECLARE_CUSTOM_INPUT_MEMBER(hopper_sensors_r);

	DECLARE_WRITE8_MEMBER(pay_w);
	DECLARE_WRITE8_MEMBER(led_w);

	DECLARE_READ16_MEMBER(spritecode_r);
	DECLARE_WRITE16_MEMBER(spritecode_w);

	DECLARE_WRITE16_MEMBER(spriteylow_w);

	DECLARE_WRITE16_MEMBER(spritectrl_w);

	DECLARE_MACHINE_RESET(setaroul);

	DECLARE_VIDEO_START(setaroul_1_layer);
	DECLARE_PALETTE_INIT(setaroul);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void setaroul(machine_config &config);
	void setaroul_map(address_map &map);
private:
	required_device<upd4992_device> m_rtc;  // ! Actually D4911C !
	required_device<ticket_dispenser_device> m_hopper;
	required_ioport_array<26> m_bet;

	uint8_t m_mux;

	uint8_t m_pay;
	uint8_t m_led;

	uint64_t m_coin_start_cycles;

	void show_outputs();
};

class jockeyc_state : public seta_state
{
public:
	jockeyc_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_rtc(*this, "rtc"),
		m_hopper1(*this, "hopper1"), m_hopper2(*this, "hopper2"),
		m_inttoote_700000(*this, "inttoote_700000"),
		m_key1(*this, "KEY1.%u", 0), m_key2(*this, "KEY2.%u", 0),
		m_dsw1(*this, "DSW1"),
		m_dsw2_3(*this, "DSW2_3"),
		m_cabinet(*this, "CABINET"),
		m_mux(0),
		m_out(0)
	{ }

	DECLARE_WRITE16_MEMBER(rtc_w);
	DECLARE_READ16_MEMBER(rtc_r);

	DECLARE_READ16_MEMBER(dsw_r);
	DECLARE_READ16_MEMBER(comm_r);

	DECLARE_READ16_MEMBER(mux_r);
	DECLARE_WRITE16_MEMBER(jockeyc_mux_w);
	DECLARE_WRITE16_MEMBER(jockeyc_out_w);

	DECLARE_READ16_MEMBER(trackball_r);

	DECLARE_VIDEO_START(jockeyc_1_layer);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	DECLARE_WRITE16_MEMBER(inttoote_mux_w);
	DECLARE_WRITE16_MEMBER(inttoote_out_w);
	DECLARE_READ16_MEMBER(inttoote_700000_r);
	DECLARE_DRIVER_INIT(inttoote);
	void inttoote(machine_config &config);
	void jockeyc(machine_config &config);
	void inttoote_map(address_map &map);
	void jockeyc_map(address_map &map);
private:
	required_device<upd4992_device> m_rtc;  // ! Actually D4911C !
	required_device<ticket_dispenser_device> m_hopper1, m_hopper2; // the 2nd hopper is optional

	optional_shared_ptr<uint16_t> m_inttoote_700000;
	required_ioport_array<5> m_key1, m_key2;
	required_ioport m_dsw1, m_dsw2_3;
	optional_ioport m_cabinet;

	uint16_t m_mux;
	uint16_t m_out;

	void update_hoppers();
	void show_outputs();
};
