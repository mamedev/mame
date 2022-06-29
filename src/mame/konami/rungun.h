// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*************************************************************************

    Run and Gun / Slam Dunk

*************************************************************************/
#ifndef MAME_INCLUDES_RUNGUN_H
#define MAME_INCLUDES_RUNGUN_H

#pragma once

#include "sound/k054539.h"
#include "machine/k053252.h"
#include "k053246_k053247_k055673.h"
#include "video/k053936.h"
#include "machine/k054321.h"
#include "konami_helper.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class rungun_state : public driver_device
{
public:
	rungun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_k054539_1(*this, "k054539_1"),
		m_k054539_2(*this, "k054539_2"),
		m_k053936(*this, "k053936"),
		m_k055673(*this, "k055673"),
		m_k053252(*this, "k053252"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_palette2(*this, "palette2"),
		m_screen(*this, "screen"),
		m_k054321(*this, "k054321"),
		m_sysreg(*this, "sysreg")
	{ }

	void rng(machine_config &config);
	void rng_dual(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<k054539_device> m_k054539_1;
	required_device<k054539_device> m_k054539_2;
	required_device<k053936_device> m_k053936;
	required_device<k055673_device> m_k055673;
	required_device<k053252_device> m_k053252;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_palette2;
	required_device<screen_device> m_screen;
	required_device<k054321_device> m_k054321;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_sysreg;

	/* video-related */
	tilemap_t   *m_ttl_tilemap[2]{};
	tilemap_t   *m_936_tilemap[2]{};
	std::unique_ptr<uint16_t[]> m_psac2_vram;
	std::unique_ptr<uint16_t[]>    m_ttl_vram;
	std::unique_ptr<uint16_t[]>   m_pal_ram;
	uint8_t       m_current_display_bank = 0;
	int         m_ttl_gfx_index = 0;
	int         m_sprite_colorbase = 0;

	uint8_t       *m_roz_rom = nullptr;
	uint8_t       m_roz_rombase = 0;

	/* sound */
	uint8_t       m_sound_ctrl = 0;
	uint8_t       m_sound_status = 0;
	uint8_t       m_sound_nmi_clk = 0;

	bool        m_video_priority_mode = false;
	std::unique_ptr<uint16_t[]> m_banked_ram;
	bool        m_single_screen_mode = false;
	uint8_t       m_video_mux_bank = 0;

	uint16_t sysregs_r(offs_t offset, uint16_t mem_mask = ~0);
	void sysregs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_irq_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_status_w(uint8_t data);
	void sound_ctrl_w(uint8_t data);
	uint16_t ttl_ram_r(offs_t offset);
	void ttl_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t psac2_videoram_r(offs_t offset);
	void psac2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t k53936_rom_r(offs_t offset);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	TILE_GET_INFO_MEMBER(get_rng_936_tile_info);
	DECLARE_WRITE_LINE_MEMBER(k054539_nmi_gen);
	uint16_t palette_read(offs_t offset);
	void palette_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);


	K055673_CB_MEMBER(sprite_callback);

	uint32_t screen_update_rng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint32_t screen_update_rng_dual_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rng_dual_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	bitmap_ind16 m_rng_dual_demultiplex_left_temp;
	bitmap_ind16 m_rng_dual_demultiplex_right_temp;
	void   sprite_dma_trigger(void);

	INTERRUPT_GEN_MEMBER(rng_interrupt);

	void rungun_map(address_map &map);
	void rungun_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_RUNGUN_H
