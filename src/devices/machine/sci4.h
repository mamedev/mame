// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SCI4 / XV833A00, 7-lines serial chip with 4 multiplexed on one and the other 3 separated

#ifndef DEVICES_MACHINE_SCI4_H
#define DEVICES_MACHINE_SCI4_H

#pragma once

class sci4_device : public device_t
{
public:
	sci4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 8000000);

	// sci port numbers are 0..2 and 30..33
	template<int Sci> void rx_w(int state) { do_rx_w(Sci, state); }
	template<int Sci> auto write_tx() { if(Sci < 3) return m_tx[Sci].bind(); else return m_tx[Sci - 30 + 3].bind(); }

	// irq line numbers are 0..3
	template<int irq> auto write_irq() { return m_irq[irq].bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

protected:
	devcb_write_line::array<7> m_tx;
	devcb_write_line::array<4> m_irq;

	emu_timer *m_tx_timer[4];
	emu_timer *m_rx_timer[4];

	std::array<u8, 7> m_rx;
	std::array<u8, 4> m_enable, m_status, m_datamode, m_div, m_cur_rx;
	std::array<u8, 4> m_tdr, m_tsr, m_tdr_full, m_tx_step, m_tx_active;
	std::array<u8, 4> m_rdr, m_rsr, m_rdr_full, m_rx_step, m_rx_active;
	u8 m_targets;

	void do_rx_w(int sci, int state);

	void default_w(offs_t offset, u8 data);
	u8 default_r(offs_t offset);

	void datamode_w(offs_t slot, u8 data);
	u8 datamode_r(offs_t slot);
	void data_w(offs_t slot, u8 data);
	u8 data_r(offs_t slot);
	void enable_w(offs_t slot, u8 data);
	u8 enable_r(offs_t slot);
	u8 status_r(offs_t slot);
	u8 reset_r(offs_t slot);

	void target_w(u8 data);

	std::string chan_id(u8 chan, u8 target);

	void wait(int timer, int full, int chan);
	void tx_enabled(int chan);
	void tx_set(int slot, int state);
	void tx_start(int chan);
	void rx_changed(int chan);
	void fifo_w(int chan, u8 data);
	u8 fifo_r(int chan);

	TIMER_CALLBACK_MEMBER(tx_tick);
	TIMER_CALLBACK_MEMBER(rx_tick);
};

DECLARE_DEVICE_TYPE(SCI4, sci4_device)

#endif
