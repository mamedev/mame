// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega SP (Spider)

 Naomi derived platform

*/

#include "naomi.h"

class segasp_state : public naomi_state
{
public:
	segasp_state(const machine_config &mconfig, device_type type, std::string tag)
		: naomi_state(mconfig, type, tag),
		m_sp_eeprom(*this, "sp_eeprom")
	{   }
	required_device<eeprom_serial_93cxx_device> m_sp_eeprom;

	DECLARE_READ64_MEMBER(sp_eeprom_r);
	DECLARE_WRITE64_MEMBER(sp_eeprom_w);
	DECLARE_READ64_MEMBER(sp_rombdflg_r);
	DECLARE_READ64_MEMBER(sp_io_r);
	DECLARE_READ64_MEMBER(sn_93c46a_r);
	DECLARE_WRITE64_MEMBER(sn_93c46a_w);
	DECLARE_READ64_MEMBER(sp_bank_r);
	DECLARE_WRITE64_MEMBER(sp_bank_w);
	UINT16 m_sp_bank;

protected:
};
