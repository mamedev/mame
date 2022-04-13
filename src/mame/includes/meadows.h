// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/*************************************************************************

    Meadows S2650 hardware

*************************************************************************/
#ifndef MAME_INCLUDES_MEADOWS_H
#define MAME_INCLUDES_MEADOWS_H

#pragma once

#include "cpu/s2650/s2650.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class meadows_state : public driver_device
{
public:
	meadows_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dac(*this, "dac"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram")
	{
	}

	void bowl3d(machine_config &config);
	void meadows(machine_config &config);
	void minferno(machine_config &config);

	void init_minferno();
	void init_gypsyjug();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override { m_main_sense_state = 0; }
	virtual void video_start() override;

private:
	required_device<s2650_device> m_maincpu;
	optional_device<s2650_device> m_audiocpu;
	optional_device<dac_8bit_r2r_device> m_dac;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	int m_channel = 0;
	int m_freq1 = 0;
	int m_freq2 = 0;
	uint8_t m_latched_0c01 = 0;
	uint8_t m_latched_0c02 = 0;
	uint8_t m_latched_0c03 = 0;
	uint8_t m_main_sense_state = 0;
	uint8_t m_audio_sense_state = 0;
	uint8_t m_0c00 = 0;
	uint8_t m_0c01 = 0;
	uint8_t m_0c02 = 0;
	uint8_t m_0c03 = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t hsync_chain_r();
	uint8_t vsync_chain_hi_r();
	uint8_t vsync_chain_lo_r();
	void meadows_audio_w(offs_t offset, uint8_t data);
	void audio_hardware_w(offs_t offset, uint8_t data);
	uint8_t audio_hardware_r(offs_t offset);
	void meadows_videoram_w(offs_t offset, uint8_t data);
	void meadows_spriteram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update_meadows(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(meadows_vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(minferno_vblank_irq);
	INTERRUPT_GEN_MEMBER(audio_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &clip);
	void meadows_sh_update();
	SAMPLES_START_CB_MEMBER(meadows_sh_start);
	void audio_map(address_map &map);
	void bowl3d_main_map(address_map &map);
	void meadows_main_map(address_map &map);
	void minferno_data_map(address_map &map);
	void minferno_main_map(address_map &map);
};

#endif // MAME_INCLUDES_MEADOWS_H
