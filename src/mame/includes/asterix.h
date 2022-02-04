// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    Asterix

*************************************************************************/
#ifndef MAME_INCLUDES_ASTERIX_H
#define MAME_INCLUDES_ASTERIX_H

#pragma once

#include "video/k053251.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053244_k053245.h"
#include "video/konami_helper.h"

class asterix_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI
	};

	asterix_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k056832(*this, "k056832"),
		m_k053244(*this, "k053244"),
		m_k053251(*this, "k053251")
	{ }

	/* video-related */
	int         m_sprite_colorbase;
	int         m_layer_colorbase[4];
	int         m_layerpri[3];
	uint16_t      m_spritebank;
	int         m_tilebanks[4];
	int         m_spritebanks[4];

	/* misc */
	uint8_t       m_cur_control2;
	uint16_t      m_prot[2];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k056832_device> m_k056832;
	required_device<k05324x_device> m_k053244;
	required_device<k053251_device> m_k053251;
	void control2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_arm_nmi_w(uint8_t data);
	void sound_irq_w(uint16_t data);
	void protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void asterix_spritebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void init_asterix();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_asterix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(asterix_interrupt);
	K05324X_CB_MEMBER(sprite_callback);
	K056832_CB_MEMBER(tile_callback);
	void reset_spritebank();

	void asterix(machine_config &config);
	void main_map(address_map &map);
	void sound_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
};

#endif // MAME_INCLUDES_ASTERIX_H
