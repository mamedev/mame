// license:BSD-3-Clause
// copyright-holders:Aaron Giles

/***************************************************************************

    HAR MadMax hardware

**************************************************************************/
#ifndef MAME_INCLUDES_DCHEESE_H
#define MAME_INCLUDES_DCHEESE_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/bsmt2000.h"
#include "emupal.h"
#include "screen.h"

class dcheese_state : public driver_device
{
public:
	dcheese_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_palrom(*this, "palrom"),
		m_gfxrom(*this, "gfx"),
		m_eepromout_io(*this, "EEPROMOUT"),
		m_2a0002_io(*this, "2a0002"),
		m_2a000e_io(*this, "2a000e"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_bsmt(*this, "bsmt"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void fredmem(machine_config &config);
	void dcheese(machine_config &config);

protected:
	enum
	{
		TIMER_BLITTER_SCANLINE,
		TIMER_SIGNAL_IRQ
	};

	virtual void machine_start() override;
	virtual void video_start() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	required_region_ptr<u16> m_palrom;
	required_region_ptr<u8> m_gfxrom;
	required_ioport m_eepromout_io;
	required_ioport m_2a0002_io;
	required_ioport m_2a000e_io;

	/* video-related */
	u16   m_blitter_color[2]{};
	u16   m_blitter_xparam[16]{};
	u16   m_blitter_yparam[16]{};
	u16   m_blitter_vidparam[32]{};

	std::unique_ptr<bitmap_ind16> m_dstbitmap;
	emu_timer *m_blitter_timer = nullptr;
	emu_timer *m_signal_irq_timer = nullptr;

	/* misc */
	u8    m_irq_state[5]{};
	u8    m_sound_control = 0;
	u8    m_sound_msb_latch = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<bsmt2000_device> m_bsmt;
	required_device<generic_latch_8_device> m_soundlatch;

	void eeprom_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 sound_status_r();
	void sound_control_w(u8 data);
	void bsmt_data_w(offs_t offset, u8 data);
	void blitter_color_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blitter_xparam_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blitter_yparam_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blitter_vidparam_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blitter_unknown_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 blitter_vidparam_r(offs_t offset);
	void dcheese_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank);
	void signal_irq(u8 which);
	void update_irq_state();
	uint8_t iack_r(offs_t offset);
	void update_scanline_irq();
	void do_clear();
	void do_blit();

	void main_cpu_map(address_map &map);
	void main_fc7_map(address_map &map);
	void sound_cpu_map(address_map &map);
};

#endif // MAME_INCLUDES_DCHEESE_H
