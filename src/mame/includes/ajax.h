// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#ifndef MAME_INCLUDES_AJAX_H
#define MAME_INCLUDES_AJAX_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/k051316.h"
#include "video/konami_helper.h"
#include "emupal.h"

class ajax_state : public driver_device
{
public:
	ajax_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_watchdog(*this, "watchdog"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k051316(*this, "k051316"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void sound_bank_w(uint8_t data);
	uint8_t ls138_f10_r(offs_t offset);
	void ls138_f10_w(offs_t offset, uint8_t data);
	void bankswitch_2_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	void lamps_w(uint8_t data);
	void k007232_extvol_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void volume_callback0(uint8_t data);
	void volume_callback1(uint8_t data);
	K051316_CB_MEMBER(zoom_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	void ajax(machine_config &config);
	void ajax_main_map(address_map &map);
	void ajax_sound_map(address_map &map);
	void ajax_sub_map(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	/* video-related */
	uint8_t      m_priority = 0U;

	/* misc */
	int        m_firq_enable = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<k051316_device> m_k051316;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	output_finder<8> m_lamps;
};

#endif // MAME_INCLUDES_AJAX_H
