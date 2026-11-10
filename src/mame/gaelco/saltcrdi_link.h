// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Tomás García-Merás Capote
#ifndef MAME_GAELCO_SALTCRDI_LINK_H
#define MAME_GAELCO_SALTCRDI_LINK_H

#pragma once

#include <memory>


class saltcrdi_link_device : public device_t
{
public:
	saltcrdi_link_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	~saltcrdi_link_device();

	void control_w(int state);
	void data_w(int state);

	int control_r();
	int data_r();

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	enum
	{
		STATE_OFFLINE,
		STATE_RESET,
		STATE_IDLE,
		STATE_RX_READY,
		STATE_RX_ACK,
		STATE_RX_END,
		STATE_TX_REQUEST,
		STATE_TX_DATA,
		STATE_TX_NEXT,
		STATE_TX_END
	};

	class context;

	TIMER_CALLBACK_MEMBER(settle);
	TIMER_CALLBACK_MEMBER(poll);

	void update();
	void enter_state(u8 state, u8 lines);
	void abort_transfer();
	void start_link() ATTR_COLD;

	std::unique_ptr<context> m_context;
	required_ioport m_config;
	emu_timer *m_settle_timer;
	emu_timer *m_poll_timer;

	u8 m_state;
	u8 m_board_lines;
	u8 m_peer_lines;
	u8 m_bit;
	u8 m_shift;
	u8 m_tx_data;
	bool m_tx_pending;
	u16 m_stall;
};

DECLARE_DEVICE_TYPE(SALTCRDI_LINK, saltcrdi_link_device)

#endif // MAME_GAELCO_SALTCRDI_LINK_H
