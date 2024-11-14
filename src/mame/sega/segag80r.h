// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega G-80 raster hardware

*************************************************************************/
#ifndef MAME_SEGA_SEGAG80R_H
#define MAME_SEGA_SEGAG80R_H

#pragma once

#include "segag80.h"
#include "segag80r_a.h"
#include "segaspeech.h"
#include "segausb.h"

#include "machine/i8255.h"
#include "segag80_m.h"
#include "sound/samples.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class sega005_sound_device;

class segag80r_state : public driver_device
{
public:
	segag80r_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_videoram(*this, "videoram"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundbrd(*this, "soundbrd"),
		m_samples(*this, "samples"),
		m_speech(*this, "speech"),
		m_g80_audio(*this, "g80sound"),
		m_usbsnd(*this, "usbsnd"),
		m_005snd(*this, "005"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void g80r_base(machine_config &config);
	void monsterb(machine_config &config);
	void sindbadm(machine_config &config);
	void astrob(machine_config &config);
	void pignewt(machine_config &config);
	void monster2(machine_config &config);
	void sega005(machine_config &config);
	void spaceod(machine_config &config);
	void sega005_sound_board(machine_config &config);
	void spaceod_sound_board(machine_config &config);
	void monsterb_sound_board(machine_config &config);

	void init_waitstates();
	void init_spaceod();
	void init_sindbadm();
	void init_pignewt();
	void init_monsterb();
	void init_005();
	void init_monster2();
	void init_astrob();

	DECLARE_INPUT_CHANGED_MEMBER(service_switch);

	uint8_t m_sound_state[2]{};
	uint8_t m_sound_rate = 0;
	uint16_t m_sound_addr = 0;
	uint8_t m_sound_data = 0;
	uint8_t m_square_state = 0;
	uint8_t m_square_count = 0;
	inline void sega005_update_sound_data();

private:
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_videoram;

	optional_device<sn76496_device> m_sn1;
	optional_device<sn76496_device> m_sn2;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<monsterb_sound_device> m_soundbrd;
	optional_device<samples_device> m_samples;
	optional_device<sega_speech_device> m_speech;
	optional_device<segag80_audio_device_base> m_g80_audio;
	optional_device<usb_sound_device> m_usbsnd;
	optional_device<sega005_sound_device> m_005snd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	std::vector<uint8_t> m_paletteram;

	offs_t m_scrambled_write_pc = 0;

	segag80_decrypt_func m_decrypt{};
	uint8_t m_background_pcb = 0;
	double m_rweights[3]{};
	double m_gweights[3]{};
	double m_bweights[2]{};
	uint8_t m_video_control = 0;
	uint8_t m_video_flip = 0;
	uint8_t m_vblank_latch = 0;
	tilemap_t *m_spaceod_bg_htilemap = nullptr;
	tilemap_t *m_spaceod_bg_vtilemap = nullptr;
	uint16_t m_spaceod_hcounter = 0;
	uint16_t m_spaceod_vcounter = 0;
	uint8_t m_spaceod_fixed_color = 0;
	uint8_t m_spaceod_bg_control = 0;
	uint8_t m_spaceod_bg_detect = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_bg_enable = 0;
	uint8_t m_bg_char_bank = 0;
	uint16_t m_bg_scrollx = 0;
	uint16_t m_bg_scrolly = 0;
	uint8_t m_pignewt_bg_color_offset = 0;

	uint8_t g80r_opcode_r(offs_t offset);
	void mainram_w(offs_t offset, uint8_t data);
	void vidram_w(offs_t offset, uint8_t data);
	void monsterb_vidram_w(offs_t offset, uint8_t data);
	void pignewt_vidram_w(offs_t offset, uint8_t data);
	void sindbadm_vidram_w(offs_t offset, uint8_t data);
	uint8_t mangled_ports_r(offs_t offset);
	uint8_t spaceod_mangled_ports_r(offs_t offset);
	uint8_t spaceod_port_fc_r();
	void coin_count_w(uint8_t data);
	void segag80r_videoram_w(offs_t offset, uint8_t data);
	uint8_t segag80r_video_port_r(offs_t offset);
	void segag80r_video_port_w(offs_t offset, uint8_t data);
	uint8_t spaceod_back_port_r(offs_t offset);
	void spaceod_back_port_w(offs_t offset, uint8_t data);
	void monsterb_videoram_w(offs_t offset, uint8_t data);
	void monsterb_back_port_w(offs_t offset, uint8_t data);
	void pignewt_videoram_w(offs_t offset, uint8_t data);
	void pignewt_back_color_w(offs_t offset, uint8_t data);
	void pignewt_back_port_w(offs_t offset, uint8_t data);
	void sindbadm_videoram_w(offs_t offset, uint8_t data);
	void sindbadm_back_port_w(offs_t offset, uint8_t data);
	void spaceod_sound_w(offs_t offset, uint8_t data);

	void usb_ram_w(offs_t offset, uint8_t data);
	void sindbadm_misc_w(uint8_t data);
	void sindbadm_sn1_SN76496_w(uint8_t data);
	void sindbadm_sn2_SN76496_w(uint8_t data);

	TILE_GET_INFO_MEMBER(spaceod_get_tile_info);
	TILEMAP_MAPPER_MEMBER(spaceod_scan_rows);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_segag80r(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(segag80r_vblank_start);
	IRQ_CALLBACK_MEMBER(segag80r_irq_ack);
	INTERRUPT_GEN_MEMBER(sindbadm_vblank_start);
	void sega005_sound_a_w(uint8_t data);
	void sega005_sound_b_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(vblank_latch_clear);
	void vblank_latch_set();
	void g80_set_palette_entry(int entry, uint8_t data);
	void spaceod_bg_init_palette();
	void draw_videoram(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *transparent_pens);
	void draw_background_spaceod(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background_page_scroll(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background_full_scroll(bitmap_ind16 &bitmap, const rectangle &cliprect);
	offs_t decrypt_offset(offs_t offset);
	inline uint8_t demangle(uint8_t d7d6, uint8_t d5d4, uint8_t d3d2, uint8_t d1d0);
	void monsterb_expand_gfx(const char *region);

	void g80r_opcodes_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
	void main_ppi8255_portmap(address_map &map) ATTR_COLD;
	void sega_315_opcodes_map(address_map &map) ATTR_COLD;
	void sindbadm_portmap(address_map &map) ATTR_COLD;
	void sindbadm_sound_map(address_map &map) ATTR_COLD;

	emu_timer *m_vblank_latch_clear_timer = nullptr;
};


/*----------- defined in audio/segag80r.c -----------*/


class sega005_sound_device : public device_t,
									public device_sound_interface
{
public:
	sega005_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	emu_timer *m_sega005_sound_timer;
	sound_stream *m_sega005_stream;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	// internal state
	TIMER_CALLBACK_MEMBER( sega005_auto_timer );
};

DECLARE_DEVICE_TYPE(SEGA005, sega005_sound_device)

/*----------- defined in video/segag80r.c -----------*/

#define G80_BACKGROUND_NONE         0
#define G80_BACKGROUND_SPACEOD      1
#define G80_BACKGROUND_MONSTERB     2
#define G80_BACKGROUND_PIGNEWT      3
#define G80_BACKGROUND_SINDBADM     4

#endif // MAME_SEGA_SEGAG80R_H
