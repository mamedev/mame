// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
#ifndef MAME_INCLUDES_SATURN_H
#define MAME_INCLUDES_SATURN_H

#pragma once

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "cpu/m68000/m68000.h"
#include "cpu/sh/sh2.h"

#include "debug/debugcon.h"
#include "debug/debugcmd.h"

#include "machine/315-5881_crypt.h"
#include "machine/315-5838_317-0229_comp.h"
#include "machine/sega_scu.h"
#include "machine/smpc.h"
#include "machine/timer.h"

#include "video/segasaturn_vdp1.h"
#include "video/segasaturn_vdp2.h"

#include "sound/scsp.h"

#include "debugger.h"
#include "emupal.h"
#include "screen.h"

class saturn_state : public driver_device
{
public:
	saturn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rom(*this, "bios")
		, m_workram_l(*this, "workram_l")
		, m_workram_h(*this, "workram_h")
		, m_sound_ram(*this, "sound_ram")
		, m_fake_comms(*this, "fake")
		, m_maincpu(*this, "maincpu")
		, m_slave(*this, "slave")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp1(*this, "vdp1")
		, m_vdp2(*this, "vdp2")
		, m_scsp(*this, "scsp")
		, m_smpc_hle(*this, "smpc")
		, m_scu(*this, "scu")
		//, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
	{
	}

	void scsp_irq(offs_t offset, uint8_t data);

	// SMPC HLE delegates
	DECLARE_WRITE_LINE_MEMBER(master_sh2_reset_w);
	DECLARE_WRITE_LINE_MEMBER(master_sh2_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(slave_sh2_reset_w);
	DECLARE_WRITE_LINE_MEMBER(sound_68k_reset_w);
	DECLARE_WRITE_LINE_MEMBER(system_reset_w);
	DECLARE_WRITE_LINE_MEMBER(system_halt_w);
	DECLARE_WRITE_LINE_MEMBER(dot_select_w);

	DECLARE_WRITE_LINE_MEMBER(m68k_reset_callback);

protected:
	required_region_ptr<uint32_t> m_rom;
	required_shared_ptr<uint32_t> m_workram_l;
	required_shared_ptr<uint32_t> m_workram_h;
	required_shared_ptr<uint16_t> m_sound_ram;
	optional_ioport m_fake_comms;

	memory_region *m_cart_reg[4];
	std::unique_ptr<uint8_t[]>     m_backupram;

	uint8_t     m_en_68k;

	int       m_minit_boost;
	int       m_sinit_boost;
	attotime  m_minit_boost_timeslice;
	attotime  m_sinit_boost_timeslice;

	required_device<sh2_device> m_maincpu;
	required_device<sh2_device> m_slave;
	required_device<m68000_base_device> m_audiocpu;
	required_device<saturn_vdp1_device> m_vdp1;
	required_device<saturn_vdp2_device> m_vdp2;
	required_device<scsp_device> m_scsp;
	required_device<smpc_hle_device> m_smpc_hle;
	required_device<sega_scu_device> m_scu;
//	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;

	DECLARE_VIDEO_START(stv_vdp2);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(saturn_slave_scanline);


	void saturn_soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t saturn_soundram_r(offs_t offset);
	void minit_w(uint32_t data);
	void sinit_w(uint32_t data);
	void saturn_minit_w(uint32_t data);
	void saturn_sinit_w(uint32_t data);
	uint8_t saturn_backupram_r(offs_t offset);
	void saturn_backupram_w(offs_t offset, uint8_t data);

	int m_scsp_last_line;

//  DECLARE_WRITE_LINE_MEMBER(scudsp_end_w);
//  uint16_t scudsp_dma_r(offs_t offset);
//  void scudsp_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

//  void debug_scudma_command(int ref, const std::vector<std::string> &params);
//  void debug_scuirq_command(int ref, const std::vector<std::string> &params);
//  void debug_help_command(int ref, const std::vector<std::string> &params);
//  void debug_commands(int ref, const std::vector<std::string> &params);
};


// These two clocks are synthesized by the 315-5746
#define MASTER_CLOCK_352 XTAL(14'318'181)*4
#define MASTER_CLOCK_320 XTAL(14'318'181)*3.75

extern gfx_decode_entry const gfx_stv[];

#endif // MAME_INCLUDES_SATURN_H
