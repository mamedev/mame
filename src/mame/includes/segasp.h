// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega SP (Spider)

 Naomi derived platform

*/

#ifndef MAME_INCLUDES_SEGASP_H
#define MAME_INCLUDES_SEGASP_H

#pragma once

#include "naomi.h"

class segasp_state : public naomi_state
{
public:
	segasp_state(const machine_config &mconfig, device_type type, const char *tag)
		: naomi_state(mconfig, type, tag),
		m_sp_eeprom(*this, "sp_eeprom")
	{   }

	void segasp(machine_config &config);

	void init_segasp();

private:
	required_device<eeprom_serial_93cxx_device> m_sp_eeprom;

	uint64_t sp_eeprom_r(offs_t offset, uint64_t mem_mask = ~0);
	void sp_eeprom_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t sp_rombdflg_r();
	uint64_t sp_io_r(offs_t offset, uint64_t mem_mask = ~0);
	uint64_t sn_93c46a_r();
	void sn_93c46a_w(uint64_t data);
	uint64_t sp_bank_r(offs_t offset, uint64_t mem_mask = ~0);
	void sp_bank_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint16_t m_sp_bank;

	void onchip_port(address_map &map);
	void segasp_map(address_map &map);
};

#endif // MAME_INCLUDES_SEGASP_H
