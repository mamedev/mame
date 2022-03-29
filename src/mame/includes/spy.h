// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    S.P.Y.

*************************************************************************/
#ifndef MAME_INCLUDES_SPY_H
#define MAME_INCLUDES_SPY_H

#pragma once

#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/konami_helper.h"
#include "emupal.h"

class spy_state : public driver_device
{
public:
	spy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_palette(*this, "palette")
	{ }

	void spy(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_ram;
	uint8_t      m_pmcram[0x800]{};
	std::vector<uint8_t> m_paletteram{};

	/* misc */
	int        m_rambank = 0;
	int        m_pmcbank = 0;
	int        m_video_enable = 0;
	int        m_old_3f90 = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<palette_device> m_palette;
	uint8_t spy_bankedram1_r(offs_t offset);
	void spy_bankedram1_w(offs_t offset, uint8_t data);
	void bankswitch_w(uint8_t data);
	void spy_3f90_w(uint8_t data);
	void spy_sh_irqtrigger_w(uint8_t data);
	void sound_bank_w(uint8_t data);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_spy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void spy_collision(  );
	void volume_callback0(uint8_t data);
	void volume_callback1(uint8_t data);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);

	void spy_map(address_map &map);
	void spy_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SPY_H
