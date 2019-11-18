// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Vendetta

*************************************************************************/
#ifndef MAME_INCLUDES_VENDETTA_H
#define MAME_INCLUDES_VENDETTA_H

#pragma once

#include "cpu/m6809/konami.h" // for the callback and the firq irq definition
#include "machine/bankdev.h"
#include "machine/k053252.h"
#include "video/k052109.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053251.h"
#include "video/k054000.h"
#include "video/konami_helper.h"
#include "emupal.h"

class vendetta_state : public driver_device
{
public:
	vendetta_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_k054000(*this, "k054000"),
		m_palette(*this, "palette"),
		m_videobank0(*this, "videobank0"),
		m_videobank1(*this, "videobank1")
	{ }

	void esckids(machine_config &config);
	void vendetta(machine_config &config);

private:
	enum
	{
		TIMER_Z80_NMI
	};

	// video-related
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	// misc
	int        m_irq_enabled;

	// devices
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	optional_device<k053252_device> m_k053252;
	optional_device<k054000_device> m_k054000;
	required_device<palette_device> m_palette;

	required_device<address_map_bank_device> m_videobank0;
	required_device<address_map_bank_device> m_videobank1;

	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_READ8_MEMBER(K052109_r);
	DECLARE_WRITE8_MEMBER(K052109_w);
	DECLARE_WRITE8_MEMBER(_5fe0_w);
	DECLARE_WRITE8_MEMBER(z80_arm_nmi_w);
	DECLARE_WRITE8_MEMBER(z80_irq_w);
	DECLARE_READ8_MEMBER(z80_irq_r);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(irq);

	K052109_CB_MEMBER(vendetta_tile_callback);
	K052109_CB_MEMBER(esckids_tile_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);
	K053246_CB_MEMBER(sprite_callback);

	void esckids_map(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void videobank0_map(address_map &map);
	void videobank1_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_VENDETTA_H
