// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
/******************************************
 *
 * NEC PC-9821
 *
 ******************************************/

#ifndef MAME_NEC_PC9821_H
#define MAME_NEC_PC9821_H

#pragma once

#include "pc9801.h"

class pc9821_state : public pc9801bx_state
{
public:
	pc9821_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9801bx_state(mconfig, type, tag)
		, m_ext_gvram(*this, "ext_gvram")
		, m_pegc_mmio_view(*this, "pegc_mmio_view")
	{
	}

	void pc9821(machine_config &config);

protected:
	void pc9821_io(address_map &map) ATTR_COLD;
	void pc9821_map(address_map &map) ATTR_COLD;

	DECLARE_MACHINE_START(pc9821);
	DECLARE_MACHINE_RESET(pc9821);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	TIMER_CALLBACK_MEMBER(pit_delay);

private:
	required_shared_ptr<uint32_t> m_ext_gvram;
	memory_view m_pegc_mmio_view;

	uint16_t pc9821_grcg_gvram_r(offs_t offset, uint16_t mem_mask = ~0);
	void pc9821_grcg_gvram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pc9821_video_ff_w(offs_t offset, uint8_t data);
	uint8_t pc9821_a0_r(offs_t offset);
	void pc9821_a0_w(offs_t offset, uint8_t data);
	uint8_t window_bank_r(offs_t offset);
	void window_bank_w(offs_t offset, uint8_t data);
	uint8_t ext2_video_ff_r();
	void ext2_video_ff_w(uint8_t data);
	void pc9821_mode_ff_w(u8 data);
	void pit_latch_delay(offs_t offset, uint8_t data);

	uint8_t m_pit_latch_cmd = 0;
	uint8_t m_pc9821_window_bank = 0;
	uint8_t m_ext2_ff = 0;

	emu_timer *m_pit_delay = nullptr;

	struct {
		uint8_t pal_entry = 0;
		uint8_t r[0x100]{}, g[0x100]{}, b[0x100]{};
		uint16_t bank[2]{};
		bool packed_mode = false;
	}m_pegc;

	void pc9821_egc_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void pegc_mmio_map(address_map &map);

//  UPD7220_DISPLAY_PIXELS_MEMBER( pegc_display_pixels );
};

// MATE A

class pc9821_mate_a_state : public pc9821_state
{
public:
	pc9821_mate_a_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
		, m_bios_view(*this, "bios_view")
	{
	}

	void pc9821as(machine_config &config);
	void pc9821ap2(machine_config &config);

protected:
	void pc9821as_map(address_map &map) ATTR_COLD;
	void pc9821as_io(address_map &map) ATTR_COLD;

	virtual void itf_43d_bank_w(offs_t offset, uint8_t data) override;
	virtual void cbus_43f_bank_w(offs_t offset, uint8_t data) override;

private:
	DECLARE_MACHINE_START(pc9821ap2);
	DECLARE_MACHINE_RESET(pc9821ap2);

	// Starting from Af
	memory_view m_bios_view;

	// Ap, As, Ae only
	u8 ext_sdip_data_r(offs_t offset);
	void ext_sdip_data_w(offs_t offset, u8 data);
	void ext_sdip_address_w(offs_t offset, u8 data);
	void ext_sdip_access_w(offs_t offset, u8 data);

	uint8_t m_ext_sdip[0x100]{}, m_ext_sdip_addr = 0;
};

// CanBe

class pc9821_canbe_state : public pc9821_state
{
public:
	pc9821_canbe_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
		, m_bios_view(*this, "bios_view")
	{
	}

	void pc9821ce(machine_config &config);
//  void pc9821ce2(machine_config &config);
	void pc9821cx3(machine_config &config);

protected:
	void pc9821ce_map(address_map &map) ATTR_COLD;
	void pc9821ce_io(address_map &map) ATTR_COLD;

	void pc9821cx3_map(address_map &map) ATTR_COLD;
	void pc9821cx3_io(address_map &map) ATTR_COLD;

	virtual void itf_43d_bank_w(offs_t offset, uint8_t data) override;
	virtual void cbus_43f_bank_w(offs_t offset, uint8_t data) override;

private:
	memory_view m_bios_view;

	DECLARE_MACHINE_START(pc9821_canbe);
	DECLARE_MACHINE_RESET(pc9821_canbe);

};

// class pc9821_cereb_state : public pc9821_canbe_state

// Mate B

// class pc9821_mate_b_state : public pc9821_state

// Mate X (NB: should be subclass of Mate B)

class pc9821_mate_x_state : public pc9821_state
{
public:
	pc9821_mate_x_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
	{
	}

	void pc9821xa16(machine_config &config);
	void pc9821xv13(machine_config &config);
//  void pc9821xs(machine_config &config);
};

// Mate R

class pc9821_mate_r_state : public pc9821_mate_x_state
{
public:
	pc9821_mate_r_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_mate_x_state(mconfig, type, tag)
	{
	}

	void pc9821ra20(machine_config &config);
	void pc9821ra266(machine_config &config);
	void pc9821ra333(machine_config &config);
};

// VLSI Supercore594 (Wildcat) or Intel 430FX (Triton) PCI 2.0
// V166 / V200 / V233 uses an Intel 430VX PCI 2.1
// https://www.pc-9800.net/db_98/data/pc-9821v13.htm
// https://www.pc-9800.net/db_98/data/pc-9821v20.htm
//class pc9821_valuestar_state : public pc9821_mate_x_state
//{
//public:
//  pc9821_valuestar_state(const machine_config &mconfig, device_type type, const char *tag)
//      : pc9821_mate_x_state(mconfig, type, tag)
//  {
//  }
//
//  void pc9821v13(machine_config &config);
//  void pc9821v20(machine_config &config);
//};

// 9821NOTE

// https://www.pc-9800.net/db_98/data/pc-9821ne.htm
// https://www.pc-9800.net/db_98/data/pc-9821ne2.htm
//class pc9821_note_state : public pc9821_state
//{
//public:
//  pc9821_note_state(const machine_config &mconfig, device_type type, const char *tag)
//      : pc9821_state(mconfig, type, tag)
//      , m_pmc(*this, "pmc")
//  {
//  }
//
//  void pc9821ne(machine_config &config);
//
//protected:
//  void pc9821ne_io(address_map &map) ATTR_COLD;
//
//private:
//  required_device<redwood1_device> m_pmc;
//};

class pc9821_note_lavie_state : public pc9821_state
{
public:
	pc9821_note_lavie_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
	{
	}

	void pc9821nr15(machine_config &config);
	void pc9821nr166(machine_config &config);
	void pc9821nw150(machine_config &config);
};


#endif // MAME_NEC_PC9821_H
