// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/******************************************
 *
 * NEC "Handy98" 1st gen portables
 *
 ******************************************/

#ifndef MAME_NEC_PC98HA_H
#define MAME_NEC_PC98HA_H

#pragma once

#include "pc9801.h"

class pc98lt_state : public pc98_base_state
{
public:
	pc98lt_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc98_base_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "upd765")
		, m_gvram(*this, "gvram")
		, m_bram_bank(*this, "bram_bank")
		, m_dict_bank(*this, "dict_bank")
		, m_kanji_bank(*this, "kanji_bank")
		, m_romdrv_bank(*this, "romdrv_bank")
	{
	}

	void lt_config(machine_config &config);

protected:
	void lt_map(address_map &map) ATTR_COLD;
	void lt_io(address_map &map) ATTR_COLD;

	required_device<v50_device> m_maincpu;

	virtual void machine_start() override ATTR_COLD;
//  virtual void machine_reset() override ATTR_COLD;
private:
	required_device<upd765a_device> m_fdc;
	required_shared_ptr<uint16_t> m_gvram;
	std::unique_ptr<uint16_t[]> m_bram_ptr;
	required_memory_bank m_bram_bank;
	required_memory_bank m_dict_bank;
	required_memory_bank m_kanji_bank;
	required_memory_bank m_romdrv_bank;

	void lt_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 power_status_r();
	void power_control_w(offs_t offset, u8 data);
	u8 floppy_mode_r(offs_t offset);
	void floppy_mode_w(offs_t offset, u8 data);
	u8 fdc_ctrl_r(offs_t offset);
	void fdc_ctrl_w(offs_t offset, u8 data);

	u8 m_romdrv_bank_reg = 0;
	u8 m_bram_banks = 0;
	u8 m_bram_bank_reg = 0;
	u8 m_dict_bank_reg = 0;

	u8 m_floppy_mode = 0;
	u8 m_fdc_ctrl = 0;
};

class pc98ha_state : public pc98lt_state
{
public:
	pc98ha_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc98lt_state(mconfig, type, tag)
		, m_ems_banks(*this, "ems_bank%u", 1U)
		, m_ext_view(*this, "ext_io")
		, m_ramdrv_bank(*this, "ramdrv_bank")
		, m_rtc_pio(*this, "prtc")
	{
	}

	void ha_config(machine_config &config);

protected:
	void ha_map(address_map &map) ATTR_COLD;
	void ha_io(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
private:
	required_memory_bank_array<4> m_ems_banks;
	memory_view m_ext_view;
	required_memory_bank m_ramdrv_bank;
	required_device<upd4991a_device> m_rtc_pio;

	std::unique_ptr<uint16_t[]> m_ems_ram;

	void ext_view_bank_w(offs_t offset, u8 data);
	void ext_view_sel_w(offs_t offset, u8 data);
	void ems_bank_w(offs_t offset, u8 data);
	u8 memcard_status_1_r(offs_t offset);
	u8 memcard_status_2_r(offs_t offset);
	u8 m_ext_view_sel = 0;
};

#endif // MAME_NEC_PC98HA_H
