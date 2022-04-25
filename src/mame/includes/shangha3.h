// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_SHANGHA3_H
#define MAME_INCLUDES_SHANGHA3_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"

class shangha3_state : public driver_device
{
public:
	shangha3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_ram(*this, "ram"),
		m_cgrom(*this, "gfx1"),
		m_okibank(*this, "okibank")
	{ }

	void shangha3(machine_config &config);
	void heberpop(machine_config &config);
	void blocken(machine_config &config);

	void init_shangha3();
	void init_heberpop();
	void init_blocken();

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_ram;
	required_region_ptr<uint8_t> m_cgrom;

	optional_memory_bank m_okibank;

	// driver init configuration
	int m_do_shadows = 0;
	uint8_t m_drawmode_table[16]{};

	int m_prot_count = 0;
	uint16_t m_gfxlist_addr = 0;
	bitmap_ind16 m_rawbitmap;

	// common
	void flipscreen_w(uint8_t data);
	void gfxlist_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void blitter_go_w(uint16_t data);
	void irq_ack_w(uint16_t data);
	uint8_t cgrom_r(offs_t offset);

	// shangha3 specific
	uint16_t shangha3_prot_r();
	void shangha3_prot_w(uint16_t data);
	void shangha3_coinctrl_w(uint8_t data);

	// heberpop specific
	void heberpop_coinctrl_w(uint8_t data);

	// blocken specific
	void blocken_coinctrl_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void blocken_map(address_map &map);
	void blocken_oki_map(address_map &map);
	void heberpop_map(address_map &map);
	void heberpop_sound_io_map(address_map &map);
	void heberpop_sound_map(address_map &map);
	void shangha3_map(address_map &map);
};

#endif // MAME_INCLUDES_SHANGHA3_H
