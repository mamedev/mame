// license:BSD-3-Clause
// copyright-holders:David Haywood, Mike Coates
#ifndef MAME_KANEKO_SNOWBROS_H
#define MAME_KANEKO_SNOWBROS_H

#pragma once

#include "kan_pand.h" // for the original pandora

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"

// base class, no pandora chip
class snowbros_state : public driver_device
{
public:
	snowbros_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this,"maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_oki(*this, "oki%u", 1U)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_soundlatch(*this, "soundlatch%u", 1U)
		, m_bootleg_spriteram(*this, "spriteram")
	{ }

	void twinadv(machine_config &config);
	void wintbob(machine_config &config);
	void honeydol(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device_array<okim6295_device, 2> m_oki;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	optional_device_array<generic_latch_8_device, 2> m_soundlatch; // not snowbro3

	optional_shared_ptr<u16> m_bootleg_spriteram;

	void bootleg_flipscreen_w(u8 data);
	template <unsigned Line> void irq_ack_w(u16 data);
	void twinadv_oki_bank_w(u8 data);

	u32 screen_update_honeydol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_twinadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_wintbob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void snowbros_base(machine_config &config) ATTR_COLD;

	void honeydol_map(address_map &map) ATTR_COLD;
	void honeydol_sound_io_map(address_map &map) ATTR_COLD;
	void honeydol_sound_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void twinadv_map(address_map &map) ATTR_COLD;
	void twinadv_sound_io_map(address_map &map) ATTR_COLD;
	void wintbob_map(address_map &map) ATTR_COLD;
};

// with original pandora hardware
class snowbros_pandora_state : public snowbros_state
{
public:
	snowbros_pandora_state(const machine_config &mconfig, device_type type, const char *tag)
		: snowbros_state(mconfig, type, tag)
		, m_pandora(*this, "pandora")
	{ }

	void snowbros(machine_config &config);
	void yutnori(machine_config &config);

	void init_toto();
	void init_yutnori();

protected:
	required_device<kaneko_pandora_device> m_pandora;

	u16 m_yutnori_prot_val = 0;

	void snowbros_flipscreen_w(u8 data);
	u16 toto_read(offs_t offset, u16 mem_mask = ~0);
	void yutnori_prot_w(u16 data);
	u16 yutnori_prot_r();

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void snowbros_map(address_map &map) ATTR_COLD;
	void yutnori_map(address_map &map) ATTR_COLD;
};

// with sound MCU (HLE until hooked up MCU) and bankswitched OKI
class snowbros3_state : public snowbros_state
{
public:
	snowbros3_state(const machine_config &mconfig, device_type type, const char *tag)
		: snowbros_state(mconfig, type, tag)
		, m_okibank(*this, "okibank")
	{ }

	void snowbro3(machine_config &config);

	void init_snowbro3();
	void init_ballboy3p();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_bank m_okibank;

	s32 m_sb3_music_is_playing = 0;
	s32 m_sb3_music = 0;

	u16 sb3_sound_r();
	void sb3_sound_w(u16 data);

	u32 screen_update_snowbro3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(snowbros3_scanline);

	void sb3_play_music(u16 data);
	void sb3_play_sound(u16 data);

	void snowbros3_map(address_map &map) ATTR_COLD;
	void snowbros3_oki_map(address_map &map) ATTR_COLD;
};

// with protection MCU
class semicom_state : public snowbros_pandora_state
{
public:
	semicom_state(const machine_config &mconfig, device_type type, const char *tag)
		: snowbros_pandora_state(mconfig, type, tag)
		, m_hyperpac_ram(*this, "hyperpac_ram")
		, m_semicom_prot_base(0)
	{ }

	void _4in1(machine_config &config);
	void semiprot(machine_config &config);
	void semicom_mcu(machine_config &config);
	void semicom(machine_config &config);
	void finalttr(machine_config &config);

	void init_pzlbreak();
	void init_cookbib3();
	void init_4in1boot();
	void init_3in1semi();
	void init_cookbib2();
	void init_hyperpac();
	void init_sutjarod();
	void init_gwasu();

protected:
	optional_shared_ptr<u16> m_hyperpac_ram;

	u8 m_semicom_prot_offset = 0;
	u16 m_semicom_prot_base;

	void prot_p0_w(u8 data);
	void prot_p1_w(u8 data);
	void prot_p2_w(u8 data);
	u16 _4in1_02_read();
	u16 _3in1_read();
	u16 cookbib3_read();

	DECLARE_MACHINE_RESET(semiprot);
	DECLARE_MACHINE_RESET(finalttr);

	void finalttr_map(address_map &map) ATTR_COLD;
	void hyperpac_map(address_map &map) ATTR_COLD;
	void hyperpac_sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_KANEKO_SNOWBROS_H
