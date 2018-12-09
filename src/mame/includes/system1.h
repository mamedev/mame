// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski, Nicola Salmoria, Mirko Buffoni
#ifndef MAME_INCLUDES_SYSTEM1_H
#define MAME_INCLUDES_SYSTEM1_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/segacrp2_device.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"

class system1_state : public driver_device
{
public:
	system1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ppi8255(*this, "ppi8255"),
		m_pio(*this, "pio"),
		m_ram(*this, "ram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "palette"),
		m_videomode_custom(nullptr),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu_region(*this, "maincpu"),
		m_color_prom(*this, "palette"),
		m_bank1(*this, "bank1"),
		m_bank0d(*this, "bank0d"),
		m_bank1d(*this, "bank1d"),
		m_banked_decrypted_opcodes(nullptr)
	{ }

	void sys1ppix_315_5051(machine_config &config);
	void sys1ppisx_315_5064(machine_config &config);
	void sys2_317_0007(machine_config &config);
	void sys1piox_315_5110(machine_config &config);
	void sys1piox_315_5065(machine_config &config);
	void sys2m(machine_config &config);
	void sys1ppix_315_5178(machine_config &config);
	void sys1ppix_315_5179(machine_config &config);
	void sys1piox_315_5093(machine_config &config);
	void sys2_315_5176(machine_config &config);
	void sys2(machine_config &config);
	void sys2_315_5177(machine_config &config);
	void nob(machine_config &config);
	void sys1ppisx_315_5041(machine_config &config);
	void sys1piox_315_5132(machine_config &config);
	void sys1piox_315_5162(machine_config &config);
	void sys1piox_315_5133(machine_config &config);
	void sys1pioxb(machine_config &config);
	void sys1ppi(machine_config &config);
	void sys1piox_315_5135(machine_config &config);
	void sys2rowxboot(machine_config &config);
	void sys1piox_315_5102(machine_config &config);
	void sys1piosx_315_spat(machine_config &config);
	void sys2x(machine_config &config);
	void sys1piox_315_5051(machine_config &config);
	void sys1piox_315_5098(machine_config &config);
	void sys1piosx_315_5099(machine_config &config);
	void sys2xboot(machine_config &config);
	void sys2xb(machine_config &config);
	void nobm(machine_config &config);
	void mcu(machine_config &config);
	void sys2_317_0006(machine_config &config);
	void sys1piox_317_0006(machine_config &config);
	void sys1ppix_315_5033(machine_config &config);
	void sys1pio(machine_config &config);
	void sys1pios(machine_config &config);
	void sys2rowm(machine_config &config);
	void sys1ppix_315_5098(machine_config &config);
	void sys1ppix_315_5048(machine_config &config);
	void sys2row(machine_config &config);
	void sys1ppis(machine_config &config);
	void sys1ppix_315_5065(machine_config &config);
	void sys1piox_315_5177(machine_config &config);
	void sys1piox_315_5155(machine_config &config);
	void sys2rowxb(machine_config &config);

	void init_bank00();
	void init_bank0c();
	void init_bank44();

	void init_nobb();
	void init_dakkochn();
	void init_bootleg();
	void init_shtngmst();
	void init_blockgal();
	void init_nob();
	void init_myherok();
	void init_ufosensi();
	void init_wbml();
	void init_tokisens();
	void init_bootsys2();
	void init_bootsys2d();
	void init_choplift();

	DECLARE_CUSTOM_INPUT_MEMBER(dakkochn_mux_data_r);
	DECLARE_CUSTOM_INPUT_MEMBER(dakkochn_mux_status_r);

private:
	optional_device<i8255_device>  m_ppi8255;
	optional_device<z80pio_device>  m_pio;
	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;

	std::unique_ptr<uint8_t[]> m_videoram;
	void (system1_state::*m_videomode_custom)(uint8_t data, uint8_t prevdata);
	uint8_t m_mute_xor;
	uint8_t m_dakkochn_mux_data;
	uint8_t m_videomode_prev;
	uint8_t m_mcu_control;
	uint8_t m_nob_maincpu_latch;
	uint8_t m_nob_mcu_latch;
	uint8_t m_nob_mcu_status;
	int m_nobb_inport23_step;
	std::unique_ptr<uint8_t[]> m_mix_collide;
	uint8_t m_mix_collide_summary;
	std::unique_ptr<uint8_t[]> m_sprite_collide;
	uint8_t m_sprite_collide_summary;
	bitmap_ind16 m_sprite_bitmap;
	uint8_t m_video_mode;
	uint8_t m_videoram_bank;
	tilemap_t *m_tilemap_page[8];
	uint8_t m_tilemap_pages;

	DECLARE_WRITE8_MEMBER(videomode_w);
	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE8_MEMBER(soundport_w);
	DECLARE_WRITE8_MEMBER(mcu_control_w);
	DECLARE_READ8_MEMBER(mcu_io_r);
	DECLARE_WRITE8_MEMBER(mcu_io_w);
	DECLARE_READ8_MEMBER(nob_mcu_latch_r);
	DECLARE_WRITE8_MEMBER(nob_mcu_latch_w);
	DECLARE_WRITE8_MEMBER(nob_mcu_status_w);
	DECLARE_WRITE8_MEMBER(nob_mcu_control_p2_w);
	DECLARE_READ8_MEMBER(nob_maincpu_latch_r);
	DECLARE_WRITE8_MEMBER(nob_maincpu_latch_w);
	DECLARE_READ8_MEMBER(nob_mcu_status_r);
	DECLARE_READ8_MEMBER(nobb_inport1c_r);
	DECLARE_READ8_MEMBER(nobb_inport22_r);
	DECLARE_READ8_MEMBER(nobb_inport23_r);
	DECLARE_WRITE8_MEMBER(nobb_outport24_w);
	DECLARE_READ8_MEMBER(nob_start_r);
	DECLARE_READ8_MEMBER(shtngmst_gunx_r);
	DECLARE_WRITE8_MEMBER(system1_videomode_w);
	DECLARE_READ8_MEMBER(system1_mixer_collision_r);
	DECLARE_WRITE8_MEMBER(system1_mixer_collision_w);
	DECLARE_WRITE8_MEMBER(system1_mixer_collision_reset_w);
	DECLARE_READ8_MEMBER(system1_sprite_collision_r);
	DECLARE_WRITE8_MEMBER(system1_sprite_collision_w);
	DECLARE_WRITE8_MEMBER(system1_sprite_collision_reset_w);
	DECLARE_READ8_MEMBER(system1_videoram_r);
	DECLARE_WRITE8_MEMBER(system1_videoram_w);
	DECLARE_WRITE8_MEMBER(system1_paletteram_w);
	DECLARE_WRITE8_MEMBER(sound_control_w);

	void encrypted_sys1ppi_maps(machine_config &config);
	void encrypted_sys1pio_maps(machine_config &config);
	void encrypted_sys2_mc8123_maps(machine_config &config);

	TILE_GET_INFO_MEMBER(tile_get_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_MACHINE_START(system2);
	DECLARE_VIDEO_START(system2);
	DECLARE_MACHINE_START(myherok);
	uint32_t screen_update_system1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_system2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_system2_rowscroll(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(soundirq_gen);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_t0_callback);
	DECLARE_WRITE8_MEMBER(system1_videoram_bank_w);
	void video_start_common(int pagecount);
	inline void videoram_wait_states(cpu_device *cpu);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset);
	void video_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind16 &fgpixmap, bitmap_ind16 **bgpixmaps, const int *bgrowscroll, int bgyscroll, int spritexoffs);
	void bank44_custom_w(uint8_t data, uint8_t prevdata);
	void bank0c_custom_w(uint8_t data, uint8_t prevdata);
	void dakkochn_custom_w(uint8_t data, uint8_t prevdata);
	required_device<z80_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_memory_region m_maincpu_region;
	optional_region_ptr<uint8_t> m_color_prom;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank0d;
	optional_memory_bank m_bank1d;

	std::unique_ptr<uint8_t[]> m_banked_decrypted_opcodes;

	void banked_decrypted_opcodes_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void mcu_io_map(address_map &map);
	void nobo_map(address_map &map);
	void sound_map(address_map &map);
	void system1_map(address_map &map);
	void system1_pio_io_map(address_map &map);
	void system1_ppi_io_map(address_map &map);
};

#endif // MAME_INCLUDES_SYSTEM1_H
