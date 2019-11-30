// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega G-80 raster hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SEGAG80R_H
#define MAME_INCLUDES_SEGAG80R_H

#pragma once

#include "audio/segag80r.h"
#include "audio/segasnd.h"
#include "machine/i8255.h"
#include "machine/segag80.h"
#include "sound/samples.h"
#include "sound/sn76496.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class sega005_sound_device;

class segag80r_state : public segag80snd_common
{
public:
	segag80r_state(const machine_config &mconfig, device_type type, const char *tag) :
		segag80snd_common(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_videoram(*this, "videoram"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundbrd(*this, "soundbrd"),
		m_samples(*this, "samples"),
		m_speech(*this, "segaspeech"),
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
	void astrob_sound_board(machine_config &config);
	void sega005_sound_board(machine_config &config);
	void spaceod_sound_board(machine_config &config);
	void monsterb_sound_board(machine_config &config);

	void init_spaceod();
	void init_sindbadm();
	void init_pignewt();
	void init_monsterb();
	void init_005();
	void init_monster2();
	void init_astrob();

	DECLARE_INPUT_CHANGED_MEMBER(service_switch);

	uint8_t m_sound_state[2];
	uint8_t m_sound_rate;
	uint16_t m_sound_addr;
	uint8_t m_sound_data;
	uint8_t m_square_state;
	uint8_t m_square_count;
	inline void sega005_update_sound_data();

private:
	enum
	{
		TIMER_VBLANK_LATCH_CLEAR
	};

	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_videoram;

	optional_device<sn76496_device> m_sn1;
	optional_device<sn76496_device> m_sn2;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<monsterb_sound_device> m_soundbrd;
	optional_device<samples_device> m_samples;
	optional_device<speech_sound_device> m_speech;
	optional_device<usb_sound_device> m_usbsnd;
	optional_device<sega005_sound_device> m_005snd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	std::vector<uint8_t> m_paletteram;

	offs_t m_scrambled_write_pc;

	segag80_decrypt_func m_decrypt;
	uint8_t m_background_pcb;
	double m_rweights[3];
	double m_gweights[3];
	double m_bweights[2];
	uint8_t m_video_control;
	uint8_t m_video_flip;
	uint8_t m_vblank_latch;
	tilemap_t *m_spaceod_bg_htilemap;
	tilemap_t *m_spaceod_bg_vtilemap;
	uint16_t m_spaceod_hcounter;
	uint16_t m_spaceod_vcounter;
	uint8_t m_spaceod_fixed_color;
	uint8_t m_spaceod_bg_control;
	uint8_t m_spaceod_bg_detect;
	tilemap_t *m_bg_tilemap;
	uint8_t m_bg_enable;
	uint8_t m_bg_char_bank;
	uint16_t m_bg_scrollx;
	uint16_t m_bg_scrolly;
	uint8_t m_pignewt_bg_color_offset;

	DECLARE_READ8_MEMBER(g80r_opcode_r);
	DECLARE_WRITE8_MEMBER(mainram_w);
	DECLARE_WRITE8_MEMBER(vidram_w);
	DECLARE_WRITE8_MEMBER(monsterb_vidram_w);
	DECLARE_WRITE8_MEMBER(pignewt_vidram_w);
	DECLARE_WRITE8_MEMBER(sindbadm_vidram_w);
	DECLARE_READ8_MEMBER(mangled_ports_r);
	DECLARE_READ8_MEMBER(spaceod_mangled_ports_r);
	DECLARE_READ8_MEMBER(spaceod_port_fc_r);
	DECLARE_WRITE8_MEMBER(coin_count_w);
	DECLARE_WRITE8_MEMBER(segag80r_videoram_w);
	DECLARE_READ8_MEMBER(segag80r_video_port_r);
	DECLARE_WRITE8_MEMBER(segag80r_video_port_w);
	DECLARE_READ8_MEMBER(spaceod_back_port_r);
	DECLARE_WRITE8_MEMBER(spaceod_back_port_w);
	DECLARE_WRITE8_MEMBER(monsterb_videoram_w);
	DECLARE_WRITE8_MEMBER(monsterb_back_port_w);
	DECLARE_WRITE8_MEMBER(pignewt_videoram_w);
	DECLARE_WRITE8_MEMBER(pignewt_back_color_w);
	DECLARE_WRITE8_MEMBER(pignewt_back_port_w);
	DECLARE_WRITE8_MEMBER(sindbadm_videoram_w);
	DECLARE_WRITE8_MEMBER(sindbadm_back_port_w);
	DECLARE_WRITE8_MEMBER(astrob_sound_w);
	DECLARE_WRITE8_MEMBER(spaceod_sound_w);

	DECLARE_WRITE8_MEMBER(usb_ram_w);
	DECLARE_WRITE8_MEMBER(sindbadm_misc_w);
	DECLARE_WRITE8_MEMBER(sindbadm_sn1_SN76496_w);
	DECLARE_WRITE8_MEMBER(sindbadm_sn2_SN76496_w);

	TILE_GET_INFO_MEMBER(spaceod_get_tile_info);
	TILEMAP_MAPPER_MEMBER(spaceod_scan_rows);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_segag80r(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(segag80r_vblank_start);
	IRQ_CALLBACK_MEMBER(segag80r_irq_ack);
	INTERRUPT_GEN_MEMBER(sindbadm_vblank_start);
	DECLARE_WRITE8_MEMBER(sega005_sound_a_w);
	DECLARE_WRITE8_MEMBER(sega005_sound_b_w);

	void vblank_latch_set();
	void g80_set_palette_entry(int entry, uint8_t data);
	void spaceod_bg_init_palette();
	void draw_videoram(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *transparent_pens);
	void draw_background_spaceod(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background_page_scroll(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background_full_scroll(bitmap_ind16 &bitmap, const rectangle &cliprect);
	offs_t decrypt_offset(address_space &space, offs_t offset);
	inline uint8_t demangle(uint8_t d7d6, uint8_t d5d4, uint8_t d3d2, uint8_t d1d0);
	void monsterb_expand_gfx(const char *region);

	void g80r_opcodes_map(address_map &map);
	void main_map(address_map &map);
	void main_portmap(address_map &map);
	void main_ppi8255_portmap(address_map &map);
	void sega_315_opcodes_map(address_map &map);
	void sindbadm_portmap(address_map &map);
	void sindbadm_sound_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	emu_timer *m_vblank_latch_clear_timer;
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
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

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

#endif // MAME_INCLUDES_SEGAG80R_H
