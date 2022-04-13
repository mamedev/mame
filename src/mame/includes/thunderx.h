// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Manuel Abadia
/*************************************************************************

    Super Contra / Thunder Cross

*************************************************************************/
#ifndef MAME_INCLUDES_THUNDERX_H
#define MAME_INCLUDES_THUNDERX_H

#pragma once

#include "cpu/m6809/konami.h" // for the callback and the firq irq definition
#include "machine/bankdev.h"
#include "sound/k007232.h"
#include "video/k051960.h"
#include "video/k052109.h"
#include "video/konami_helper.h"
#include "emupal.h"

class thunderx_state : public driver_device
{
public:
	thunderx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bank5800(*this, "bank5800"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_palette(*this, "palette"),
		m_rombank(*this, "rombank"),
		m_pmcram(*this, "pmcram")
	{ }

	void scontra(machine_config &config);
	void gbusters(machine_config &config);
	void thunderx(machine_config &config);

	void init_thunderx();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* devices */
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<address_map_bank_device> m_bank5800;
	optional_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<palette_device> m_palette;

	/* memory */
	required_memory_bank m_rombank;
	optional_shared_ptr<uint8_t> m_pmcram;

	/* misc */
	int        m_priority = 0;
	uint8_t      m_1f98_latch = 0;
	emu_timer *m_thunderx_firq_timer = nullptr;

	void scontra_bankswitch_w(uint8_t data);
	void thunderx_videobank_w(uint8_t data);
	void gbusters_videobank_w(uint8_t data);
	uint8_t pmc_r(offs_t offset);
	void pmc_w(offs_t offset, uint8_t data);
	uint8_t _1f98_r();
	void scontra_1f98_w(uint8_t data);
	void thunderx_1f98_w(uint8_t data);
	void sh_irqtrigger_w(uint8_t data);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);
	void k007232_bankswitch_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(thunderx_firq_cb);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void run_collisions( int s0, int e0, int s1, int e1, int cm, int hm );
	void calculate_collisions(  );
	void volume_callback(uint8_t data);
	K052109_CB_MEMBER(tile_callback);
	K052109_CB_MEMBER(gbusters_tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	void banking_callback(uint8_t data);

	void gbusters_map(address_map &map);
	void scontra_bank5800_map(address_map &map);
	void scontra_map(address_map &map);
	void scontra_sound_map(address_map &map);
	void thunderx_bank5800_map(address_map &map);
	void thunderx_map(address_map &map);
	void thunderx_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_THUNDERX_H
