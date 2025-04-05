// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 056230 LAN controller skeleton device

***************************************************************************/

#ifndef MAME_MACHINE_K056230_H
#define MAME_MACHINE_K056230_H

#pragma once

#include "asio.h"

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
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_share_creator<u32> m_ram;

	devcb_write_line m_irq_cb;
	int m_irq_state;
	u8 m_ctrl_reg;
	u8 m_status;

private:
	asio::io_context m_ioctx;
	std::optional<asio::ip::tcp::endpoint> m_localaddr;
	std::optional<asio::ip::tcp::endpoint> m_remoteaddr;
	asio::ip::tcp::acceptor m_acceptor;
	asio::ip::tcp::socket m_sock_rx;
	asio::ip::tcp::socket m_sock_tx;
	asio::steady_timer m_tx_timeout;
	u8 m_rx_state;
	u8 m_tx_state;
	u8 m_buffer0[0x201];
	u8 m_linkenable;
	u8 m_linkid;
	u8 m_txmode;

	void set_mode(u8 data);
	void set_ctrl(u8 data);
	void comm_tick();
	void check_sockets();
	void comm_start();
	void comm_stop();
	unsigned read_frame(unsigned data_size);
	void send_frame(unsigned data_size);
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
	u8 m_control;
	bool m_irq_enable;
	u8 m_unk[2];
};

// device type definition
DECLARE_DEVICE_TYPE(K056230, k056230_device)
DECLARE_DEVICE_TYPE(K056230_VIPER, k056230_viper_device)

#endif // MAME_MACHINE_K056230_H
