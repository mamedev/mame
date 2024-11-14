// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 16B hardware

***************************************************************************/
#ifndef MAME_SEGA_SEGAS16B_H
#define MAME_SEGA_SEGAS16B_H

#pragma once

#include "315_5195.h"
#include "segaic16_m.h"
#include "segaic16.h"
#include "sega16sp.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/cxd1095.h"
#include "machine/gen_latch.h"
#include "machine/msm6253.h"
#include "machine/nvram.h"
#include "machine/upd4701.h"
#include "sound/dac.h"
#include "sound/upd7759.h"
#include "sound/ymopm.h"
#include "sound/ymopl.h"

#include "screen.h"

INPUT_PORTS_EXTERN( system16b_generic );

// ======================> segas16b_state

class segas16b_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segas16b_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag)
		, m_mapper(*this, "mapper")
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_mcu(*this, "mcu")
		, m_ym2151(*this, "ym2151")
		, m_ym2413(*this, "ym2413")
		, m_upd7759(*this, "upd")
		, m_multiplier(*this, "multiplier")
		, m_cmptimer_1(*this, "cmptimer_1")
		, m_cmptimer_2(*this, "cmptimer_2")
		, m_nvram(*this, "nvram")
		, m_screen(*this, "screen")
		, m_sprites(*this, "sprites")
		, m_segaic16vid(*this, "segaic16vid")
		, m_soundlatch(*this, "soundlatch")
		, m_cxdio(*this, "cxdio")
		, m_upd4701a(*this, "upd4701a%u", 1U)
		, m_adc(*this, "adc")
		, m_workram(*this, "workram")
		, m_i8751_sync_timer(nullptr)
		, m_romboard(ROM_BOARD_INVALID)
		, m_tilemap_type(segaic16_video_device::TILEMAP_16B)
		, m_custom_io_r(*this)
		, m_custom_io_w(*this)
		, m_disable_screen_blanking(false)
		, m_i8751_initial_config(nullptr)
		, m_atomicp_sound_divisor(0)
		, m_atomicp_sound_count(0)
		, m_hwc_left_limit(*this, "LEFT_LIMIT")
		, m_hwc_right_limit(*this, "RIGHT_LIMIT")
		, m_mj_input_num(0)
		, m_mj_last_val(0)
		, m_mj_inputs(*this, "MJ%u", 0U)
		, m_spritepalbase(0x400)
		, m_gfxdecode(*this, "gfxdecode")
		, m_sound_decrypted_opcodes(*this, "sound_decrypted_opcodes")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_bootleg_scroll(*this, "bootleg_scroll")
		, m_bootleg_page(*this, "bootleg_page")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void rom_5797_fragment(machine_config &config);
	void system16b_fd1094_5797(machine_config &config);
	void fpointbla(machine_config &config);
	void atomicp(machine_config &config);
	void aceattacb_fd1094(machine_config &config);
	void hwchamp(machine_config &config);
	void hwchamp_fd1094(machine_config &config);
	void system16b_i8751(machine_config &config);
	void system16c(machine_config &config);
	void system16b_mc8123(machine_config &config);
	void system16b_i8751_5797(machine_config &config);
	void system16b_fd1089a(machine_config &config);
	void system16b_5797(machine_config &config);
	void system16b_split(machine_config &config);
	void system16b_fd1089b(machine_config &config);
	void system16b(machine_config &config);
	void system16b_fd1094(machine_config &config);
	void fpointbl(machine_config &config);
	void lockonph(machine_config &config);

	// ROM board-specific driver init
	void init_generic_5521();
	void init_generic_5358();
	void init_generic_5704();
	void init_generic_5358_small();
	void init_generic_5797();
	void init_generic_korean();
	void init_generic_bootleg();
	void init_lockonph();
	// game-specific driver init
	void init_isgsm();
	void init_tturf_5704();
	void init_wb3_5704();
	void init_hwchamp_5521();
	void init_sdi_5358_small();
	void init_fpointbla();
	void init_snapper();
	void init_shinobi4_5521();
	void init_defense_5358_small();
	void init_sjryuko_5358_small();
	void init_exctleag_5358();
	void init_tetrbx();
	void init_aceattac_5358();
	void init_passshtj_5358();
	void init_cencourt_5358();
	void init_shinfz();
	void init_dunkshot_5358_small();
	void init_timescan_5358_small();
	void init_shinobi3_5358();
	void init_altbeas4_5521();
	void init_aliensyn7_5358_small();

	DECLARE_INPUT_CHANGED_MEMBER(handy_w);

