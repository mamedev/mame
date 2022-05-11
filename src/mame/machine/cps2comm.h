// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_MACHINE_CPS2COMM_H
#define MAME_MACHINE_CPS2COMM_H

#pragma once

#include <memory>


class cps2_comm_device : public device_t
{
public:
	cps2_comm_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	u16 usart_data_r(offs_t offset, u16 mem_mask);
	u16 usart_status_r(offs_t offset, u16 mem_mask);
	void usart_data_w(offs_t offset, u16 data, u16 mem_mask);
	void usart_control_w(offs_t offset, u16 data, u16 mem_mask);
	void route_w(offs_t offset, u16 data, u16 mem_mask);

	bool comm_enabled() const { return bool(m_context); }

protected:
	enum
	{
		CTRL_MODE,
		CTRL_SYNC1,
		CTRL_SYNC2,
		CTRL_COMMAND
	};

	class context;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void start_comm() ATTR_COLD;

	std::unique_ptr<context> m_context;
	required_ioport m_config;

	bool m_is_head;

	u8 m_usart_control_phase;
	u8 m_usart_mode;
	u8 m_usart_sync_char[2];
	u8 m_usart_status;
	u8 m_usart_rx_data;

	u8 m_route;
};

DECLARE_DEVICE_TYPE(CAPCOM_CPS2_COMM, cps2_comm_device)

#endif // MAME_MACHINE_CPS2COMM_H
