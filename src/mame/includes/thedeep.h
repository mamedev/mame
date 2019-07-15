// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_THEDEEP_H
#define MAME_INCLUDES_THEDEEP_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "video/decmxc06.h"
#include "emupal.h"


class thedeep_state : public driver_device
{
public:
	thedeep_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_scroll(*this, "scroll"),
		m_scroll2(*this, "scroll2")
	{ }

	void thedeep(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8751_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_vram_0;
	required_shared_ptr<uint8_t> m_vram_1;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_scroll2;

	int m_nmi_enable;
	uint8_t m_protection_command;
	uint8_t m_protection_data;
	int m_protection_index;
	int m_protection_irq;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	uint8_t m_mcu_p3_reg;

	DECLARE_WRITE8_MEMBER(nmi_w);
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_READ8_MEMBER(e004_r);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(e100_w);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_READ8_MEMBER(from_main_r);
	DECLARE_WRITE8_MEMBER(to_main_w);
	DECLARE_WRITE8_MEMBER(p3_w);
	DECLARE_READ8_MEMBER(p0_r);
	DECLARE_WRITE8_MEMBER(vram_0_w);
	DECLARE_WRITE8_MEMBER(vram_1_w);

	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows_back);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);

	void thedeep_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(mcu_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void audio_map(address_map &map);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_THEDEEP_H
