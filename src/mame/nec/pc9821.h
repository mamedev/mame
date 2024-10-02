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
	{
	}

	void pc9821(machine_config &config);

protected:
	void pc9821_io(address_map &map) ATTR_COLD;
	void pc9821_map(address_map &map) ATTR_COLD;

	DECLARE_MACHINE_START(pc9821);
	DECLARE_MACHINE_RESET(pc9821);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

private:
	required_shared_ptr<uint32_t> m_ext_gvram;

	uint16_t pc9821_grcg_gvram_r(offs_t offset, uint16_t mem_mask = ~0);
	void pc9821_grcg_gvram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pc9821_grcg_gvram0_r(offs_t offset, uint16_t mem_mask = ~0);
	void pc9821_grcg_gvram0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pc9821_video_ff_w(offs_t offset, uint8_t data);
	uint8_t pc9821_a0_r(offs_t offset);
	void pc9821_a0_w(offs_t offset, uint8_t data);
	uint8_t window_bank_r(offs_t offset);
	void window_bank_w(offs_t offset, uint8_t data);
	uint8_t ext2_video_ff_r();
	void ext2_video_ff_w(uint8_t data);

	uint8_t m_pc9821_window_bank = 0;
	uint8_t m_ext2_ff = 0;

	struct {
		uint8_t pal_entry = 0;
		uint8_t r[0x100]{}, g[0x100]{}, b[0x100]{};
		uint16_t bank[2]{};
	}m_analog256;

	void pc9821_egc_w(offs_t offset, u16 data, u16 mem_mask = ~0);


//  UPD7220_DISPLAY_PIXELS_MEMBER( pegc_display_pixels );
};

// MATE A

class pc9821_mate_a_state : public pc9821_state
{
public:
	pc9821_mate_a_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
	{
	}

	void pc9821as(machine_config &config);
	void pc9821ap2(machine_config &config);

protected:
	void pc9821as_io(address_map &map) ATTR_COLD;

private:
	DECLARE_MACHINE_START(pc9821ap2);

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
	{
	}

	void pc9821ce2(machine_config &config);
	void pc9821cx3(machine_config &config);

protected:
	void pc9821cx3_map(address_map &map) ATTR_COLD;
	void pc9821cx3_io(address_map &map) ATTR_COLD;

private:
	void remote_addr_w(offs_t offset, u8 data);
	u8 remote_data_r(offs_t offset);
	void remote_data_w(offs_t offset, u8 data);

	DECLARE_MACHINE_START(pc9821_canbe);

	struct {
		u8 index = 0;
	}m_remote;
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
	void pc9821xs(machine_config &config);
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

class pc9821_valuestar_state : public pc9821_mate_x_state
{
public:
	pc9821_valuestar_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_mate_x_state(mconfig, type, tag)
	{
	}

	void pc9821v13(machine_config &config);
	void pc9821v20(machine_config &config);
};

// 9821NOTE

class pc9821_note_state : public pc9821_state
{
public:
	pc9821_note_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
	{
	}

	void pc9821ne(machine_config &config);
};

class pc9821_note_lavie_state : public pc9821_note_state
{
public:
	pc9821_note_lavie_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_note_state(mconfig, type, tag)
	{
	}

	void pc9821nr15(machine_config &config);
	void pc9821nr166(machine_config &config);
	void pc9821nw150(machine_config &config);
};


#endif // MAME_NEC_PC9821_H
