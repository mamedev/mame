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
	asterix_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k056832(*this, "k056832"),
		m_k053244(*this, "k053244"),
		m_k053251(*this, "k053251")
	{ }

	void asterix(machine_config &config);
	void init_asterix();

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void main_map(address_map &map);
	void sound_map(address_map &map);

	void control2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_arm_nmi_w(uint8_t data);
	void sound_irq_w(uint16_t data);
	void protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void asterix_spritebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t screen_update_asterix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(asterix_interrupt);
	K05324X_CB_MEMBER(sprite_callback);
	K056832_CB_MEMBER(tile_callback);
	void reset_spritebank();
	TIMER_CALLBACK_MEMBER(audio_nmi);

	/* video-related */
	int         m_sprite_colorbase = 0;
	int         m_layer_colorbase[4]{};
	int         m_layerpri[3]{};
	uint16_t    m_spritebank = 0U;
	int         m_tilebanks[4]{};
	int         m_spritebanks[4]{};

	/* misc */
	uint8_t     m_cur_control2 = 0U;
	uint16_t    m_prot[2]{};
	emu_timer  *m_audio_nmi_timer = nullptr;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k056832_device> m_k056832;
	required_device<k05324x_device> m_k053244;
	required_device<k053251_device> m_k053251;
};

#endif // MAME_INCLUDES_ASTERIX_H
