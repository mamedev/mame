// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "sms.h"

class smsbootleg_state : public sms_state
{
	public:
		smsbootleg_state(const machine_config &mconfig, device_type type, const char *tag)
			: sms_state(mconfig, type, tag),
			m_mainrom(*this, "maincpu"),
			m_unpaged(*this, "unpaged"),
			m_page0(*this, "page0"),
			m_page1(*this, "page1"),
			m_page2(*this, "page2")
		{}


	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port18_w);
	DECLARE_WRITE8_MEMBER(bootleg_mapper_w);
	DECLARE_READ8_MEMBER(bootleg_mapper_r);

	DECLARE_DRIVER_INIT(sms_supergame);
	DECLARE_DRIVER_INIT(sms_supergamea);

private:

	void bootleg_set_banks();
	void bootleg_init_common();

	int m_bankbase;
	int m_bankmappers[0x4];

	required_region_ptr<uint8_t> m_mainrom;
	required_memory_bank m_unpaged;
	required_memory_bank m_page0;
	required_memory_bank m_page1;
	required_memory_bank m_page2;
};
