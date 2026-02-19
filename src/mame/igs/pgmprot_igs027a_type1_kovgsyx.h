// license:BSD-3-Clause
// copyright-holders:eziochiu
// Derived state class for kovgsyx / kovzscs bootleg protection
// using NXP LPC2132 ARM7 with VIC for I2C-based handshake

#ifndef MAME_IGS_PGMPROT_IGS027A_TYPE1_KOVGSYX_H
#define MAME_IGS_PGMPROT_IGS027A_TYPE1_KOVGSYX_H

#pragma once

#include "pgmprot_igs027a_type1.h"
#include "machine/lpc2132_vic.h"

class pgm_arm_type1_kovgsyx_state : public pgm_arm_type1_state
{
public:
	pgm_arm_type1_kovgsyx_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_arm_type1_state(mconfig, type, tag)
		, m_lpc2132_vic(*this, "lpc2132_vic")
	{
	}

	void init_kovgsyx();
	void init_kovzscs();

	void pgm_arm_type1_kovgsyx(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<lpc2132_vic_device> m_lpc2132_vic;

	// 68k <-> ARM latch registers
	u16 m_kovgsyx_highlatch_arm_w = 0;
	u16 m_kovgsyx_lowlatch_arm_w = 0;
	u16 m_kovgsyx_highlatch_68k_w = 0;
	u16 m_kovgsyx_lowlatch_68k_w = 0;

	// LPC2132 PLL state
	u32 m_kovgsyx_pll_lock_timer = 0;
	u8  m_kovgsyx_pll_enabled = 0;
	u8  m_kovgsyx_pll_config = 0;

	// LPC2132 I2C peripheral state
	u32 m_kovgsyx_i2c1sclh = 0;
	u32 m_kovgsyx_i2c1conset = 0;
	u32 m_kovgsyx_i2c1dat = 0;
	u32 m_kovgsyx_i2c1_status = 0;
	u8  m_kovgsyx_handshake_done = 0;

	// 68k-side ASIC27A handlers
	void kovgsyx_asic27a_write_word(offs_t offset, u16 data);
	void kovgsyx_asic27a_write_sync(s32 param);
	u16 kovgsyx_asic27a_read_word(offs_t offset);

	// ARM-side LPC2132 peripheral handlers
	u32 kovgsyx_lpc2132_read_long(offs_t offset);
	void kovgsyx_lpc2132_write_long(offs_t offset, u32 data, u32 mem_mask = ~0);

	// VIC default vector address callback for handshake detection
	void kovgsyx_handshake_callback(u32 data);

	// ROM decode helpers
	void pgm_decode_kovgsyx_samples();
	void pgm_decode_kovgsyx_program();
	void pgm_decode_kovzscs_program();
	void kovgsyx_common_init();

	// address maps
	void kovgsyx_map(address_map &map) ATTR_COLD;
	void kovgsyx_arm7_map(address_map &map) ATTR_COLD;
};

#endif // MAME_IGS_PGMPROT_IGS027A_TYPE1_KOVGSYX_H
