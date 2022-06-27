// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Vapor Trail

*************************************************************************/
#ifndef MAME_INCLUDES_VAPORTRA_H
#define MAME_INCLUDES_VAPORTRA_H

#pragma once

#include "cpu/h6280/h6280.h"
#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "deco16ic.h"
#include "decmxc06.h"
#include "emupal.h"

class vaportra_state : public driver_device
{
public:
	vaportra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco_tilegen(*this, "tilegen%u", 1U)
		, m_spritegen(*this, "spritegen")
		, m_spriteram(*this, "spriteram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "colors")
		, m_soundlatch(*this, "soundlatch")
		, m_paletteram(*this, "palette")
		, m_paletteram_ext(*this, "palette_ext")
	{ }

	void vaportra(machine_config &config);

	void init_vaportra();

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_paletteram_ext;

	/* misc */
	uint16_t    m_priority[2]{};

	uint8_t irq6_ack_r();
	void irq6_ack_w(uint8_t data);
	void priority_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void palette_ext_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void vaportra_colpri_cb(u32 &colour, u32 &pri_mask);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_palette( int offset );

	DECO16IC_BANK_CB_MEMBER(bank_callback);

	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_VAPORTRA_H
