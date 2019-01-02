// license:BSD-3-Clause
// copyright-holders:Farfetch'd, David Haywood
#ifndef MAME_INCLUDES_GSTRIKER_H
#define MAME_INCLUDES_GSTRIKER_H

#pragma once

#include "machine/6850acia.h"
#include "machine/gen_latch.h"
#include "machine/mb3773.h"
#include "video/vsystem_spr.h"
#include "video/mb60553.h"
#include "video/vs920a.h"
#include "emupal.h"
#include "screen.h"



class gstriker_state : public driver_device
{
public:
	gstriker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spr(*this, "vsystem_spr"),
		m_bg(*this, "zoomtilemap"),
		m_tx(*this, "texttilemap"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_watchdog(*this, "watchdog"),
		m_acia(*this, "acia"),
		m_CG10103_m_vram(*this, "cg10103_m_vram"),
		m_work_ram(*this, "work_ram"),
		m_mixerregs(*this, "mixerregs")
	{ }

	void base(machine_config &config);
	void twc94(machine_config &config);
	void gstriker(machine_config &config);
	void vgoal(machine_config &config);

	void init_vgoalsoc();
	void init_twcup94();
	void init_twcup94a();
	void init_twcup94b();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<vsystem_spr_device> m_spr;
	required_device<mb60553_zooming_tilemap_device> m_bg;
	required_device<vs920a_text_tilemap_device> m_tx;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<mb3773_device> m_watchdog;
	optional_device<acia6850_device> m_acia;

	required_shared_ptr<uint16_t> m_CG10103_m_vram;
	std::unique_ptr<uint16_t[]>    m_buffered_spriteram;
	std::unique_ptr<uint16_t[]>    m_buffered_spriteram2;
	required_shared_ptr<uint16_t> m_work_ram;
	required_shared_ptr<uint16_t> m_mixerregs;

	enum {
		TECMO_WCUP94_MCU = 1,
		TECMO_WCUP94A_MCU,
		TECMO_WCUP94B_MCU,
		VGOAL_SOCCER_MCU
	}m_mcutype;
	int m_gametype;
	uint16_t m_prot_reg[2];

	// common
	DECLARE_WRITE8_MEMBER(sh_bankswitch_w);

	// vgoalsoc and twrldc
	DECLARE_WRITE8_MEMBER(twcup94_prot_reg_w);

	// vgoalsoc only
	DECLARE_READ16_MEMBER(vbl_toggle_r);
	DECLARE_WRITE16_MEMBER(vbl_toggle_w);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	void mcu_init();
	void gstriker_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
	void twcup94_map(address_map &map);
};

#endif // MAME_INCLUDES_GSTRIKER_H
