// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    GI Joe

*************************************************************************/
#ifndef MAME_INCLUDES_GIJOE_H
#define MAME_INCLUDES_GIJOE_H

#pragma once

#include "sound/k054539.h"
#include "video/k053251.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/konami_helper.h"
#include "machine/k054321.h"
#include "emupal.h"

class gijoe_state : public driver_device
{
public:
	gijoe_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k054539(*this, "k054539"),
		m_k056832(*this, "k056832"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_palette(*this, "palette"),
		m_k054321(*this, "k054321")
	{ }

	void gijoe(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_workram;

	/* video-related */
	int         m_avac_bits[4];
	int         m_avac_occupancy[4];
	int         m_layer_colorbase[4];
	int         m_layer_pri[4];
	int         m_avac_vrc;
	int         m_sprite_colorbase;

	/* misc */
	uint16_t      m_cur_control2;
	emu_timer   *m_dmadelay_timer;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device<k056832_device> m_k056832;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<palette_device> m_palette;
	required_device<k054321_device> m_k054321;

	DECLARE_READ16_MEMBER(control2_r);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_gijoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gijoe_interrupt);
	TIMER_CALLBACK_MEMBER(dmaend_callback);
	void gijoe_objdma();
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
	void gijoe_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_GIJOE_H