protected:
	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, uint8_t index);

	// main CPU read/write handlers
	void rom_5704_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rom_5797_bank_math_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void rom_5797_bank_math_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t unknown_rgn2_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void unknown_rgn2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t standard_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void standard_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void atomicp_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t bootleg_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void bootleg_custom_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// sound CPU read/write handlers
	void upd7759_control_w(uint8_t data);
	uint8_t upd7759_status_r();
	void sound_w16(uint16_t data);

	// other callbacks
	void upd7759_generate_nmi(int state);
	INTERRUPT_GEN_MEMBER( i8751_main_cpu_vblank );
	void spin_68k_w(uint8_t data);

	// video updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_segaic16vid->tileram_w(offset,data,mem_mask); }
	void textram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_segaic16vid->textram_w(offset,data,mem_mask); }

	// bootleg stuff
	void tilemap_16b_fpointbl_fill_latch(int i, uint16_t* latched_pageselect, uint16_t* latched_yscroll, uint16_t* latched_xscroll, uint16_t* textram);

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map_fpointbla(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map_x(address_map &map) ATTR_COLD;
	void fpointbl_map(address_map &map) ATTR_COLD;
	void fpointbl_sound_map(address_map &map) ATTR_COLD;
	void lockonph_map(address_map &map) ATTR_COLD;
	void lockonph_sound_iomap(address_map &map) ATTR_COLD;
	void lockonph_sound_map(address_map &map) ATTR_COLD;
	void map_fpointbla(address_map &map) ATTR_COLD;
	void mcu_io_map(address_map &map) ATTR_COLD;
	void sound_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
	void bootleg_sound_map(address_map &map) ATTR_COLD;
	void bootleg_sound_portmap(address_map &map) ATTR_COLD;
	void system16b_bootleg_map(address_map &map) ATTR_COLD;
	void system16b_map(address_map &map) ATTR_COLD;
	void system16c_map(address_map &map) ATTR_COLD;

	// internal types
	typedef delegate<void ()> i8751_sim_delegate;

	// rom board types
	enum segas16b_rom_board
	{
		ROM_BOARD_INVALID,
		ROM_BOARD_171_5358_SMALL,       // 171-5358 with smaller ROMs
		ROM_BOARD_171_5358,             // 171-5358
		ROM_BOARD_171_5521,             // 171-5521
		ROM_BOARD_171_5704,             // 171-5704 - don't know any diff between this and 171-5521
		ROM_BOARD_171_5797,             // 171-5797
		ROM_BOARD_KOREAN                // (custom Korean)
	};

	// device overrides
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(i8751_sync);
	TIMER_CALLBACK_MEMBER(atomicp_sound_irq);

	// internal helpers
	void init_generic(segas16b_rom_board rom_board);

	// i8751 simulations
	void tturf_i8751_sim();
	void wb3_i8751_sim();

	// custom I/O handlers
	uint16_t aceattac_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void aceattac_custom_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dunkshot_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	uint16_t hwchamp_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void hwchamp_custom_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t passshtj_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	uint16_t sdi_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	uint16_t sjryuko_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void sjryuko_custom_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// devices
	optional_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	optional_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	optional_device<ym2151_device> m_ym2151;
	optional_device<ym2413_device> m_ym2413;
	optional_device<upd7759_device> m_upd7759;
	optional_device<sega_315_5248_multiplier_device> m_multiplier;
	optional_device<sega_315_5250_compare_timer_device> m_cmptimer_1;
	optional_device<sega_315_5250_compare_timer_device> m_cmptimer_2;
	required_device<nvram_device> m_nvram;
	required_device<screen_device> m_screen;
	optional_device<sega_sys16b_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	optional_device<generic_latch_8_device> m_soundlatch; // not for atomicp
	optional_device<cxd1095_device> m_cxdio; // for aceattac
	optional_device_array<upd4701_device, 2> m_upd4701a; // for aceattac
	optional_device<msm6253_device> m_adc; // for hwchamp

	// memory pointers
	required_shared_ptr<uint16_t> m_workram;

	// timers
	emu_timer *         m_i8751_sync_timer;

	// configuration
	segas16b_rom_board  m_romboard;
	int                 m_tilemap_type;
	read16_delegate     m_custom_io_r;
	write16_delegate    m_custom_io_w;
	bool                m_disable_screen_blanking;
	const uint8_t *     m_i8751_initial_config;
	i8751_sim_delegate  m_i8751_vblank_hook;
	uint8_t             m_atomicp_sound_divisor;

	// game-specific state
	uint8_t             m_atomicp_sound_count;
	optional_ioport     m_hwc_left_limit;
	optional_ioport     m_hwc_right_limit;
	uint8_t             m_mj_input_num;
	uint8_t             m_mj_last_val;
	optional_ioport_array<6> m_mj_inputs;
	int                 m_spritepalbase;

	required_device<gfxdecode_device> m_gfxdecode;
	optional_shared_ptr<uint8_t> m_sound_decrypted_opcodes;
	optional_shared_ptr<uint16_t> m_decrypted_opcodes;
	optional_shared_ptr<uint16_t> m_bootleg_scroll;
	optional_shared_ptr<uint16_t> m_bootleg_page;
	output_finder<2> m_lamps;
};

class dfjail_state : public segas16b_state
{
public:
	// construction/destruction
	dfjail_state(const machine_config &mconfig, device_type type, const char *tag)
		: segas16b_state(mconfig, type, tag)
		, m_nmi_enable(false)
		, m_dac_data(0)
		, m_dac(*this, "dac")
	{ }

	void dfjail(machine_config &config);

protected:
	void sound_control_w(uint8_t data);
	void dac_data_w(offs_t offset, uint8_t data);
	INTERRUPT_GEN_MEMBER( soundirq_cb );
	bool m_nmi_enable;
	uint16_t m_dac_data;

	void dfjail_map(address_map &map) ATTR_COLD;
	void dfjail_sound_iomap(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<dac_word_interface> m_dac;
};

class afighter_16b_analog_state : public segas16b_state
{
public:
	// construction/destruction
	afighter_16b_analog_state(const machine_config &mconfig, device_type type, const char *tag)
		: segas16b_state(mconfig, type, tag)
		, m_accel(*this, "ACCEL")
		, m_steer(*this, "STEER")
	{ }

	ioport_value afighter_accel_r();
	ioport_value afighter_handl_left_r();
	ioport_value afighter_handl_right_r();

private:
	required_ioport     m_accel;
	required_ioport     m_steer;
};

#endif // MAME_SEGA_SEGAS16B_H
