// license:BSD-3-Clause
// copyright-holders:BUT
#ifndef MAME_INCLUDES_CHAKNPOP_H
#define MAME_INCLUDES_CHAKNPOP_H

#pragma once

#include "machine/taito68705interface.h"
#include "emupal.h"
#include "tilemap.h"

class chaknpop_state : public driver_device
{
public:
	chaknpop_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bmcu(*this, "bmcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_tx_ram(*this, "tx_ram"),
		m_attr_ram(*this, "attr_ram"),
		m_spr_ram(*this, "spr_ram"),
		m_vram_bank(*this, "vram_bank"),
		m_vram(*this, "vram", 0x8000, ENDIANNESS_LITTLE)
	{ }

	void chaknpop(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_tx_ram;
	required_shared_ptr<uint8_t> m_attr_ram;
	required_shared_ptr<uint8_t> m_spr_ram;
	required_memory_bank m_vram_bank;
	memory_share_creator<uint8_t> m_vram;

	/* video-related */
	tilemap_t  *m_tx_tilemap = nullptr;
	uint8_t    m_gfxmode = 0U;
	uint8_t    m_flip_x = 0U;
	uint8_t    m_flip_y = 0U;

	void coinlock_w(uint8_t data);
	uint8_t gfxmode_r();
	void gfxmode_w(uint8_t data);
	void txram_w(offs_t offset, uint8_t data);
	void attrram_w(offs_t offset, uint8_t data);
	void unknown_port_1_w(uint8_t data);
	void unknown_port_2_w(uint8_t data);
	void unknown_port_3_w(uint8_t data);
	uint8_t mcu_status_r();
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void chaknpop_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tx_tilemap_mark_all_dirty();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void chaknpop_map(address_map &map);
};

#endif // MAME_INCLUDES_CHAKNPOP_H
