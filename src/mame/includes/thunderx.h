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

private:
	enum
	{
		TIMER_THUNDERX_FIRQ
	};

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
	int        m_priority;
	uint8_t      m_1f98_latch;
	emu_timer *m_thunderx_firq_timer;

	DECLARE_WRITE8_MEMBER(scontra_bankswitch_w);
	DECLARE_WRITE8_MEMBER(thunderx_videobank_w);
	DECLARE_WRITE8_MEMBER(gbusters_videobank_w);
	DECLARE_READ8_MEMBER(pmc_r);
	DECLARE_WRITE8_MEMBER(pmc_w);
	DECLARE_READ8_MEMBER(_1f98_r);
	DECLARE_WRITE8_MEMBER(scontra_1f98_w);
	DECLARE_WRITE8_MEMBER(thunderx_1f98_w);
	DECLARE_WRITE8_MEMBER(sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
	DECLARE_WRITE8_MEMBER(k007232_bankswitch_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void run_collisions( int s0, int e0, int s1, int e1, int cm, int hm );
	void calculate_collisions(  );
	DECLARE_WRITE8_MEMBER(volume_callback);
	K052109_CB_MEMBER(tile_callback);
	K052109_CB_MEMBER(gbusters_tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);

	void gbusters_map(address_map &map);
	void scontra_bank5800_map(address_map &map);
	void scontra_map(address_map &map);
	void scontra_sound_map(address_map &map);
	void thunderx_bank5800_map(address_map &map);
	void thunderx_map(address_map &map);
	void thunderx_sound_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_THUNDERX_H
