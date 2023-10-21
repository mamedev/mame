// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Jaleco Mega System 1 =-

                    driver by   Luca Elia (l.elia@tin.it)

***************************************************************************/
#ifndef MAME_JALECO_MEGASYS1_H
#define MAME_JALECO_MEGASYS1_H

#pragma once

#include "cpu/tlcs90/tlcs90.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "ms1_tmap.h"
#include "emupal.h"
#include "screen.h"


class megasys1_state : public driver_device
{
public:
	megasys1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch%u", 1U),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_tmap(*this, "scroll%u", 0),
		m_oki(*this, "oki%u", 1U),
		m_ram(*this, "ram"),
		m_io_system(*this, "SYSTEM"),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2"),
		m_io_dsw(*this, "DSW"),
		m_io_dsw1(*this, "DSW1"),
		m_io_dsw2(*this, "DSW2"),
		m_scantimer(*this, "scantimer"),
		m_objectram(*this, "objectram"),
		m_ymsnd(*this, "ymsnd"),
		m_p47b_adpcm(*this, "msm%u", 1U),
		m_rom_maincpu(*this, "maincpu"),
		m_okibank(*this, "okibank")
	{
		m_hardware_type_z = 0;
	}

	void system_A_soldam(machine_config &config);
	void system_B_monkelf(machine_config &config);
	void system_A_iganinju(machine_config &config);
	void kickoffb(machine_config &config);
	void p47b(machine_config &config);
	void system_D(machine_config &config);
	void system_C(machine_config &config);
	void system_Bbl(machine_config &config);
	void system_A(machine_config &config);
	void system_A_jitsupro(machine_config &config);

	void init_peekaboo();
	void init_soldam();
	void init_astyanax();
	void init_stdragon();
	void init_soldamj();
	void init_phantasm();
	void init_jitsupro();
	void init_iganinju();
	void init_rodlandj();
	void init_rittam();
	void init_rodlandjb();
	void init_monkelf();
	void init_edfp();
	void init_rodland();
	void init_stdragona();
	void init_stdragonb();
	void init_lordofkbp();

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device_array<generic_latch_16_device, 2> m_soundlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	optional_device_array<megasys1_tilemap_device, 3> m_tmap;
	optional_device_array<okim6295_device, 2> m_oki;
	required_shared_ptr<u16> m_ram;
	required_ioport m_io_system;
	required_ioport m_io_p1;
	required_ioport m_io_p2;
	optional_ioport m_io_dsw;
	optional_ioport m_io_dsw1;
	optional_ioport m_io_dsw2;
	optional_device<timer_device> m_scantimer;

	void megasys1B_map(address_map &map);
	void megasys1C_map(address_map &map);

