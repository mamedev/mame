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
	tilemap_t *m_fg_tilemap[4]{};

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette0;
	required_device<palette_device> m_palette1;
	required_device<palette_device> m_palette2;
	required_device<palette_device> m_palette3;
	required_device<dac_byte_interface> m_dac;

	uint32_t m_sound_data[4]{};
	uint32_t m_sound_status = 0;
	uint32_t m_sub_data = 0;
	uint32_t m_sub_status = 0;

	void main_map(address_map &map);
	void sub_map(address_map &map);
	void sub_io_map(address_map &map);
	void sound_map(address_map &map);

	uint8_t sub_r();
	void sub_dac_w(uint8_t data);
	void sub_nmi_w(uint8_t data);
	void sub_status_w(uint8_t data);
	uint8_t sub_snd_r();
	uint8_t soundstatus_r();
	uint8_t sub_status_r();
	uint16_t sound_status_r();
	template<int screen> void sound_w(uint16_t data);
	template<int screen> uint8_t sndcpu_sound_r();
	void ym1_port_a_w(uint8_t data);
	void ym1_port_b_w(uint8_t data);
	void ym2_port_a_w(uint8_t data);
	void ym2_port_b_w(uint8_t data);
	void ym3_port_a_w(uint8_t data);
	void ym3_port_b_w(uint8_t data);
	void ym4_port_a_w(uint8_t data);
	void ym4_port_b_w(uint8_t data);

	template<int screen> void text_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int screen> TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t xo, uint32_t yo);
	template<int which> uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#endif // MAME_INCLUDES_SPDHEAT_H
