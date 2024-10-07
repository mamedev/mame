// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 16A/16B/18/Outrun/Hang On/X-Board/Y-Board hardware

***************************************************************************/
#ifndef MAME_SEGA_SEGAS18_H
#define MAME_SEGA_SEGAS18_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "315_5195.h"
#include "315_5296.h"
#include "machine/i8255.h"
#include "machine/msm6253.h"
#include "machine/nvram.h"
#include "machine/upd4701.h"
#include "video/315_5313.h"
#include "segaic16.h"
#include "sega16sp.h"
#include "screen.h"


// ======================> segas18_state

class segas18_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segas18_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag)
		, m_mapper(*this, "mapper")
		, m_maincpu(*this, "maincpu")
		, m_maincpu_region(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_mcu(*this, "mcu")
		, m_vdp(*this, "gen_vdp")
		, m_io(*this, "io")
		, m_nvram(*this, "nvram")
		, m_screen(*this, "screen")
		, m_sprites(*this, "sprites")
		, m_segaic16vid(*this, "segaic16vid")
		, m_gfxdecode(*this, "gfxdecode")
		, m_ppi(*this, "ppi")
		, m_upd4701(*this, "upd%u", 1U)
		, m_lghost_adc(*this, "adc")
		, m_special_ports(*this, {"SERVICE", "COINAGE"})
		, m_lghost_gunx(*this, "GUNX%u", 1U)
		, m_lghost_guny(*this, "GUNY%u", 1U)
		, m_lghost_fake(*this, "FAKE")
		, m_workram(*this, "workram")
		, m_sprites_region(*this, "sprites")
		, m_soundbank(*this, "soundbank")
		, m_gun_recoil(*this, "P%u_Gun_Recoil", 1U)
		, m_romboard(ROM_BOARD_INVALID)
		, m_custom_io_r(*this)
		, m_custom_io_w(*this)
		, m_grayscale_enable(0)
		, m_vdp_enable(0)
		, m_vdp_mixing(0)
		, m_lghost_select(0)
	{
	}

	void wwally(machine_config &config);
	void system18(machine_config &config);
	void lghost_fd1094(machine_config &config);
	void wwally_fd1094(machine_config &config);
	void system18_fd1094(machine_config &config);
	void system18_fd1094_i8751(machine_config &config);
	void lghost(machine_config &config);
	void system18_i8751(machine_config &config);
	void ddcrew4p(machine_config &config);
	void ddcrew4p_fd1094(machine_config &config);

	// driver init
	void init_ddcrew4p();
	void init_lghost();
	void init_generic_shad();
	void init_generic_5874();
	void init_wwally();
	void init_generic_5987();
	void init_hamaway();

private:
	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, uint8_t index);

	// read/write handlers
	void rom_5874_bank_w(uint8_t data);
	void rom_5987_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void rom_837_7525_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void misc_outputs_w(uint8_t data);
	uint16_t misc_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void misc_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void soundbank_w(uint8_t data);

	// custom I/O
	uint16_t ddcrew_custom_io_r(offs_t offset);
	void ddcrew_custom_io_w(offs_t offset, uint16_t data);
	uint16_t lghost_custom_io_r(offs_t offset);
	void lghost_gun_recoil_w(uint8_t data);
	void lghost_custom_io_w(offs_t offset, uint16_t data);
	uint16_t wwally_custom_io_r(offs_t offset);
	void wwally_custom_io_w(offs_t offset, uint16_t data);

	ioport_value lghost_y1_r();
	ioport_value lghost_x1_r();
	ioport_value lghost_y2_r();
	ioport_value lghost_x2_r();

	// video rendering
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vdp_sndirqline_callback_s18(int state);
	void vdp_lv6irqline_callback_s18(int state);
	void vdp_lv4irqline_callback_s18(int state);

	uint16_t genesis_vdp_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0) { return m_vdp->vdp_r(offset, mem_mask); }
	void genesis_vdp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_vdp->vdp_w(offset, data, mem_mask); }
	void tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_segaic16vid->tileram_w(offset, data, mem_mask); }
	void textram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_segaic16vid->textram_w(offset, data, mem_mask); }

	void set_grayscale(int state);
	void set_vdp_enable(int state);

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void mcu_io_map(address_map &map) ATTR_COLD;
	void pcm_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
	void system18_map(address_map &map) ATTR_COLD;

	// rom board types
	enum segas18_rom_board
	{
		ROM_BOARD_INVALID,
		ROM_BOARD_171_SHADOW,   // 171-???? -- used by shadow dancer
		ROM_BOARD_171_5874,     // 171-5874
		ROM_BOARD_171_5987,     // 171-5987
		ROM_BOARD_837_7525      // Hammer Away proto
	};

	// device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// internal helpers
	TIMER_CALLBACK_MEMBER(initial_boost);
	void init_generic(segas18_rom_board rom_board);
	void set_vdp_mixing(uint8_t mixing);
	void draw_vdp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	required_memory_region m_maincpu_region;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<sega315_5313_device> m_vdp;
	required_device<sega_315_5296_device> m_io;
	required_device<nvram_device> m_nvram;
	required_device<screen_device> m_screen;
	required_device<sega_sys16b_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<i8255_device> m_ppi;
	optional_device_array<upd4701_device, 3> m_upd4701;
	optional_device<msm6253_device> m_lghost_adc;

	required_ioport_array<2> m_special_ports;
	optional_ioport_array<3> m_lghost_gunx;
	optional_ioport_array<3> m_lghost_guny;
	optional_ioport m_lghost_fake;

	// memory pointers
	required_shared_ptr<uint16_t> m_workram;

	required_memory_region m_sprites_region;
	optional_memory_bank m_soundbank;

	output_finder<3> m_gun_recoil;

	// configuration
	segas18_rom_board   m_romboard;
	read16sm_delegate   m_custom_io_r;
	write16sm_delegate  m_custom_io_w;

	// internal state
	emu_timer *         m_init_boost_timer;
	int                 m_grayscale_enable;
	int                 m_vdp_enable;
	uint8_t             m_vdp_mixing;
	bitmap_ind16        m_temp_bitmap;

	// game-specific state
	uint8_t             m_lghost_select;
};

#endif // MAME_SEGA_SEGAS18_H
