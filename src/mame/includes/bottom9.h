// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Bottom of the Ninth

*************************************************************************/
#ifndef MAME_INCLUDES_BOTTOM9_H
#define MAME_INCLUDES_BOTTOM9_H

#pragma once

#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/k051316.h"
#include "video/konami_helper.h"
#include "emupal.h"

class bottom9_state : public driver_device
{
public:
	bottom9_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k051316(*this, "k051316"),
		m_palette(*this, "palette")
	{ }

	void bottom9(machine_config &config);

private:
	/* misc */
	int        m_video_enable = 0;
	int        m_zoomreadroms = 0;
	int        m_k052109_selected = 0;
	int        m_nmienable = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<k051316_device> m_k051316;
	required_device<palette_device> m_palette;
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);
	uint8_t bottom9_bankedram1_r(offs_t offset);
	void bottom9_bankedram1_w(offs_t offset, uint8_t data);
	uint8_t bottom9_bankedram2_r(offs_t offset);
	void bottom9_bankedram2_w(offs_t offset, uint8_t data);
	void bankswitch_w(uint8_t data);
	void bottom9_1f90_w(uint8_t data);
	void bottom9_sh_irqtrigger_w(uint8_t data);
	void nmi_enable_w(uint8_t data);
	void sound_bank_w(uint8_t data);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_bottom9(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(bottom9_sound_interrupt);
	void volume_callback0(uint8_t data);
	void volume_callback1(uint8_t data);
	K051316_CB_MEMBER(zoom_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	void audio_map(address_map &map);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_BOTTOM9_H
