// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Crude Buster

*************************************************************************/
#ifndef MAME_INCLUDES_CBUSTER_H
#define MAME_INCLUDES_CBUSTER_H

#pragma once

#include "machine/gen_latch.h"
#include "cpu/h6280/h6280.h"
#include "video/bufsprite.h"
#include "decospr.h"
#include "deco16ic.h"
#include "emupal.h"

class cbuster_state : public driver_device
{
public:
	cbuster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco_tilegen(*this, "tilegen%u", 1U)
		, m_palette(*this, "palette")
		, m_spriteram(*this, "spriteram")
		, m_soundlatch(*this, "soundlatch")
		, m_sprgen(*this, "spritegen")
		, m_pf_rowscroll(*this, "pf%u_rowscroll", 1U)
	{ }

	void init_twocrude();

	void twocrude(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<decospr_device> m_sprgen;

	/* memory pointers */
	required_shared_ptr_array<u16, 4> m_pf_rowscroll;

	/* misc */
	u16    m_prot = 0U;
	int    m_pri = 0;

	void prot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 prot_r();
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	static rgb_t cbuster_XBGR_888(u32 raw);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_CBUSTER_H
