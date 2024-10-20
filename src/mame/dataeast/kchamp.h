// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Karate Champ

*************************************************************************/
#ifndef MAME_DATAEAST_KCHAMP_H
#define MAME_DATAEAST_KCHAMP_H

#pragma once

#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "sound/dac.h"
#include "emupal.h"
#include "tilemap.h"

class kchamp_state : public driver_device
{
public:
	kchamp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ay(*this, "ay%u", 1),
		m_adpcm_select(*this, "adpcm_select"),
		m_msm(*this, "msm"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void kchamp(machine_config &config);
	void kchampvs(machine_config &config);
	void kchamp_arfyc(machine_config &config);

	void init_kchampvs();
	void init_kchampvs2();

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	/* video-related */
	tilemap_t    *m_bg_tilemap = nullptr;

	/* misc */
	bool       m_nmi_enable = false;
	bool       m_sound_nmi_enable = false;
	bool       m_msm_play_lo_nibble = false;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<ay8910_device, 2> m_ay;
	optional_device<ls157_device> m_adpcm_select;
	optional_device<msm5205_device> m_msm;
	optional_device<dac08_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void nmi_enable_w(int state);
	void sound_reset_w(int state);
	uint8_t sound_reset_r();
	void kc_sound_control_w(offs_t offset, uint8_t data);
	void kchamp_videoram_w(offs_t offset, uint8_t data);
	void kchamp_colorram_w(offs_t offset, uint8_t data);
	void sound_control_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void kchamp_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(kchampvs);
	DECLARE_MACHINE_START(kchamp);
	uint32_t screen_update_kchampvs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kchamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	INTERRUPT_GEN_MEMBER(sound_int);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int dx, int dy);
	void decrypt_code();
	void msmint(int state);
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void kchamp_io_map(address_map &map) ATTR_COLD;
	void kchamp_map(address_map &map) ATTR_COLD;
	void kchamp_sound_io_map(address_map &map) ATTR_COLD;
	void kchamp_sound_map(address_map &map) ATTR_COLD;
	void kchampvs_io_map(address_map &map) ATTR_COLD;
	void kchampvs_map(address_map &map) ATTR_COLD;
	void kchampvs_sound_io_map(address_map &map) ATTR_COLD;
	void kchampvs_sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_DATAEAST_KCHAMP_H
