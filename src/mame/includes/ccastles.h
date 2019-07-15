// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Crystal Castles hardware

*************************************************************************/
#ifndef MAME_INCLUDES_CCASTLES_H
#define MAME_INCLUDES_CCASTLES_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/x2212.h"
#include "emupal.h"
#include "screen.h"

class ccastles_state : public driver_device
{
public:
	ccastles_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram_4b(*this, "nvram_4b"),
		m_nvram_4a(*this, "nvram_4a"),
		m_outlatch(*this, "outlatch%u", 0U),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(get_vblank);
	void ccastles(machine_config &config);

protected:
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(leta_r);
	DECLARE_WRITE8_MEMBER(nvram_recall_w);
	DECLARE_WRITE_LINE_MEMBER(nvram_store_w);
	DECLARE_READ8_MEMBER(nvram_r);
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_WRITE8_MEMBER(ccastles_hscroll_w);
	DECLARE_WRITE8_MEMBER(ccastles_vscroll_w);
	DECLARE_WRITE8_MEMBER(ccastles_video_control_w);
	DECLARE_WRITE8_MEMBER(ccastles_paletteram_w);
	DECLARE_WRITE8_MEMBER(ccastles_videoram_w);
	DECLARE_READ8_MEMBER(ccastles_bitmode_r);
	DECLARE_WRITE8_MEMBER(ccastles_bitmode_w);
	DECLARE_WRITE8_MEMBER(ccastles_bitmode_addr_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_ccastles(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(clock_irq);
	inline void ccastles_write_vram( uint16_t addr, uint8_t data, uint8_t bitmd, uint8_t pixba );
	inline void bitmode_autoinc(  );
	inline void schedule_next_irq( int curscanline );
	void main_map(address_map &map);

private:
	/* devices */
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram_4b;
	required_device<x2212_device> m_nvram_4a;
	required_device_array<ls259_device, 2> m_outlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	const uint8_t *m_syncprom;
	const uint8_t *m_wpprom;
	const uint8_t *m_priprom;
	bitmap_ind16 m_spritebitmap;
	double m_rweights[3];
	double m_gweights[3];
	double m_bweights[3];
	uint8_t m_bitmode_addr[2];
	uint8_t m_hscroll;
	uint8_t m_vscroll;

	/* misc */
	int      m_vblank_start;
	int      m_vblank_end;
	emu_timer *m_irq_timer;
	uint8_t    m_irq_state;
};

#endif // MAME_INCLUDES_CCASTLES_H
