// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Bennett
/*************************************************************************

    Kyuukoukabakugekitai - Dive Bomber Squad

*************************************************************************/
#ifndef MAME_INCLUDES_DIVEBOMB_H
#define MAME_INCLUDES_DIVEBOMB_H

#pragma once


#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/sn76496.h"
#include "video/k051316.h"
#include "emupal.h"

#define XTAL1 XTAL(24'000'000)

class divebomb_state : public driver_device
{
public:
	divebomb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spritecpu(*this, "spritecpu")
		, m_fgcpu(*this, "fgcpu")
		, m_rozcpu(*this, "rozcpu")
		, m_rozbank(*this, "rozbank")
		, m_fgram(*this, "fgram")
		, m_spriteram(*this, "spriteram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_k051316(*this, "k051316_%u", 1)
		, m_fgcpu_irq(*this, "fgcpu_irq")
		, m_spr2fg_latch(*this, "spr2fg")
		, m_roz2fg_latch(*this, "roz2fg")
	{
	}

	void divebomb(machine_config &config);

private:
	required_device<cpu_device> m_spritecpu;
	required_device<cpu_device> m_fgcpu;
	required_device<cpu_device> m_rozcpu;
	required_memory_bank m_rozbank;
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<k051316_device, 2> m_k051316;
	required_device<input_merger_any_high_device> m_fgcpu_irq;
	required_device<generic_latch_8_device> m_spr2fg_latch;
	required_device<generic_latch_8_device> m_roz2fg_latch;

	tilemap_t *m_fg_tilemap;

	uint8_t m_roz_pal;
	bool m_roz_enable[2];

	DECLARE_MACHINE_RESET(divebomb);
	DECLARE_MACHINE_START(divebomb);
	DECLARE_VIDEO_START(divebomb);
	void divebomb_palette(palette_device &palette) const;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	static void decode_proms(palette_device &palette, const uint8_t* rgn, int size, int index, bool inv);
	uint32_t screen_update_divebomb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	K051316_CB_MEMBER(zoom_callback_1);
	K051316_CB_MEMBER(zoom_callback_2);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	DECLARE_READ8_MEMBER(fgcpu_comm_flags_r);
	DECLARE_WRITE8_MEMBER(fgram_w);

	DECLARE_WRITE8_MEMBER(spritecpu_port00_w);

	DECLARE_WRITE8_MEMBER(rozcpu_bank_w);
	template<int Chip> DECLARE_WRITE8_MEMBER(rozcpu_wrap_enable_w);
	template<int Chip> DECLARE_WRITE8_MEMBER(rozcpu_enable_w);
	DECLARE_WRITE8_MEMBER(rozcpu_pal_w);
	void divebomb_fgcpu_iomap(address_map &map);
	void divebomb_fgcpu_map(address_map &map);
	void divebomb_rozcpu_iomap(address_map &map);
	void divebomb_rozcpu_map(address_map &map);
	void divebomb_spritecpu_iomap(address_map &map);
	void divebomb_spritecpu_map(address_map &map);
};

#endif // MAME_INCLUDES_DIVEBOMB_H
