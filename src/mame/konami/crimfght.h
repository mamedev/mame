// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Crime Fighters

*************************************************************************/
#ifndef MAME_INCLUDES_CRIMFGHT_H
#define MAME_INCLUDES_CRIMFGHT_H

#pragma once

#include "cpu/m6809/konami.h" /* for the callback and the firq irq definition */
#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "sound/k007232.h"
#include "k052109.h"
#include "k051960.h"
#include "emupal.h"

class crimfght_state : public driver_device
{
public:
	crimfght_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bank0000(*this, "bank0000"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_rombank(*this, "rombank")
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(system_r);
	void crimfght(machine_config &config);

private:
	/* devices */
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<address_map_bank_device> m_bank0000;
	required_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_memory_bank m_rombank;

	void crimfght_coin_w(uint8_t data);
	void sound_w(uint8_t data);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);
	IRQ_CALLBACK_MEMBER(audiocpu_irq_ack);
	void ym2151_ct_w(uint8_t data);
	virtual void machine_start() override;
	uint32_t screen_update_crimfght(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void volume_callback(uint8_t data);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	void banking_callback(uint8_t data);

	void bank0000_map(address_map &map);
	void crimfght_map(address_map &map);
	void crimfght_sound_map(address_map &map);
	int m_woco = 0;
	int m_rmrd = 0;
	int m_init = 0;
};

#endif // MAME_INCLUDES_CRIMFGHT_H
