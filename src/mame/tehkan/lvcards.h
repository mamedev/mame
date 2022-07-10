// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Curt Coder
#ifndef MAME_INCLUDES_LVCARDS_H
#define MAME_INCLUDES_LVCARDS_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class lvcards_state : public driver_device
{
public:
	lvcards_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void lvcards(machine_config &config);
	void lvcardsa(machine_config &config);

	void init_lvcardsa();

protected:
	virtual void video_start() override;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);

	required_device<cpu_device> m_maincpu;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	tilemap_t *m_bg_tilemap = nullptr;
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void lvcards_palette(palette_device &palette) const;
	uint32_t screen_update_lvcards(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void lvcards_io_map(address_map &map);
	void lvcards_map(address_map &map);
	void lvcardsa_decrypted_opcodes_map(address_map &map);
};


class lvpoker_state : public lvcards_state
{
public:
	using lvcards_state::lvcards_state;

	void lvpoker(machine_config &config);
	void ponttehk(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void control_port_2_w(uint8_t data);
	void control_port_2a_w(uint8_t data);
	uint8_t payout_r();

	void lvpoker_map(address_map &map);
	void ponttehk_map(address_map &map);

	uint8_t m_payout = 0U;
	uint8_t m_pulse = 0U;
	uint8_t m_result = 0U;
};

#endif // MAME_INCLUDES_LVCARDS_H
