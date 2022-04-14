// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_INCLUDES_TRUCOCL_H
#define MAME_INCLUDES_TRUCOCL_H

#pragma once

#include "sound/dac.h"
#include "emupal.h"
#include "tilemap.h"

class trucocl_state : public driver_device
{
public:
	trucocl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode") { }

	void trucocl(machine_config &config);

	void init_trucocl();

protected:
	enum
	{
		TIMER_DAC_IRQ
	};

	virtual void machine_reset() override;
	virtual void video_start() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	int m_cur_dac_address = 0;
	int m_cur_dac_address_index = 0;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t m_irq_mask = 0;
	emu_timer *m_dac_irq_timer = nullptr;

	void irq_enable_w(uint8_t data);
	void trucocl_videoram_w(offs_t offset, uint8_t data);
	void trucocl_colorram_w(offs_t offset, uint8_t data);
	void audio_dac_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void trucocl_palette(palette_device &palette) const;
	uint32_t screen_update_trucocl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(trucocl_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<dac_byte_interface> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;

	void main_map(address_map &map);
	void main_io(address_map &map);
};

#endif // MAME_INCLUDES_TRUCOCL_H
