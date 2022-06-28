// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    The Main Event / Devastators

*************************************************************************/
#ifndef MAME_INCLUDES_MAINEVT_H
#define MAME_INCLUDES_MAINEVT_H

#pragma once

#include "sound/upd7759.h"
#include "sound/k007232.h"
#include "k052109.h"
#include "k051960.h"
#include "k051733.h"
#include "konami_helper.h"

class mainevt_state : public driver_device
{
public:
	mainevt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_upd7759(*this, "upd")
		, m_k007232(*this, "k007232")
		, m_k052109(*this, "k052109")
		, m_k051960(*this, "k051960")
		, m_rombank(*this, "rombank")
		, m_leds(*this, "led%u", 0U)
	{ }

	void devstors(machine_config &config);
	void mainevt(machine_config &config);

private:
	void dv_nmienable_w(uint8_t data);
	void mainevt_bankswitch_w(uint8_t data);
	void mainevt_coin_w(uint8_t data);
	void mainevt_sh_irqtrigger_w(uint8_t data);
	void mainevt_sh_irqcontrol_w(uint8_t data);
	void devstor_sh_irqcontrol_w(uint8_t data);
	void mainevt_sh_bankswitch_w(uint8_t data);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);
	uint8_t mainevt_sh_busy_r();
	void dv_sh_bankswitch_w(uint8_t data);
	uint32_t screen_update_mainevt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(dv_vblank_w);
	INTERRUPT_GEN_MEMBER(mainevt_sound_timer_irq);
	INTERRUPT_GEN_MEMBER(devstors_sound_timer_irq);
	void volume_callback(uint8_t data);
	K052109_CB_MEMBER(mainevt_tile_callback);
	K052109_CB_MEMBER(dv_tile_callback);
	K051960_CB_MEMBER(mainevt_sprite_callback);
	K051960_CB_MEMBER(dv_sprite_callback);
	void devstors_map(address_map &map);
	void devstors_sound_map(address_map &map);
	void mainevt_map(address_map &map);
	void mainevt_sound_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	/* misc */
	int        m_nmi_enable = 0;
	uint8_t      m_sound_irq_mask = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<upd7759_device> m_upd7759;
	required_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;

	required_memory_bank m_rombank;
	output_finder<4> m_leds;
};

#endif // MAME_INCLUDES_MAINEVT_H
