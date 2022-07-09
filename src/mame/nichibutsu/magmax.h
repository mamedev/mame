// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_INCLUDES_MAGMAX_H
#define MAME_INCLUDES_MAGMAX_H

#pragma once

#include "screen.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "emupal.h"

class magmax_state : public driver_device
{
public:
	magmax_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_vreg(*this, "vreg"),
		m_scroll_x(*this, "scroll_x"),
		m_scroll_y(*this, "scroll_y"),
		m_rom18B(*this, "user1"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ay(*this, "ay%u", 0U),
		m_soundlatch(*this, "soundlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void magmax(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_vreg;
	required_shared_ptr<uint16_t> m_scroll_x;
	required_shared_ptr<uint16_t> m_scroll_y;
	required_region_ptr<uint8_t> m_rom18B;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<ay8910_device, 3> m_ay;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t m_sound_latch = 0;
	uint8_t m_LS74_clr = 0;
	uint8_t m_LS74_q = 0;
	uint8_t m_gain_control = 0;
	emu_timer *m_interrupt_timer = nullptr;
	uint8_t m_flipscreen = 0;
	std::unique_ptr<uint32_t[]> m_prom_tab;
	bitmap_ind16 m_bitmap;

	void cpu_irq_ack_w(uint16_t data);
	uint8_t sound_r();
	void vreg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ay8910_portB_0_w(uint8_t data);
	void ay8910_portA_0_w(uint8_t data);

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);

	void main_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_MAGMAX_H
