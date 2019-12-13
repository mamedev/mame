// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

    Super Dead Heat hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SPDHEAT_H
#define MAME_INCLUDES_SPDHEAT_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"
#include "machine/input_merger.h"
#include "sound/dac.h"


/*************************************
 *
 *  Machine class
 *
 *************************************/

class spdheat_state : public driver_device
{
public:
	spdheat_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_subcpu(*this, "subcpu"),
	m_audiocpu(*this, "audiocpu"),
	m_audio_irq(*this, "audio_irq"),
	m_fg_ram(*this, "fg_ram%u", 0U),
	m_spriteram(*this, "spriteram"),
	m_gfxdecode(*this, "gfxdecode"),
	m_palette0(*this, "palette0"),
	m_palette1(*this, "palette1"),
	m_palette2(*this, "palette2"),
	m_palette3(*this, "palette3"),
	m_dac(*this, "dac")
	{ }

	void spdheat(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<input_merger_any_high_device> m_audio_irq;
	required_shared_ptr_array<uint16_t, 4> m_fg_ram;
	required_shared_ptr<uint16_t> m_spriteram;
	tilemap_t *m_fg_tilemap[4];

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette0;
	required_device<palette_device> m_palette1;
	required_device<palette_device> m_palette2;
	required_device<palette_device> m_palette3;
	required_device<dac_byte_interface> m_dac;

	uint32_t m_sound_data[4];
	uint32_t m_sound_status;
	uint32_t m_sub_data;
	uint32_t m_sub_status;

	void main_map(address_map &map);
	void sub_map(address_map &map);
	void sub_io_map(address_map &map);
	void sound_map(address_map &map);

	DECLARE_READ8_MEMBER(sub_r);
	DECLARE_WRITE8_MEMBER(sub_dac_w);
	DECLARE_WRITE8_MEMBER(sub_nmi_w);
	DECLARE_WRITE8_MEMBER(sub_status_w);
	DECLARE_READ8_MEMBER(sub_snd_r);
	DECLARE_READ8_MEMBER(soundstatus_r);
	template<int screen> DECLARE_READ8_MEMBER(soundcpu_r);
	DECLARE_READ8_MEMBER(sub_status_r);
	DECLARE_READ16_MEMBER(sound_status_r);
	template<int screen> DECLARE_WRITE16_MEMBER(sound_w);
	template<int screen> DECLARE_READ8_MEMBER(sndcpu_sound_r);
	DECLARE_WRITE8_MEMBER(ym1_port_a_w);
	DECLARE_WRITE8_MEMBER(ym1_port_b_w);
	DECLARE_WRITE8_MEMBER(ym2_port_a_w);
	DECLARE_WRITE8_MEMBER(ym2_port_b_w);
	DECLARE_WRITE8_MEMBER(ym3_port_a_w);
	DECLARE_WRITE8_MEMBER(ym3_port_b_w);
	DECLARE_WRITE8_MEMBER(ym4_port_a_w);
	DECLARE_WRITE8_MEMBER(ym4_port_b_w);

	template<int screen> DECLARE_WRITE16_MEMBER(text_w);
	template<int screen> TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t xo, uint32_t yo);
	template<int which> uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#endif // MAME_INCLUDES_SPDHEAT_H
