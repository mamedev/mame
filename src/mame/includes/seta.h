// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_SETA_H
#define MAME_INCLUDES_SETA_H

#pragma once

/***************************************************************************

                            -= Seta Hardware =-

***************************************************************************/

#include "machine/74157.h"
#include "machine/adc083x.h"
#include "machine/gen_latch.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/tmp68301.h"
#include "machine/upd4701.h"
#include "machine/upd4992.h"
#include "sound/x1_010.h"
#include "video/seta001.h"
#include "emupal.h"
#include "tilemap.h"


class seta_state : public driver_device
{
public:
	struct uPD71054_state
	{
		emu_timer *timer[3];            // Timer
		u16  max[3];             // Max counter
		u16  write_select;       // Max counter write select
		u8   reg[4];             //
	};

	struct game_offset
	{
		/* 2 values, for normal and flipped */
		const char *gamename;
		int sprite_offs[2];
		int tilemap_offs[2];
	};

	seta_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this,"sub"),
		m_seta001(*this, "spritegen"),
		m_x1(*this, "x1snd"),
		m_soundlatch(*this, "soundlatch%u", 1U),
		m_dsw(*this, "DSW"),
		m_rot(*this, "ROT%u", 1),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_coins(*this, "COINS"),
		m_extra_port(*this, "EXTRA"),
		m_sharedram(*this,"sharedram"),
		m_vram(*this,"vram_%u", 0U),
		m_vctrl(*this,"vctrl_%u", 0U),
		m_paletteram(*this,"paletteram%u", 1U),
		m_subbank(*this,"subbank"),
		m_x1_bank(*this,"x1_bank"),
		m_leds(*this, "led%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

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
	void msgundamb(machine_config &config);
	void extdwnhl(machine_config &config);
	void pairlove(machine_config &config);
	void zingzip(machine_config &config);
	void wiggie(machine_config &config);
	void umanclub(machine_config &config);
	void tndrcade(machine_config &config);
	void daioh(machine_config &config);
	void atehate(machine_config &config);
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

	void init_bank6502();
	void init_downtown();
	void init_rezon();
	void init_twineagl();
	void init_crazyfgt();
	void init_metafox();
	void init_arbalest();
	void init_wiggie();
	void init_bankx1();
	void init_eightfrc();
	void init_pairlove();

	void palette_init_RRRRRGGGGGBBBBB_proms(palette_device &palette) const;

	SETA001_SPRITE_GFXBANK_CB_MEMBER(setac_gfxbank_callback);

	u32 screen_update_seta_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	required_device<seta001_device> m_seta001;
	optional_device<x1_010_device> m_x1;
	optional_device_array<generic_latch_8_device, 2> m_soundlatch;

	optional_ioport m_dsw;
	optional_ioport_array<2> m_rot;
	optional_ioport m_p1;
	optional_ioport m_p2;
	optional_ioport m_coins;
	optional_ioport m_extra_port;

	optional_shared_ptr<u8> m_sharedram;
	optional_shared_ptr_array<u16, 2> m_vram;
	optional_shared_ptr_array<u16, 2> m_vctrl;
	optional_shared_ptr_array<u16, 2> m_paletteram;

	optional_memory_bank m_subbank;
	optional_memory_bank m_x1_bank;

	output_finder<48> m_leds;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	u8 m_vregs;

	int m_tiles_offset;
	tilemap_t *m_tilemap[2]; // Max 2 Layers
	int m_rambank[2]; // 2 Tilemap banks for each layers
	int m_tilemaps_flip;
	int m_samples_bank;
	int m_color_mode_shift;
	int m_current_tilemap_mode[2];

	uPD71054_state m_uPD71054;
	const game_offset *m_global_offsets;

	int m_sub_ctrl_data;

	int m_keroppi_prize_hop;
	int m_keroppi_protection_count;
	emu_timer *m_keroppi_prize_hop_timer;

	u8 m_twineagl_xram[8];
	int m_twineagl_tilebank[4];

	u16 m_magspeed_lights[3];

	std::unique_ptr<u16[]> m_pairslove_protram;
	std::unique_ptr<u16[]> m_pairslove_protram_old;
	std::unique_ptr<u16[]> m_downtown_protection;

	DECLARE_READ16_MEMBER(metafox_protection_r);
	void seta_coin_counter_w(u8 data);
	void seta_coin_lockout_w(u8 data);
	void seta_vregs_w(u8 data);
	template<int Layer> void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void twineagl_tilebank_w(offs_t offset, u8 data);
	void timer_regs_w(offs_t offset, u16 data);
	u8 sharedram_68000_r(offs_t offset);
	void sharedram_68000_w(offs_t offset, u8 data);
	void sub_ctrl_w(offs_t offset, u8 data);
	u16 seta_dsw_r(offs_t offset);

	u16 zingzipbl_unknown_r();
	u16 keroppi_protection_r();
	u16 keroppi_protection_init_r();
	u16 keroppi_coin_r();
	void keroppi_prize_w(u16 data);
	u16 thunderl_protection_r();
	void thunderl_protection_w(u16 data);
	void utoukond_sound_control_w(u8 data);
	u16 pairlove_prot_r(offs_t offset);
	void pairlove_prot_w(offs_t offset, u16 data);
	void sub_bankswitch_w(u8 data);
	void sub_bankswitch_lockout_w(u8 data);
	u8 ff_r();
	u8 downtown_ip_r(offs_t offset);
	void calibr50_sub_bankswitch_w(u8 data);
	void calibr50_soundlatch2_w(u8 data);
	void twineagl_ctrl_w(u8 data);
	DECLARE_READ16_MEMBER(twineagl_debug_r);
	DECLARE_READ16_MEMBER(twineagl_200100_r);
	DECLARE_WRITE16_MEMBER(twineagl_200100_w);
	DECLARE_READ16_MEMBER(downtown_protection_r);
	DECLARE_WRITE16_MEMBER(downtown_protection_w);
	DECLARE_READ16_MEMBER(arbalest_debug_r);
	void magspeed_lights_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 dsw1_r();
	u8 dsw2_r();
	u16 extra_r();

	TILE_GET_INFO_MEMBER(twineagl_get_tile_info);
	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_VIDEO_START(seta_no_layers);
	DECLARE_VIDEO_START(kyustrkr_no_layers);
	DECLARE_VIDEO_START(twineagl_1_layer);
	DECLARE_VIDEO_START(seta_1_layer);
	DECLARE_MACHINE_RESET(calibr50);

	DECLARE_VIDEO_START(seta_2_layers);
	void blandia_palette(palette_device &palette) const;
	void zingzip_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(wrofaero);
	void gundhara_palette(palette_device &palette) const;
	void jjsquawk_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(keroppi);
	DECLARE_MACHINE_START(magspeed);
	DECLARE_VIDEO_START(oisipuzl_2_layers);
	u32 screen_update_seta_no_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_seta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(screen_vblank_seta_buffer_sprites);
	u16 ipl0_ack_r();
	void ipl0_ack_w(u16 data);
	u16 ipl1_ack_r();
	void ipl1_ack_w(u16 data);
	u16 ipl2_ack_r();
	void ipl2_ack_w(u16 data);
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

	void set_pens();
	void draw_tilemap_palette_effect(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tilemap, int scrollx, int scrolly, int gfxnum, int flipscreen);
	void seta_layers_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_bank_size);
	void uPD71054_timer_init();
	DECLARE_WRITE_LINE_MEMBER(pit_out0);
	DECLARE_WRITE_LINE_MEMBER(utoukond_ym3438_interrupt);

	void atehate_map(address_map &map);
	void blandia_map(address_map &map);
	void blandia_x1_map(address_map &map);
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
	void krzybowl_map(address_map &map);
	void madshark_map(address_map &map);
	void magspeed_map(address_map &map);
	void metafox_sub_map(address_map &map);
	void msgundam_map(address_map &map);
	void msgundamb_map(address_map &map);
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
	void utoukond_map(address_map &map);
	void utoukond_sound_io_map(address_map &map);
	void utoukond_sound_map(address_map &map);
	void wiggie_map(address_map &map);
	void wiggie_sound_map(address_map &map);
	void wrofaero_map(address_map &map);
	void zingzipbl_map(address_map &map);
};

