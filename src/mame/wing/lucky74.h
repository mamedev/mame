// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
#ifndef MAME_WING_LUCKY74_H
#define MAME_WING_LUCKY74_H

#pragma once

#include "sound/msm5205.h"
#include "emupal.h"
#include "tilemap.h"

class lucky74_state : public driver_device
{
public:
	lucky74_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_colorram(*this, "bg_colorram"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void lucky74(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void sound_start() override;

private:
	uint8_t custom_09R81P_port_r(offs_t offset);
	void custom_09R81P_port_w(offs_t offset, uint8_t data);
	uint8_t usart_8251_r();
	void usart_8251_w(uint8_t data);
	uint8_t copro_sm7831_r();
	void copro_sm7831_w(uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void fg_colorram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	void bg_colorram_w(offs_t offset, uint8_t data);
	void ym2149_portb_w(uint8_t data);
	void lamps_a_w(uint8_t data);
	void lamps_b_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(nmi_interrupt);
	void adpcm_int(int state);
	void prg_map(address_map &map) ATTR_COLD;
	void portmap(address_map &map) ATTR_COLD;

	uint8_t m_ym2149_portb = 0U;
	uint8_t m_usart_8251 = 0U;
	uint8_t m_copro_sm7831 = 0U;
	int m_adpcm_pos = 0;
	int m_adpcm_end = 0;
	int m_adpcm_data = 0;
	uint8_t m_adpcm_reg[6]{};
	uint8_t m_adpcm_busy_line = 0;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_fg_colorram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_bg_colorram;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<12> m_lamps;
};

#endif // MAME_WING_LUCKY74_H
