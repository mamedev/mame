// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Aaron Giles
/*************************************************************************

    Atari Cloud 9 (prototype) hardware

*************************************************************************/
#ifndef MAME_INCLUDES_CLOUD9_H
#define MAME_INCLUDES_CLOUD9_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/x2212.h"
#include "emupal.h"
#include "screen.h"

class cloud9_state : public driver_device
{
public:
	cloud9_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram") ,
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videolatch(*this, "videolatch")
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(get_vblank);
	void cloud9(machine_config &config);

protected:
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(leta_r);
	DECLARE_WRITE8_MEMBER(nvram_recall_w);
	DECLARE_WRITE8_MEMBER(nvram_store_w);
	DECLARE_WRITE8_MEMBER(cloud9_paletteram_w);
	DECLARE_WRITE8_MEMBER(cloud9_videoram_w);
	DECLARE_READ8_MEMBER(cloud9_bitmode_r);
	DECLARE_WRITE8_MEMBER(cloud9_bitmode_w);
	DECLARE_WRITE8_MEMBER(cloud9_bitmode_addr_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cloud9(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(clock_irq);
	inline void cloud9_write_vram( uint16_t addr, uint8_t data, uint8_t bitmd, uint8_t pixba );
	inline void bitmode_autoinc();
	inline void schedule_next_irq(int curscanline);
	void cloud9_map(address_map &map);

private:
	/* devices */
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram;
	/* memory pointers */
	std::unique_ptr<uint8_t[]>    m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_videolatch;

	/* video-related */
	const uint8_t *m_syncprom;
	const uint8_t *m_wpprom;
	const uint8_t *m_priprom;
	bitmap_ind16 m_spritebitmap;
	double      m_rweights[3];
	double      m_gweights[3];
	double      m_bweights[3];
	uint8_t       m_bitmode_addr[2];

	/* misc */
	int         m_vblank_start;
	int         m_vblank_end;
	emu_timer   *m_irq_timer;
	uint8_t       m_irq_state;
};

#endif // MAME_INCLUDES_CLOUD9_H
