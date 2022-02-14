// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_INCLUDES_SIMPSONS_H
#define MAME_INCLUDES_SIMPSONS_H

#pragma once

#include "cpu/m6809/konami.h" // for the callback and the firq irq definition
#include "video/k052109.h"
#include "video/k053251.h"
#include "video/k053246_k053247_k055673.h"
#include "video/konami_helper.h"

class simpsons_state : public driver_device
{
public:
	simpsons_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_palette_view(*this, "palette_view"),
		m_video_view(*this, "video_view"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251")
	{ }

	void simpsons(machine_config &config);

private:
	enum
	{
		TIMER_DMASTART,
		TIMER_DMAEND
	};

	/* memory pointers */
	std::unique_ptr<uint16_t[]>   m_spriteram;

	/* video-related */
	int        m_sprite_colorbase;
	int        m_layer_colorbase[3];
	int        m_layerpri[3];

	/* misc */
	int        m_firq_enabled;
	u64        m_nmi_enabled;

	/* views */
	memory_view m_palette_view;
	memory_view m_video_view;

	/* devices */
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	void z80_bankswitch_w(uint8_t data);
	void z80_arm_nmi_w(uint8_t data);
	void simpsons_eeprom_w(uint8_t data);
	void simpsons_coin_counter_w(uint8_t data);
	uint8_t simpsons_sound_interrupt_r();
	uint8_t simpsons_k052109_r(offs_t offset);
	void simpsons_k052109_w(offs_t offset, uint8_t data);
	uint8_t simpsons_k053247_r(offs_t offset);
	void simpsons_k053247_w(offs_t offset, uint8_t data);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_simpsons(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(simpsons_irq);
	void simpsons_video_banking(int bank);
	void simpsons_objdma();
	void z80_nmi_w(int state);
	K052109_CB_MEMBER(tile_callback);
	void banking_callback(u8 data);
	K053246_CB_MEMBER(sprite_callback);

	void bank0000_map(address_map &map);
	void bank2000_map(address_map &map);
	void main_map(address_map &map);
	void z80_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
};

#endif // MAME_INCLUDES_SIMPSONS_H
