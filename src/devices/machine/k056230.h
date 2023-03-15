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

	u8 regs_r(offs_t offset);
	void regs_w(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;

	memory_share_creator<u32> m_ram;

	devcb_write_line m_irq_cb;
	int m_irq_state;
	u8 m_ctrl_reg;
};


// device type definition
DECLARE_DEVICE_TYPE(K056230, k056230_device)

#endif // MAME_MACHINE_K056230_H
