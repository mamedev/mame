// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Data East 'Rohga' era hardware

*************************************************************************/
#ifndef MAME_DATAEAST_ROHGA_H
#define MAME_DATAEAST_ROHGA_H

#pragma once

#include "deco104.h"
#include "deco146.h"
#include "deco16ic.h"
#include "decocomn.h"
#include "decospr.h"

#include "cpu/h6280/h6280.h"
#include "sound/okim6295.h"
#include "video/bufsprite.h"

#include "emupal.h"


class rohga_state : public driver_device
{
public:
	rohga_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ioprot(*this, "ioprot"),
		m_decocomn(*this, "deco_common"),
		m_deco_tilegen(*this, "tilegen%u", 1),
		m_oki(*this, "oki%u", 1),
		m_spriteram(*this, "spriteram%u", 1),
		m_pf_rowscroll(*this, "pf%u_rowscroll", 1),
		m_sprgen(*this, "spritegen%u", 1),
		m_palette(*this, "palette")
	{ }

	void wizdfire(machine_config &config);
	void nitrobal(machine_config &config);
	void hangzo(machine_config &config);
	void schmeisr(machine_config &config);
	void rohga(machine_config &config);

	void init_wizdfire();
	void init_nitrobal();
	void init_schmeisr();
	void init_hangzo();
	void init_rohga();

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<deco_146_base_device> m_ioprot;
	required_device<decocomn_device> m_decocomn;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device_array<okim6295_device, 2> m_oki;
	optional_device_array<buffered_spriteram16_device, 2> m_spriteram;

	/* memory pointers */
	optional_shared_ptr_array<u16, 4> m_pf_rowscroll;

	optional_device_array<decospr_device, 2> m_sprgen;

	required_device<palette_device> m_palette;

	u16 irq_ack_r();
	void irq_ack_w(u16 data);
	void rohga_buffer_spriteram16_w(u16 data);
	void sound_bankswitch_w(u8 data);

	DECLARE_VIDEO_START(wizdfire);
	u32 screen_update_rohga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_wizdfire(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_nitrobal(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mixwizdfirelayer(bitmap_rgb32 &bitmap, const rectangle &cliprect, u16 pri, u16 primask);
	void mixnitroballlayer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(rohga_pri_callback);
	DECOSPR_COLOUR_CB_MEMBER(rohga_col_callback);
	DECOSPR_COLOUR_CB_MEMBER(schmeisr_col_callback);

	u16 ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void rohga_base(machine_config &config);

	void rohga_map(address_map &map) ATTR_COLD;
	void wizdfire_map(address_map &map) ATTR_COLD;
	void nitrobal_map(address_map &map) ATTR_COLD;
	void hotb_base_map(address_map &map) ATTR_COLD;
	void schmeisr_map(address_map &map) ATTR_COLD;
	void hangzo_map(address_map &map) ATTR_COLD;

	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_DATAEAST_ROHGA_H