class usclssic_state : public seta_state
{
public:
	usclssic_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_upd4701(*this, "upd4701"),
		m_buttonmux(*this, "buttonmux"),
		m_track_x(*this, "TRACK%u_X", 1U),
		m_track_y(*this, "TRACK%u_Y", 1U)
	{ }

	void usclssic(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(trackball_x_r);
	DECLARE_CUSTOM_INPUT_MEMBER(trackball_y_r);

protected:
	virtual void machine_start() override;

private:
	u16 dsw_r(offs_t offset);
	void lockout_w(u8 data);

	void usclssic_palette(palette_device &palette) const;

	u32 screen_update_usclssic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void usclssic_set_pens();

	void usclssic_map(address_map &map);

	required_device<upd4701_device> m_upd4701;
	required_device<hc157_device> m_buttonmux;
	required_ioport_array<2> m_track_x;
	required_ioport_array<2> m_track_y;

	u8 m_port_select;
};

class kiwame_state : public seta_state
{
public:
	kiwame_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_key(*this, "KEY%u", 0U)
	{ }

	void kiwame(machine_config &config);

private:
	void row_select_w(u16 data);
	u16 input_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(kiwame_vblank);

	void kiwame_map(address_map &map);

	required_device<tmp68301_device> m_maincpu;
	required_ioport_array<10> m_key;

