// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari "Stella on Steroids" hardware

***************************************************************************/
#ifndef MAME_INCLUDES_BEATHEAD_H
#define MAME_INCLUDES_BEATHEAD_H

#pragma once

#include "machine/atarigen.h"
#include "machine/timer.h"
#include "cpu/asap/asap.h"
#include "audio/atarijsa.h"

class beathead_state : public atarigen_state
{
public:
	beathead_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_jsa(*this, "jsa"),
		m_videoram(*this, "videoram"),
		m_vram_bulk_latch(*this, "vram_bulk_latch"),
		m_palette_select(*this, "palette_select"),
		m_ram_base(*this, "ram_base"),
		m_rom_base(*this, "rom_base")
	{ }

	void beathead(machine_config &config);

protected:
	// in drivers/beathead.c
	virtual void update_interrupts() override;
	DECLARE_WRITE32_MEMBER( interrupt_control_w );
	DECLARE_READ32_MEMBER( interrupt_control_r );
	DECLARE_WRITE32_MEMBER( sound_reset_w );
	DECLARE_WRITE32_MEMBER( coin_count_w );
	DECLARE_READ32_MEMBER( speedup_r );
	DECLARE_READ32_MEMBER( movie_speedup_r );

	// in video/beathead.c
	DECLARE_WRITE32_MEMBER( vram_transparent_w );
	DECLARE_WRITE32_MEMBER( vram_bulk_w );
	DECLARE_WRITE32_MEMBER( vram_latch_w );
	DECLARE_WRITE32_MEMBER( vram_copy_w );
	DECLARE_WRITE32_MEMBER( finescroll_w );
	DECLARE_READ32_MEMBER( hsync_ram_r );
	DECLARE_WRITE32_MEMBER( hsync_ram_w );
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);

	virtual void machine_reset() override;
	virtual void video_start() override;
	void main_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	required_device<atari_jsa_iii_device> m_jsa;

	required_shared_ptr<uint32_t> m_videoram;

	required_shared_ptr<uint32_t> m_vram_bulk_latch;
	required_shared_ptr<uint32_t> m_palette_select;

	uint32_t          m_finescroll;
	offs_t          m_vram_latch_offset;

	offs_t          m_hsyncram_offset;
	offs_t          m_hsyncram_start;
	uint8_t           m_hsyncram[0x800];

	required_shared_ptr<uint32_t> m_ram_base;
	required_shared_ptr<uint32_t> m_rom_base;

	attotime        m_hblank_offset;

	uint8_t           m_irq_line_state;
	uint8_t           m_irq_enable[3];
	uint8_t           m_irq_state[3];

	uint8_t           m_eeprom_enabled;
};

#endif // MAME_INCLUDES_BEATHEAD_H
