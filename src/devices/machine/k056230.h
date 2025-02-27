// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 056230 LAN controller skeleton device

***************************************************************************/

#ifndef MAME_MACHINE_K056230_H
#define MAME_MACHINE_K056230_H

#pragma once

#include "osdfile.h"

class k056230_device : public device_t
{
public:
	// construction/destruction
	k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto irq_cb() { return m_irq_cb.bind(); }

	u32 ram_r(offs_t offset, u32 mem_mask = ~0);
	void ram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void regs_map(address_map &map) ATTR_COLD;

protected:
	k056230_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_share_creator<u32> m_ram;

	devcb_write_line m_irq_cb;
	int m_irq_state = 0;
	u8 m_ctrl_reg = 0;
	u8 m_status = 0;

private:
	osd_file::ptr m_line_rx; // "fake" RX line, real hardware is half-duplex
	osd_file::ptr m_line_tx; // "fake" TX line, real hardware is half-duplex
	char m_localhost[256]{};
	char m_remotehost[256]{};
	u8 m_buffer0[0x201]{};
	u8 m_buffer1[0x201]{};
	u8 m_linkenable = 0;
	u8 m_linkid = 0;
	u8 m_txmode = 0;

	void set_mode(u8 data);
	void set_ctrl(u8 data);
	void comm_tick();
	int read_frame(int dataSize);
	void send_frame(int dataSize);
};

class k056230_viper_device : public k056230_device
{
public:
	// construction/destruction
	k056230_viper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void regs_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_control = 0;
	bool m_irq_enable = false;
	u8 m_unk[2]{};
};

// device type definition
DECLARE_DEVICE_TYPE(K056230, k056230_device)
DECLARE_DEVICE_TYPE(K056230_VIPER, k056230_viper_device)

#endif // MAME_MACHINE_K056230_H
