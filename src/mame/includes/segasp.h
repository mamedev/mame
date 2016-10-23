// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega SP (Spider)

 Naomi derived platform

*/

#include "naomi.h"

class segasp_state : public naomi_state
{
public:
	segasp_state(const machine_config &mconfig, device_type type, const char *tag)
		: naomi_state(mconfig, type, tag),
		m_sp_eeprom(*this, "sp_eeprom")
	{   }
	required_device<eeprom_serial_93cxx_device> m_sp_eeprom;

	uint64_t sp_eeprom_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void sp_eeprom_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t sp_rombdflg_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t sp_io_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t sn_93c46a_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void sn_93c46a_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t sp_bank_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void sp_bank_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint16_t m_sp_bank;

protected:
};
