// license:BSD-3-Clause
// copyright-holders:Mark McDougall, R. Belmont
/***************************************************************************

  namcond1.h

  Common functions & declarations for the Namco ND-1 driver

***************************************************************************/

#include "video/ygv608.h"

class namcond1_state : public driver_device
{
public:
	namcond1_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_ygv608(*this, "ygv608"),
		m_shared_ram(*this, "shared_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<ygv608_device> m_ygv608;

	required_shared_ptr<UINT16> m_shared_ram;

	UINT8 m_h8_irq5_enabled;
	int m_p8;

	DECLARE_READ16_MEMBER(mcu_p7_read);
	DECLARE_READ16_MEMBER(mcu_pa_read);
	DECLARE_WRITE16_MEMBER(mcu_pa_write);
	DECLARE_READ16_MEMBER(cuskey_r);
	DECLARE_WRITE16_MEMBER(cuskey_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	INTERRUPT_GEN_MEMBER(mcu_interrupt);
};
