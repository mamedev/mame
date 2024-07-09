// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 056230 LAN controller skeleton device

***************************************************************************/

#ifndef MAME_MACHINE_K056230_H
#define MAME_MACHINE_K056230_H

#pragma once

class k056230_device : public device_t
{
public:
	// construction/destruction
	k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto irq_cb() { return m_irq_cb.bind(); }

	u32 ram_r(offs_t offset, u32 mem_mask = ~0);
	void ram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void regs_map(address_map &map);

protected:
	k056230_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	memory_share_creator<u32> m_ram;

	devcb_write_line m_irq_cb;
	int m_irq_state = 0;
	u8 m_ctrl_reg = 0;
	u8 m_status = 0;
};

class k056230_viper_device : public k056230_device
{
public:
	// construction/destruction
	k056230_viper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void regs_map(address_map &map) override;

protected:
	virtual void device_reset() override;

private:
	u8 m_control = 0;
	bool m_irq_enable = false;
	u8 m_unk[2]{};
};

// device type definition
DECLARE_DEVICE_TYPE(K056230, k056230_device)
DECLARE_DEVICE_TYPE(K056230_VIPER, k056230_viper_device)

#endif // MAME_MACHINE_K056230_H
