// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    88 Games

*************************************************************************/
#ifndef MAME_INCLUDES_88GAMES_H
#define MAME_INCLUDES_88GAMES_H

#pragma once

#include "cpu/m6809/konami.h"
#include "sound/upd7759.h"
#include "video/k051316.h"
#include "video/k051960.h"
#include "video/k052109.h"
#include "video/konami_helper.h"

class _88games_state : public driver_device
{
public:
	_88games_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k051316(*this, "k051316"),
		m_upd7759(*this, "upd%d", 1),
		m_bank0000(*this, "bank0000"),
		m_bank1000(*this, "bank1000"),
		m_ram(*this, "ram")
	{ }

	void _88games(machine_config &config);

private:
	/* video-related */
	int          m_k88games_priority = 0;
	int          m_videobank = 0;
	int          m_zoomreadroms = 0;
	int          m_speech_chip = 0;

	/* devices */
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<k051316_device> m_k051316;
	required_device_array<upd7759_device, 2> m_upd7759;

	/* memory banks */
	required_memory_bank m_bank0000;
	required_memory_bank m_bank1000;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_ram;

	uint8_t bankedram_r(offs_t offset);
	void bankedram_w(offs_t offset, uint8_t data);
	void k88games_5f84_w(uint8_t data);
	void k88games_sh_irqtrigger_w(uint8_t data);
	void speech_control_w(uint8_t data);
	void speech_msg_w(uint8_t data);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_88games(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K051316_CB_MEMBER(zoom_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	void banking_callback(uint8_t data);

	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_88GAMES_H