	void megasys1c_handle_scanline_irq(int scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(megasys1A_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(megasys1B_scanline);

	void megasys_base_map(address_map &map);
	void megasys1B_sound_map(address_map &map);

	virtual void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void mix_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void partial_clear_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 param);
	inline void draw_16x16_priority_sprite(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect, s32 code, s32 color, s32 sx, s32 sy, s32 flipx, s32 flipy, u8 mosaic, u8 mosaicsol, s32 priority);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void screen_flag_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	int m_hardware_type_z = 0; // System Z

	u8 m_ignore_oki_status = 0;

	 // System C
	u16 m_sprite_bank = 0;

	// soldam
	u16 *m_spriteram = nullptr;

	u16 m_screen_flag = 0;

	// all
	bitmap_ind16 m_sprite_buffer_bitmap;
	std::unique_ptr<u16[]> m_buffer_objectram;
	std::unique_ptr<u16[]> m_buffer2_objectram;
	std::unique_ptr<u16[]> m_buffer_spriteram16;
	std::unique_ptr<u16[]> m_buffer2_spriteram16;

	// all but System Z
	u16 m_active_layers = 0;
	u16 m_sprite_flag = 0;

private:
	required_shared_ptr<u16> m_objectram;
	optional_device<device_t> m_ymsnd;
	optional_device_array<msm5205_device, 2> m_p47b_adpcm;
	required_region_ptr<u16> m_rom_maincpu;
	optional_memory_bank m_okibank;

	// configuration
	int m_layers_order[16]{};

	// System A only
	int m_mcu_hs = 0;
	u16 m_mcu_hs_ram[0x10]{};

	// peekaboo
	u16 m_protection_val = 0;

	void sound_irq(int state);
	u16 protection_peekaboo_r();
	void protection_peekaboo_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 megasys1A_mcu_hs_r(offs_t offset, u16 mem_mask = ~0);
	void megasys1A_mcu_hs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 iganinju_mcu_hs_r(offs_t offset, u16 mem_mask = ~0);
	void iganinju_mcu_hs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 soldamj_spriteram16_r(offs_t offset);
	void soldamj_spriteram16_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 stdragon_mcu_hs_r(offs_t offset, u16 mem_mask = ~0);
	void stdragon_mcu_hs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void active_layers_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprite_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sprite_flag_r();
	void sprite_flag_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void soundlatch_w(u16 data);
	void soundlatch_c_w(u16 data);
	void monkelf_scroll0_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void monkelf_scroll1_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template<int Chip> u8 oki_status_r();
	void ram_w(offs_t offset, u16 data);
	void p47b_adpcm_w(offs_t offset, u8 data);


	void megasys1_palette(palette_device &palette);

	void screen_vblank(int state);
	INTERRUPT_GEN_MEMBER(megasys1D_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(megasys1A_iganinju_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(megasys1C_scanline);

	void priority_create();
	void rodland_gfx_unmangle(const char *region);
	void jitsupro_gfx_unmangle(const char *region);
	void stdragona_gfx_unmangle(const char *region);
	void kickoffb_sound_map(address_map &map);
	void p47b_sound_map(address_map &map);
	void p47b_extracpu_prg_map(address_map &map);
	void p47b_extracpu_io_map(address_map &map);
	void megasys1A_map(address_map &map);
	void megasys1A_sound_map(address_map &map);
	void megasys1A_jitsupro_sound_map(address_map &map);
	void megasys1B_edfbl_map(address_map &map);
	void megasys1B_monkelf_map(address_map &map);
	void megasys1D_map(address_map &map);
	void megasys1D_oki_map(address_map &map);
};

class megasys1_hachoo_state : public megasys1_state
{
public:
	megasys1_hachoo_state(const machine_config &mconfig, device_type type, const char *tag) :
		megasys1_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;
};

class megasys1_typez_state : public megasys1_state
{
public:
	megasys1_typez_state(const machine_config &mconfig, device_type type, const char *tag) :
		megasys1_state(mconfig, type, tag)
	{
		m_hardware_type_z = 1;
	}

	void system_Z(machine_config &config);

protected:
	virtual void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect) override;

private:
	void soundlatch_z_w(u16 data);

	void megasys1Z_map(address_map &map);
	void z80_sound_io_map(address_map &map);
	void z80_sound_map(address_map &map);
};


class megasys1_bc_iosim_state : public megasys1_state
{
public:
	megasys1_bc_iosim_state(const machine_config &mconfig, device_type type, const char *tag) :
		megasys1_state(mconfig, type, tag)
	{ }

	void init_avspirit();
	void init_64street();
	void init_chimerab();
	void init_chimeraba();
	void init_cybattlr();
	void init_hayaosi1();
	void init_edf();

	void system_B(machine_config &config);
	void system_B_hayaosi1(machine_config &config);
	void system_C_iosim(machine_config &config);

protected:
	virtual void machine_reset() override;

	u16 m_ip_select_values[7]{}; // System B and C

	u16 m_ip_latched = 0;

	void megasys1B_iosim_map(address_map &map);
	void megasys1C_iosim_map(address_map &map);

	u16 ip_select_r();
	void ip_select_w(u16 data);
};

class megasys1_bc_iomcu_state : public megasys1_state
{
public:
	megasys1_bc_iomcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		megasys1_state(mconfig, type, tag),
		m_iomcu(*this, "iomcu")
	{ }

	void init_bigstrik();

	void system_C_bigstrik(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	required_device<tlcs90_device> m_iomcu;

	u16 ip_select_bigstrik_r();
	void ip_select_bigstrik_w(u16 data);
	u8 mcu_capture_inputs_r(offs_t offset);
	u8 mcu_port1_r();
	void mcu_port2_w(u8 data);
	void mcu_port6_w(u8 data);

	u8 m_mcu_input_data;
	u8 m_mcu_io_data;

	void megasys1C_bigstrik_map(address_map &map);
	void iomcu_map(address_map &map);

	TIMER_DEVICE_CALLBACK_MEMBER(megasys1C_bigstrik_scanline);
};

#endif // MAME_JALECO_MEGASYS1_H
