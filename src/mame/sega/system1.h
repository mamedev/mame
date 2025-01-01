// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski, Nicola Salmoria, Mirko Buffoni
#ifndef MAME_SEGA_SYSTEM1_H
#define MAME_SEGA_SYSTEM1_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "machine/z80pio.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/segacrpt_device.h"
#include "machine/segacrp2_device.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class system1_state : public driver_device
{
public:
	system1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(nullptr),
		m_videomode_custom(nullptr),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_mcu(*this, "mcu"),
		m_sn(*this, "sn%u", 1U),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_ppi8255(*this, "ppi8255"),
		m_pio(*this, "pio"),
		m_ram(*this, "ram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu_region(*this, "maincpu"),
		m_spriterom(*this, "sprites"),
		m_lookup_prom(*this, "lookup_proms"),
		m_color_prom(*this, "color_proms"),
		m_bank1(*this, "bank1"),
		m_bank0d(*this, "bank0d"),
		m_bank1d(*this, "bank1d"),
		m_banked_decrypted_opcodes(nullptr)
	{ }

	void sys1ppi(machine_config &config);
	void sys1ppis(machine_config &config);
	void sys1pio(machine_config &config);
	void sys1pios(machine_config &config);
	void mcu(machine_config &config);
	void sys2(machine_config &config);
	void sys2x(machine_config &config);
	void sys2row(machine_config &config);

	void starjack(machine_config &config);
	void upndown(machine_config &config);
	void regulus(machine_config &config);
	void mrviking(machine_config &config);
	void swat(machine_config &config);
	void flickyo(machine_config &config);
	void bullfgt(machine_config &config);
	void wmatch(machine_config &config);
	void wboy2(machine_config &config);
	void wboy6(machine_config &config);
	void nob(machine_config &config);
	void nobb(machine_config &config);

	void flicky(machine_config &config);
	void thetogyu(machine_config &config);
	void spatter(machine_config &config);
	void spattera(machine_config &config);
	void pitfall2(machine_config &config);
	void seganinj(machine_config &config);
	void seganinja(machine_config &config);
	void nprinceso(machine_config &config);
	void imsorry(machine_config &config);
	void teddybb(machine_config &config);
	void teddybboa(machine_config &config);
	void myheroj(machine_config &config);
	void _4dwarrio(machine_config &config);
	void wboy(machine_config &config);
	void wboyo(machine_config &config);
	void blockgal(machine_config &config);
	void gardia(machine_config &config);

	void choplift(machine_config &config);
	void gardiab(machine_config &config);
	void gardiaj(machine_config &config);
	void wboysys2(machine_config &config);
	void wboysys2a(machine_config &config);
	void wbmlb(machine_config &config);
	void blockgalb(machine_config &config);
	void ufosensi(machine_config &config);
	void ufosensib(machine_config &config);

	void init_bank0c();
	void init_bank44();

	void init_nobb();
	void init_bootleg();
	void init_blockgal();
	void init_nob();
	void init_myherok();
	void init_ufosensi();
	void init_wbml();
	void init_bootsys2();
	void init_bootsys2d();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD { }
	virtual void video_start() override ATTR_COLD;

	// video related
	std::unique_ptr<u8[]> m_videoram;
	void (system1_state::*m_videomode_custom)(u8 data);
	std::unique_ptr<u8[]> m_mix_collide;
	u8 m_mix_collide_summary = 0;
	std::unique_ptr<u8[]> m_sprite_collide;
	u8 m_sprite_collide_summary = 0;
	bitmap_ind16 m_sprite_bitmap;
	u8 m_video_mode = 0;
	u8 m_videoram_bank = 0;
	tilemap_t *m_tilemap_page[8]{};
	u8 m_tilemap_pages = 0;

	// protection, misc
	u8 m_adjust_cycles = 0;
	u8 m_mcu_control = 0;
	u8 m_nob_maincpu_latch = 0;
	u8 m_nob_mcu_latch = 0;
	u8 m_nob_mcu_status = 0;
	u8 m_nobb_inport23_step = 0;

	// video handlers
	void common_videomode_w(u8 data);
	virtual void videomode_w(u8 data);
	void videoram_bank_w(u8 data);
	u8 mixer_collision_r(offs_t offset);
	void mixer_collision_w(offs_t offset, u8 data);
	void mixer_collision_reset_w(u8 data);
	u8 sprite_collision_r(offs_t offset);
	void sprite_collision_w(offs_t offset, u8 data);
	void sprite_collision_reset_w(u8 data);
	u8 videoram_r(offs_t offset);
	void videoram_w(offs_t offset, u8 data);
	void paletteram_w(offs_t offset, u8 data);

	// sound handlers
	u8 sound_data_r();
	void soundport_w(u8 data);
	void sound_control_w(u8 data);

	// misc handlers
	void adjust_cycles(u8 data);
	void mcu_control_w(u8 data);
	u8 mcu_io_r(offs_t offset);
	void mcu_io_w(offs_t offset, u8 data);
	u8 nob_mcu_latch_r();
	void nob_mcu_latch_w(u8 data);
	void nob_mcu_status_w(u8 data);
	void nob_mcu_control_p2_w(u8 data);
	u8 nob_maincpu_latch_r();
	void nob_maincpu_latch_w(u8 data);
	u8 nob_mcu_status_r();
	u8 nobb_inport1c_r();
	u8 nobb_inport22_r();
	u8 nobb_inport23_r();
	void nobb_outport24_w(u8 data);
	u8 nob_start_r();

	// video functions
	TILE_GET_INFO_MEMBER(tile_get_info);
	void system1_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(system2);
	DECLARE_MACHINE_START(myherok);
	u32 screen_update_system1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_system2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_system2_rowscroll(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void video_start_common(int pagecount);
	inline void videoram_wait_states(cpu_device *cpu);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset);
	void video_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind16 &fgpixmap, bitmap_ind16 **bgpixmaps, const int *bgrowscroll, int bgyscroll, int spritexoffs);

	// misc functions
	TIMER_DEVICE_CALLBACK_MEMBER(soundirq_gen);
	IRQ_CALLBACK_MEMBER(mcu_t0_callback);
	void bank44_custom_w(u8 data);
	void bank0c_custom_w(u8 data);

	// devices
	required_device<z80_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device_array<sn76489a_device, 2> m_sn;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<i8255_device> m_ppi8255;
	optional_device<z80pio_device> m_pio;

	// shared pointers
	required_shared_ptr<u8> m_ram;
	required_shared_ptr<u8> m_spriteram;
	required_shared_ptr<u8> m_paletteram;
	optional_shared_ptr<u8> m_decrypted_opcodes;

	// memory regions
	required_memory_region m_maincpu_region;
	required_region_ptr<u8> m_spriterom;
	required_region_ptr<u8> m_lookup_prom;
	optional_region_ptr<u8> m_color_prom;

	// banks
	required_memory_bank m_bank1;
	optional_memory_bank m_bank0d;
	optional_memory_bank m_bank1d;
	std::unique_ptr<u8[]> m_banked_decrypted_opcodes;

	// address maps
	void banked_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void encrypted_sys1ppi_maps(machine_config &config);
	void encrypted_sys1pio_maps(machine_config &config);
	void encrypted_sys2_mc8123_maps(machine_config &config);
	void mcu_io_map(address_map &map) ATTR_COLD;
	void nobo_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void system1_map(address_map &map) ATTR_COLD;
	void blockgal_io_map(address_map &map) ATTR_COLD;
	void system1_pio_io_map(address_map &map) ATTR_COLD;
	void system1_ppi_io_map(address_map &map) ATTR_COLD;
};

class shtngmst_state : public system1_state
{
public:
	shtngmst_state(const machine_config &mconfig, device_type type, const char *tag) :
		system1_state(mconfig, type, tag),
		m_gun_solenoid(*this, "gun_solenoid")
	{ }

	void shtngmst(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(gun_trigger);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	output_finder<> m_gun_solenoid;

	u8 m_gun_output = 0;
	u8 m_gun_trigger = 0;

	void shtngmst_io_map(address_map &map) ATTR_COLD;

	u8 gun_output_r() { return m_gun_output; }
	void gun_output_w(u8 data);
	u8 gun_trigger_r();
};

class dakkochn_state : public system1_state
{
public:
	dakkochn_state(const machine_config &mconfig, device_type type, const char *tag) :
		system1_state(mconfig, type, tag),
		m_keys(*this, "KEY%u", 0)
	{ }

	ioport_value mux_data_r();
	ioport_value mux_status_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void videomode_w(u8 data) override;

private:
	required_ioport_array<7> m_keys;

	u8 m_mux_count = 0;
	u8 m_mux_clock = 0;
};

#endif // MAME_SEGA_SYSTEM1_H
