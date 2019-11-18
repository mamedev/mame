// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 16A/16B/18/Outrun/Hang On/X-Board/Y-Board hardware

***************************************************************************/
#ifndef MAME_INCLUDES_SEGAS18_H
#define MAME_INCLUDES_SEGAS18_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/315_5195.h"
#include "machine/315_5296.h"
#include "machine/nvram.h"
#include "machine/upd4701.h"
#include "video/315_5313.h"
#include "video/segaic16.h"
#include "video/sega16sp.h"
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
		, m_upd4701(*this, "upd%u", 1U)
		, m_workram(*this, "workram")
		, m_sprites_region(*this, "sprites")
		, m_soundbank(*this, "soundbank")
		, m_gun_recoil(*this, "P%u_Gun_Recoil", 1U)
		, m_romboard(ROM_BOARD_INVALID)
		, m_custom_io_r(*this)
		, m_custom_io_w(*this)
		, m_grayscale_enable(false)
		, m_vdp_enable(false)
		, m_vdp_mixing(0)
		, m_lghost_value(0)
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

	// driver init
	void init_ddcrew();
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
	DECLARE_WRITE8_MEMBER( rom_5874_bank_w );
	DECLARE_WRITE16_MEMBER( rom_5987_bank_w );
	DECLARE_WRITE16_MEMBER( rom_837_7525_bank_w );
	DECLARE_WRITE8_MEMBER( misc_outputs_w );
	DECLARE_READ16_MEMBER( misc_io_r );
	DECLARE_WRITE16_MEMBER( misc_io_w );
	DECLARE_WRITE8_MEMBER( soundbank_w );

	// custom I/O
	DECLARE_READ16_MEMBER( ddcrew_custom_io_r );
	DECLARE_READ16_MEMBER( lghost_custom_io_r );
	DECLARE_WRITE8_MEMBER( lghost_gun_recoil_w );
	DECLARE_WRITE16_MEMBER( lghost_custom_io_w );
	DECLARE_READ16_MEMBER( wwally_custom_io_r );
	DECLARE_WRITE16_MEMBER( wwally_custom_io_w );

	// video rendering
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(vdp_sndirqline_callback_s18);
	DECLARE_WRITE_LINE_MEMBER(vdp_lv6irqline_callback_s18);
	DECLARE_WRITE_LINE_MEMBER(vdp_lv4irqline_callback_s18);

	DECLARE_READ16_MEMBER( genesis_vdp_r ) { return m_vdp->vdp_r(offset, mem_mask); }
	DECLARE_WRITE16_MEMBER( genesis_vdp_w ) { m_vdp->vdp_w(offset, data, mem_mask); }
	DECLARE_WRITE16_MEMBER( tileram_w ) { m_segaic16vid->tileram_w(space, offset, data, mem_mask); }
	DECLARE_WRITE16_MEMBER( textram_w ) { m_segaic16vid->textram_w(space, offset, data, mem_mask); }

	DECLARE_WRITE_LINE_MEMBER(set_grayscale);
	DECLARE_WRITE_LINE_MEMBER(set_vdp_enable);

	void decrypted_opcodes_map(address_map &map);
	void mcu_io_map(address_map &map);
	void pcm_map(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
	void system18_map(address_map &map);

	// timer IDs
	enum
	{
		TID_INITIAL_BOOST
	};

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
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// internal helpers
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
	optional_device_array<upd4701_device, 3> m_upd4701;

	// memory pointers
	required_shared_ptr<uint16_t> m_workram;

	required_memory_region m_sprites_region;
	optional_memory_bank m_soundbank;

	output_finder<3> m_gun_recoil;

	// configuration
	segas18_rom_board   m_romboard;
	read16_delegate     m_custom_io_r;
	write16_delegate    m_custom_io_w;

	// internal state
	int                 m_grayscale_enable;
	int                 m_vdp_enable;
	uint8_t               m_vdp_mixing;
	bitmap_ind16        m_temp_bitmap;

	// game-specific state
	uint8_t               m_lghost_value;
	uint8_t               m_lghost_select;
};

#endif // MAME_INCLUDES_SEGAS18_H
