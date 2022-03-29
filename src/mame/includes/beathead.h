// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari "Stella on Steroids" hardware

***************************************************************************/
#ifndef MAME_INCLUDES_BEATHEAD_H
#define MAME_INCLUDES_BEATHEAD_H

#pragma once

#include "machine/timer.h"
#include "cpu/asap/asap.h"
#include "audio/atarijsa.h"
#include "emupal.h"
#include "screen.h"

class beathead_state : public driver_device
{
public:
	beathead_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_jsa(*this, "jsa"),
		m_scan_timer(*this, "scan_timer"),
		m_videoram(*this, "videoram"),
		m_vram_bulk_latch(*this, "vram_bulk_latch"),
		m_palette_select(*this, "palette_select"),
		m_ram_base(*this, "ram_base"),
		m_rom_base(*this, "user1")
	{ }

	void beathead(machine_config &config);

protected:
	// in drivers/beathead.cpp
	void update_interrupts();
	void interrupt_control_w(offs_t offset, uint32_t data);
	uint32_t interrupt_control_r();
	void sound_reset_w(offs_t offset, uint32_t data);
	void coin_count_w(offs_t offset, uint32_t data);

	// in video/beathead.cpp
	void vram_transparent_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vram_bulk_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void vram_latch_w(offs_t offset, uint32_t data);
	void vram_copy_w(offs_t offset, uint32_t data);
	void finescroll_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t hsync_ram_r(offs_t offset);
	void hsync_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);

	virtual void machine_reset() override;
	virtual void video_start() override;
	void main_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_device<atari_jsa_iii_device> m_jsa;
	required_device<timer_device> m_scan_timer;

	required_shared_ptr<uint32_t> m_videoram;

	required_shared_ptr<uint32_t> m_vram_bulk_latch;
	required_shared_ptr<uint32_t> m_palette_select;

	uint32_t          m_finescroll = 0U;
	offs_t          m_vram_latch_offset = 0U;

	offs_t          m_hsyncram_offset = 0U;
	offs_t          m_hsyncram_start = 0U;
	uint8_t           m_hsyncram[0x800]{};

	required_shared_ptr<uint32_t> m_ram_base;
	required_region_ptr<uint32_t> m_rom_base;

	attotime        m_hblank_offset{};

	uint8_t           m_irq_line_state = 0U;
	uint8_t           m_irq_enable[3]{};
	uint8_t           m_irq_state[3]{};
};

#endif // MAME_INCLUDES_BEATHEAD_H
