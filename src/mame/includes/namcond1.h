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
	namcond1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_ygv608(*this, "ygv608"),
		m_shared_ram(*this, "shared_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<ygv608_device> m_ygv608;

	required_shared_ptr<uint16_t> m_shared_ram;

	uint8_t m_h8_irq5_enabled;
	int m_p8;

	uint16_t mcu_p7_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mcu_pa_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mcu_pa_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t cuskey_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cuskey_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void mcu_interrupt(device_t &device);
};
