// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_INCLUDES_DARKSEAL_H
#define MAME_INCLUDES_DARKSEAL_H

#pragma once

#include "machine/gen_latch.h"
#include "cpu/h6280/h6280.h"
#include "deco16ic.h"
#include "video/bufsprite.h"
#include "decospr.h"
#include "emupal.h"

class darkseal_state : public driver_device
{
public:
	darkseal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_palette(*this, "colors")
		, m_deco_tilegen(*this, "tilegen%u", 1U)
		, m_sprgen(*this, "spritegen")
		, m_spriteram(*this, "spriteram")
		, m_soundlatch(*this, "soundlatch")
		, m_pf1_rowscroll(*this, "pf1_rowscroll")
		, m_pf3_rowscroll(*this, "pf3_rowscroll")
		, m_paletteram(*this, "palette")
		, m_paletteram_ext(*this, "palette_ext")
	{ }

	void darkseal(machine_config &config);

	void init_darkseal();

private:
	void irq_ack_w(uint16_t data);
	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void palette_ext_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_palette(int offset);
	void darkseal_map(address_map &map);
	void sound_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<palette_device> m_palette;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<decospr_device> m_sprgen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_pf1_rowscroll;
	//uint16_t *m_pf2_rowscroll;
	required_shared_ptr<uint16_t> m_pf3_rowscroll;
	//uint16_t *m_pf4_rowscroll;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_paletteram_ext;
};

#endif // MAME_INCLUDES_DARKSEAL_H
