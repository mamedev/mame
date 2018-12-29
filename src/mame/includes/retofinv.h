// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski, Andrea Mazzoleni
#ifndef MAME_INCLUDES_RETOFINV_H
#define MAME_INCLUDES_RETOFINV_H

#pragma once

#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/taito68705interface.h"
#include "emupal.h"

class retofinv_state : public driver_device
{
public:
	retofinv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "sub")
		, m_68705(*this, "68705")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_mainlatch(*this, "mainlatch")
		, m_soundlatch(*this, "soundlatch")
		, m_fg_videoram(*this, "fg_videoram")
		, m_sharedram(*this, "sharedram")
		, m_bg_videoram(*this, "bg_videoram")
	{
	}

	void retofinvb1_nomcu(machine_config &config);
	void retofinvb_nomcu(machine_config &config);
	void retofinv(machine_config &config);
	void retofinvb1(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	DECLARE_WRITE8_MEMBER(cpu2_m6000_w);
	DECLARE_READ8_MEMBER(cpu0_mf800_r);
	DECLARE_WRITE8_MEMBER(soundcommand_w);
	DECLARE_WRITE_LINE_MEMBER(irq0_ack_w);
	DECLARE_WRITE_LINE_MEMBER(irq1_ack_w);
	DECLARE_WRITE8_MEMBER(coincounter_w);
	DECLARE_WRITE_LINE_MEMBER(coinlockout_w);
	DECLARE_READ8_MEMBER(mcu_status_r);
	DECLARE_WRITE8_MEMBER(bg_videoram_w);
	DECLARE_WRITE8_MEMBER(fg_videoram_w);
	DECLARE_WRITE8_MEMBER(gfx_ctrl_w);

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);

	void retofinv_palette(palette_device &palette) const;
	void retofinv_bl_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(main_vblank_irq);
	INTERRUPT_GEN_MEMBER(sub_vblank_irq);

	void bootleg_map(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void sub_map(address_map &map);

	void draw_sprites(bitmap_ind16 &bitmap);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device<taito68705_mcu_device> m_68705;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_mainlatch;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_bg_videoram;

	uint8_t m_main_irq_mask;
	uint8_t m_sub_irq_mask;
	uint8_t m_cpu2_m6000;
	int m_fg_bank;
	int m_bg_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
};

#endif // MAME_INCLUDES_RETOFINV_H
