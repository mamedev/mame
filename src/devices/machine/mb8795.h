// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_MB8795_H
#define MAME_MACHINE_MB8795_H

#include "dinetwork.h"

class mb8795_device :   public device_t,
						public device_network_interface
{
public:
	mb8795_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto tx_irq() { return m_irq_tx_cb.bind(); }
	auto rx_irq() { return m_irq_rx_cb.bind(); }
	auto tx_drq() { return m_drq_tx_cb.bind(); }
	auto rx_drq() { return m_drq_rx_cb.bind(); }

	void tx_dma_w(u8 data, bool eof);
	void rx_dma_r(u8 &data, bool &eof);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual int recv_start_cb(u8 *buf, int len) override;
	virtual void recv_complete_cb(int result) override;

	TIMER_CALLBACK_MEMBER(tx_update);
	TIMER_CALLBACK_MEMBER(rx_update);

private:
	u8 m_mac[6];
	u8 m_txbuf[2000], m_rxbuf[2000];
	u8 m_txstat, m_txmask, m_rxstat, m_rxmask, m_txmode, m_rxmode;
	u16 m_txlen, m_rxlen, m_txcount;
	bool m_drq_tx, m_drq_rx, m_irq_tx, m_irq_rx;
	emu_timer *m_timer_tx, *m_timer_rx;

	devcb_write_line m_irq_tx_cb, m_irq_rx_cb, m_drq_tx_cb, m_drq_rx_cb;

	void check_irq();
	void start_send();
	void receive();
	bool recv_is_broadcast();
	bool recv_is_me();
	bool recv_is_multicast();
	bool recv_is_local_multicast();

	u8 txstat_r();
	void txstat_w(u8 data);
	u8 txmask_r();
	void txmask_w(u8 data);
	u8 rxstat_r();
	void rxstat_w(u8 data);
	u8 rxmask_r();
	void rxmask_w(u8 data);
	u8 txmode_r();
	void txmode_w(u8 data);
	u8 rxmode_r();
	void rxmode_w(u8 data);
	void reset_w(u8 data);
	u8 tdr1_r();
	u8 mac_r(offs_t offset);
	void mac_w(offs_t offset, u8 data);
	u8 tdr2_r();
};

DECLARE_DEVICE_TYPE(MB8795, mb8795_device)

#endif // MAME_MACHINE_MB8795_H
