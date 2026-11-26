// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_MB8795_H
#define MAME_MACHINE_MB8795_H

#include "dinetwork.h"

class mb8795_device :   public device_t,
						public device_network_interface
{
public:
	mb8795_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto tx_irq() { return m_irq_tx_cb.bind(); }
	auto rx_irq() { return m_irq_rx_cb.bind(); }
	auto tx_drq() { return m_drq_tx_cb.bind(); }
	auto rx_drq() { return m_drq_rx_cb.bind(); }

	void tx_dma_w(u8 data, bool eof);
	void rx_dma_r(u8 &data, bool &eof);

	virtual void map(address_map &map) ATTR_COLD;

protected:
	mb8795_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 txstat_ack, u8 txmask_bits, u8 rxstat_ack, u8 rxmask_bits);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual int recv_start_cb(u8 *buf, int len) override;
	virtual void recv_complete_cb(int result) override;

	virtual TIMER_CALLBACK_MEMBER(tx_update);
	TIMER_CALLBACK_MEMBER(rx_update);

	virtual bool recv_accept();
	virtual bool tx_echo() const;
	virtual void txmode_w(u8 data);
	virtual u8 tdr1_r();
	virtual u8 tdr2_r();

	void start_send();
	void stop_send();
	bool recv_is_broadcast();

	u8 m_mac[6];
	u8 m_rxbuf[2000];
	u8 m_txstat, m_txmode, m_rxmode;
	u8 m_rx_nibble;

private:
	const u8 m_txstat_ack, m_txmask_bits, m_rxstat_ack, m_rxmask_bits;
	u8 m_txbuf[2000];
	u8 m_txmask, m_rxstat, m_rxmask;
	u16 m_txlen, m_rxlen, m_txcount;
	bool m_drq_tx, m_drq_rx, m_irq_tx, m_irq_rx;
	emu_timer *m_timer_tx, *m_timer_rx;

	devcb_write_line m_irq_tx_cb, m_irq_rx_cb, m_drq_tx_cb, m_drq_rx_cb;

	void check_irq();
	void receive();
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
	u8 rxmode_r();
	void rxmode_w(u8 data);
	void reset_w(u8 data);
	u8 mac_r(offs_t offset);
	void mac_w(offs_t offset, u8 data);
};

// The MB8795 register layout with different mode bits and explicit
// transmitter and receiver enables.
class att7213_device : public mb8795_device
{
public:
	att7213_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual TIMER_CALLBACK_MEMBER(tx_update) override;

	virtual bool recv_accept() override;
	virtual bool tx_echo() const override;
	virtual void txmode_w(u8 data) override;
	virtual u8 tdr1_r() override;
	virtual u8 tdr2_r() override;

private:
	u8 m_control;

	u8 control_r();
	void control_w(u8 data);
};

DECLARE_DEVICE_TYPE(MB8795, mb8795_device)
DECLARE_DEVICE_TYPE(ATT7213, att7213_device)

#endif // MAME_MACHINE_MB8795_H