	u16 m_kiwame_row_select;
};

class zombraid_state : public seta_state
{
public:
	zombraid_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_adc(*this, "adc"),
		m_gun_inputs(*this, {"GUNX1", "GUNY1", "GUNX2", "GUNY2"}),
		m_gun_recoil(*this, "Player%u_Gun_Recoil", 1U)
	{ }

	void zombraid(machine_config &config);
	void init_zombraid();

protected:
	DECLARE_MACHINE_START(zombraid);

private:
	double adc_cb(u8 input);
	u16 gun_r();
	void gun_w(u16 data);

	void zombraid_map(address_map &map);
	void zombraid_x1_map(address_map &map);

	required_device<adc083x_device> m_adc;
	required_ioport_array<4> m_gun_inputs;
	output_finder<2> m_gun_recoil;
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

	void setaroul(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_drop_start);
	DECLARE_CUSTOM_INPUT_MEMBER(coin_sensors_r);
	DECLARE_CUSTOM_INPUT_MEMBER(hopper_sensors_r);

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

private:
	DECLARE_WRITE16_MEMBER(rtc_w);
	DECLARE_READ16_MEMBER(rtc_r);

	u16 inputs_r();
	void mux_w(u16 data);

	void pay_w(u8 data);
	void led_w(u8 data);

	DECLARE_READ16_MEMBER(spritecode_r);
	DECLARE_WRITE16_MEMBER(spritecode_w);

	DECLARE_WRITE16_MEMBER(spriteylow_w);

	DECLARE_WRITE16_MEMBER(spritectrl_w);

	DECLARE_MACHINE_START(setaroul);
	DECLARE_MACHINE_RESET(setaroul);

	DECLARE_VIDEO_START(setaroul_1_layer);
	void setaroul_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void setaroul_map(address_map &map);

	required_device<upd4992_device> m_rtc;  // ! Actually D4911C !
	required_device<ticket_dispenser_device> m_hopper;
	required_ioport_array<26> m_bet;

	u8 m_mux;

	u8 m_pay;
	u8 m_led;

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
		m_p1x(*this, "P1X"),
		m_p1y(*this, "P1Y"),
		m_out_cancel(*this, "cancel%u", 1U),
		m_out_payout(*this, "payout%u", 1U),
		m_out_start(*this, "start%u", 1U),
		m_out_help(*this, "help"),
		m_out_itstart(*this, "start"),
		m_mux(0),
		m_out(0)
	{ }

	void inttoote(machine_config &config);
	void jockeyc(machine_config &config);

	void init_inttoote();

private:
	DECLARE_WRITE16_MEMBER(rtc_w);
	DECLARE_READ16_MEMBER(rtc_r);

	u16 dsw_r(offs_t offset);
	u16 comm_r();

	u16 mux_r();
	void jockeyc_mux_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void jockeyc_out_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 trackball_r(offs_t offset);

	DECLARE_MACHINE_START(jockeyc);
	DECLARE_MACHINE_START(inttoote);
	DECLARE_VIDEO_START(jockeyc_1_layer);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void inttoote_mux_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void inttoote_out_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 inttoote_700000_r(offs_t offset);

	void inttoote_map(address_map &map);
	void jockeyc_map(address_map &map);

	required_device<upd4992_device> m_rtc;  // ! Actually D4911C !
	required_device<ticket_dispenser_device> m_hopper1, m_hopper2; // the 2nd hopper is optional

	optional_shared_ptr<u16> m_inttoote_700000;
	required_ioport_array<5> m_key1, m_key2;
	required_ioport m_dsw1, m_dsw2_3;
	optional_ioport m_cabinet;
	optional_ioport m_p1x;
	optional_ioport m_p1y;

	output_finder<2> m_out_cancel;
	output_finder<2> m_out_payout;
	output_finder<2> m_out_start;
	output_finder<> m_out_help;
	output_finder<> m_out_itstart;

	u16 m_mux;
	u16 m_out;

	void update_hoppers();
	void show_outputs();
};

#endif // MAME_INCLUDES_SETA_H
