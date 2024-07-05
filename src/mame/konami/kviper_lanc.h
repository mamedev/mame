// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

Konami Viper LAN Controller

***************************************************************************/

#ifndef MAME_MACHINE_KVIPER_LANC_H
#define MAME_MACHINE_KVIPER_LANC_H

#pragma once

class kviper_lanc_device : public device_t
{
public:
	// construction/destruction
	kviper_lanc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto irq_cb() { return m_irq_cb.bind(); }

	void map(address_map &map);

	void network_id_w(uint8_t data);
	void control_w(uint8_t data);
	uint8_t status_r();
	uint8_t unk1_r();
	void unk1_w(uint8_t data);
	uint8_t unk2_r();
	void unk2_w(uint8_t data);
	void start_w(uint8_t data);
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line m_irq_cb;
	int m_irq_state;

	bool m_irq_enable = false;

private:
	uint8_t m_ram[0x2000]{};
	uint8_t m_network_id;
	uint8_t m_control;
	uint8_t m_status;
	uint8_t m_unk1;
	uint8_t m_unk2;
};

// device type definition
DECLARE_DEVICE_TYPE(KVIPER_LANC, kviper_lanc_device)

#endif // MAME_MACHINE_KVIPER_LANC_H