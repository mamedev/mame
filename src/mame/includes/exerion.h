// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Jaleco Exerion

*************************************************************************/
#ifndef MAME_INCLUDES_EXERION_H
#define MAME_INCLUDES_EXERION_H

#pragma once

#include "emupal.h"
#include "screen.h"


#define EXERION_MASTER_CLOCK      (XTAL(19'968'000))   /* verified on pcb */
#define EXERION_CPU_CLOCK         (EXERION_MASTER_CLOCK / 6)
#define EXERION_AY8910_CLOCK      (EXERION_CPU_CLOCK / 2)
#define EXERION_PIXEL_CLOCK       (EXERION_MASTER_CLOCK / 3)
#define EXERION_HCOUNT_START      (0x58)
#define EXERION_HTOTAL            (512-EXERION_HCOUNT_START)
#define EXERION_HBEND             (12*8)    /* ?? */
#define EXERION_HBSTART           (52*8)    /* ?? */
#define EXERION_VTOTAL            (256)
#define EXERION_VBEND             (16)
#define EXERION_VBSTART           (240)


class exerion_state : public driver_device
{
public:
	exerion_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_main_ram(*this, "main_ram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu_region(*this, "maincpu"),
		m_background_mixer(*this, "bg_char_mixer_prom"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_inputs(*this, "P%u", 1U)
	{ }

	void exerion(machine_config &config);
	void irion(machine_config &config);

	void init_exerion();
	void init_exerionb();
	void init_irion();

	DECLARE_CUSTOM_INPUT_MEMBER(controls_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_main_ram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_maincpu_region;
	required_region_ptr<uint8_t> m_background_mixer;

	// video-related
	uint8_t    m_cocktail_flip = 0U;
	uint8_t    m_char_palette = 0U;
	uint8_t    m_sprite_palette = 0U;
	uint8_t    m_char_bank = 0U;
	std::unique_ptr<uint16_t[]>  m_background_gfx[4]{};
	uint8_t    m_background_latches[13]{};

	// protection?
	uint8_t m_porta = 0U;
	uint8_t m_portb = 0U;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_ioport_array<2> m_inputs;

	uint8_t protection_r(offs_t offset);
	void videoreg_w(uint8_t data);
	void video_latch_w(offs_t offset, uint8_t data);
	uint8_t video_timing_r();
	uint8_t porta_r();
	void portb_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);
	void sub_map(address_map &map);
};

#endif // MAME_INCLUDES_EXERION_H
