// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
/******************************************
 *
 * Epson PC98 clones
 *
 ******************************************/

#ifndef MAME_NEC_PC9801_EPSON_H
#define MAME_NEC_PC9801_EPSON_H

#pragma once

#include "pc9801.h"

class pc98_epson_state : public pc9801vm_state
{
public:
	pc98_epson_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9801vm_state(mconfig, type, tag)
		, m_shadow_ipl(*this, "shadow_ipl_%u", 0)
	{
	}

	void pc286vs(machine_config &config);
	void pc386m(machine_config &config);
	void pc486mu(machine_config &config);
	void pc486se(machine_config &config);

protected:
	void config_base_epson(machine_config &config);

	void pc386m_ipl_bank(address_map &map) ATTR_COLD;

	void epson_base_io(address_map &map) ATTR_COLD;

	void pc286vs_io(address_map &map) ATTR_COLD;
	void pc286vs_map(address_map &map) ATTR_COLD;
	void pc386m_io(address_map &map) ATTR_COLD;
	void pc386m_map(address_map &map) ATTR_COLD;
	void pc486se_io(address_map &map) ATTR_COLD;
	void pc486se_map(address_map &map) ATTR_COLD;

//  virtual void machine_start() override ATTR_COLD;
//  virtual void machine_reset() override ATTR_COLD;

	DECLARE_MACHINE_START(pc98_epson);
	DECLARE_MACHINE_RESET(pc98_epson);

private:
	void epson_ipl_bank_w(offs_t offset, u8 data);
	void epson_itf_bank_w(offs_t offset, u8 data);
	void epson_a20_w(offs_t offset, u8 data);
	void epson_vram_bank_w(offs_t offset, u8 data);

	required_shared_ptr_array<uint16_t, 2> m_shadow_ipl;

	template <unsigned which> void shadow_ipl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	u8 m_shadow_ipl_bank = 0;
	bool m_itf_bank_enable = false;
//  u8 m_itf_bank;
};

#endif // MAME_NEC_PC9801_EPSON_H
