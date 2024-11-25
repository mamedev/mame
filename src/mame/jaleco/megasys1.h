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
#include "ms1_gatearray.h"
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
		m_rom_maincpu(*this, "maincpu"),
		m_objectram(*this, "objectram"),
		m_ymsnd(*this, "ymsnd")
	{
		m_hardware_type_z = 0;
	}

	void system_B_monkelf(machine_config &config);

	void system_C(machine_config &config);
	void system_Bbl(machine_config &config);
	void system_base(machine_config &config);

	void init_monkelf();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

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
	required_region_ptr<u16> m_rom_maincpu;

	void megasys1B_map(address_map &map) ATTR_COLD;
	void megasys1C_map(address_map &map) ATTR_COLD;

	void megasys1c_handle_scanline_irq(int scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(megasys_base_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(megasys1B_scanline);

	void megasys_base_map(address_map &map) ATTR_COLD;
	void megasys1B_sound_map(address_map &map) ATTR_COLD;

	void megasys1_palette(palette_device &palette);

	virtual void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void mix_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void partial_clear_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 param);
	inline void draw_16x16_priority_sprite(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect, s32 code, s32 color, s32 sx, s32 sy, s32 flipx, s32 flipy, u8 mosaic, u8 mosaicsol, s32 priority);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_irq(int state);
	void screen_vblank(int state);

	void screen_flag_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void active_layers_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sprite_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sprite_flag_r();
	void sprite_flag_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void soundlatch_w(u16 data);
	void soundlatch_c_w(u16 data);
	template<int Chip> u8 oki_status_r();

	int m_hardware_type_z = 0; // System Z

	u8 m_ignore_oki_status = 0;

	 // System C
	u16 m_sprite_bank = 0;

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

	// configuration
	int m_layers_order[16]{};

	void monkelf_scroll0_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void monkelf_scroll1_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void ram_w(offs_t offset, u16 data);


	TIMER_DEVICE_CALLBACK_MEMBER(megasys1C_scanline);

	void priority_create();

	void megasys1B_edfbl_map(address_map &map) ATTR_COLD;
	void megasys1B_monkelf_map(address_map &map) ATTR_COLD;
};

class megasys1_typea_state : public megasys1_state
{
public:
	megasys1_typea_state(const machine_config &mconfig, device_type type, const char *tag) :
		megasys1_state(mconfig, type, tag),
		m_p47bl_adpcm(*this, "msm%u", 1U),
		m_gatearray(*this, "gatearray")
	{ }

	void system_A(machine_config &config);
	void system_A_d65006_soldam(machine_config &config);
	void system_A_gs88000_soldam(machine_config &config);
	void system_A_iganinju(machine_config &config);
	void system_A_kickoffb(machine_config &config);
	void system_A_p47bl(machine_config &config);
	void system_A_d65006(machine_config &config);
	void system_A_d65006_iganinju(machine_config &config);
	void system_A_gs88000(machine_config &config);
	void system_A_unkarray(machine_config &config);

	void init_jitsupro_gfx();
	void init_rodland_gfx();
	void init_stdragon_gfx();
	void init_lordofkbp();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void kickoffb_sound_map(address_map &map) ATTR_COLD;
	void p47bl_sound_map(address_map &map) ATTR_COLD;
	void p47bl_extracpu_prg_map(address_map &map) ATTR_COLD;
	void p47bl_extracpu_io_map(address_map &map) ATTR_COLD;
	void megasys1A_map(address_map &map) ATTR_COLD;
	void megasys1A_sound_map(address_map &map) ATTR_COLD;

	void p47bl_adpcm_w(offs_t offset, u8 data);


private:
	optional_device_array<msm5205_device, 2> m_p47bl_adpcm;
	optional_device<megasys1_gatearray_device> m_gatearray;

	TIMER_DEVICE_CALLBACK_MEMBER(megasys1A_iganinju_scanline);

	void rodland_gfx_unmangle(const char *region);
	void jitsupro_gfx_unmangle(const char *region);
	void stdragona_gfx_unmangle(const char *region);
};

class megasys1_typea_hachoo_state : public megasys1_typea_state
{
public:
	megasys1_typea_hachoo_state(const machine_config &mconfig, device_type type, const char *tag) :
		megasys1_typea_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override ATTR_COLD;
};

class megasys1_typed_state : public megasys1_state
{
public:
	megasys1_typed_state(const machine_config &mconfig, device_type type, const char *tag) :
		megasys1_state(mconfig, type, tag),
		m_okibank(*this, "okibank")
	{ }

	void system_D(machine_config &config);

	void init_peekaboo();

private:
	required_memory_bank m_okibank;

	// peekaboo
	u16 m_protection_val = 0;

	u16 protection_peekaboo_r();
	void protection_peekaboo_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	INTERRUPT_GEN_MEMBER(megasys1D_irq);

	void megasys1D_map(address_map &map) ATTR_COLD;
	void megasys1D_oki_map(address_map &map) ATTR_COLD;
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

	void megasys1Z_map(address_map &map) ATTR_COLD;
	void z80_sound_io_map(address_map &map) ATTR_COLD;
	void z80_sound_map(address_map &map) ATTR_COLD;
};


class megasys1_bc_iosim_state : public megasys1_state
{
public:
	megasys1_bc_iosim_state(const machine_config &mconfig, device_type type, const char *tag) :
		megasys1_state(mconfig, type, tag)
	{ }

	void init_avspirit();
	void init_chimeraba();
	void init_hayaosi1();
	void init_edf();

	void system_B(machine_config &config);
	void system_B_hayaosi1(machine_config &config);
	void system_C_iosim(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	const u8 *m_ip_select_values = nullptr; // System B and C

	u16 m_ip_latched = 0;

	static constexpr u8 avspirit_seq[7] =    { 0x37,0x35,0x36,0x33,0x34,  0xff,0x06 };
	static constexpr u8 edf_seq[7] =         { 0x20,0x21,0x22,0x23,0x24,  0xf0,0x06 };
	static constexpr u8 hayaosi1_seq[7] =    { 0x51,0x52,0x53,0x54,0x55,  0xfc,0x06 };
	static constexpr u8 chimeraba_seq[7]   = { 0x56,0x52,0x53,0x55,0x54,  0xfa,0x06 };

	void megasys1B_iosim_map(address_map &map) ATTR_COLD;
	void megasys1C_iosim_map(address_map &map) ATTR_COLD;

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

	void system_C_iomcu(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<tlcs90_device> m_iomcu;

	u16 ip_select_iomcu_r();
	void ip_select_iomcu_w(u16 data);
	u8 mcu_capture_inputs_r(offs_t offset);
	u8 mcu_port1_r();
	void mcu_port2_w(u8 data);
	void mcu_port6_w(u8 data);

	u8 m_mcu_input_data;
	u8 m_mcu_io_data;

	void megasys1C_iomcu_map(address_map &map) ATTR_COLD;
	void iomcu_map(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(megasys1C_iomcu_scanline);
};

#endif // MAME_JALECO_MEGASYS1_H
