// license:BSD-3-Clause
// copyright-holders:R. Belmont, Acho A. Tang
/*************************************************************************

    Wild West C.O.W.boys of Moo Mesa / Bucky O'Hare

*************************************************************************/
#ifndef MAME_INCLUDES_MOO_H
#define MAME_INCLUDES_MOO_H

#pragma once

#include "sound/okim6295.h"
#include "sound/k054539.h"
#include "machine/k053252.h"
#include "video/k053251.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k054000.h"
#include "video/k054338.h"
#include "machine/k054321.h"
#include "video/konami_helper.h"
#include "emupal.h"
#include "screen.h"

class moo_state : public driver_device
{
public:
	moo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_oki(*this, "oki"),
		m_k054539(*this, "k054539"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_k056832(*this, "k056832"),
		m_k054338(*this, "k054338"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_k054321(*this, "k054321")
	{ }

	void bucky(machine_config &config);
	void moo(machine_config &config);
	void moobl(machine_config &config);

private:
	/* memory pointers */
	optional_shared_ptr<uint16_t> m_workram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	int         m_sprite_colorbase = 0;
	int         m_layer_colorbase[4];
	int         m_layerpri[3];
	int         m_alpha_enabled = 0;
	uint16_t      m_zmask = 0;

	/* misc */
	uint16_t      m_protram[16];
	uint16_t      m_cur_control2 = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<okim6295_device> m_oki;
	optional_device<k054539_device> m_k054539;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	optional_device<k053252_device> m_k053252;
	required_device<k056832_device> m_k056832;
	required_device<k054338_device> m_k054338;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	optional_device<k054321_device> m_k054321;

	emu_timer *m_dmaend_timer = nullptr;
	uint16_t control2_r();
	void control2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_irq_w(uint16_t data);
	void sound_bankswitch_w(uint8_t data);
	void moo_prot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void moobl_oki_bank_w(uint16_t data);
	DECLARE_MACHINE_START(moo);
	DECLARE_MACHINE_RESET(moo);
	DECLARE_VIDEO_START(moo);
	DECLARE_VIDEO_START(bucky);
	uint32_t screen_update_moo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(moo_interrupt);
	INTERRUPT_GEN_MEMBER(moobl_interrupt);
	TIMER_CALLBACK_MEMBER(dmaend_callback);
	void moo_objdma();
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
	void bucky_map(address_map &map);
	void moo_map(address_map &map);
	void moobl_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_MOO_H
