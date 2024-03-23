// license:BSD-3-Clause
// copyright-holders:Carl, Miodrag Milanovic

#include "emu.h"
#include "dinetwork.h"

#include "osdnet.h"

#include <algorithm>


device_network_interface::device_network_interface(const machine_config &mconfig, device_t &device, u32 bandwidth, u32 mtu)
	: device_interface(device, "network")
	, m_poll_timer(nullptr)
	, m_send_timer(nullptr)
	, m_recv_timer(nullptr)
{
	// Convert to Mibps to Bps
	m_bandwidth = bandwidth << (20 - 3);
	m_mtu = mtu;
	m_intf = -1;
	m_loopback_control = false;
}

device_network_interface::~device_network_interface()
{
}

void device_network_interface::interface_post_start()
{
	m_poll_timer = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_network_interface::poll_device), this));
	m_send_timer = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_network_interface::send_complete), this));
	m_recv_timer = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_network_interface::recv_complete), this));

	device().save_item(NAME(m_loopback_control));
}

void device_network_interface::interface_post_load()
{
	if (!m_dev)
		m_poll_timer->reset();
	else if (!m_loopback_control && !m_recv_timer->enabled())
		start_net_device();
	else
		stop_net_device();
}

int device_network_interface::send(u8 *buf, int len, int fcs)
{
	// TODO: enable this check when other devices implement delayed transmit
	//if (m_send_timer->enabled())
		//throw emu_fatalerror("%s(%s): attempted to transmit while transmit already in progress", device().shortname(), device().tag());

	int result = 0;

	if (m_loopback_control)
	{
		// loop data back to receiver
		result = recv_start_cb(buf, len);

		if (result)
		{
			// schedule receive complete callback
			m_recv_timer->adjust(attotime::from_ticks(len, m_bandwidth), result);
		}
	}
	else if (m_dev)
	{
		// send the data (excluding fcs)
		result = m_dev->send(buf, len - fcs);
		if (result)
			result += fcs;
	}

	// schedule transmit complete callback
	m_send_timer->adjust(attotime::from_ticks(len, m_bandwidth), result);

	return result;
}

TIMER_CALLBACK_MEMBER(device_network_interface::poll_device)
{
	m_dev->poll();
}

void device_network_interface::start_net_device()
{
	// Set device polling time to transfer time for one MTU
	m_dev->start();
	const attotime interval = attotime::from_hz(m_bandwidth / m_mtu);
	m_poll_timer->adjust(attotime::zero, 0, interval);
}

void device_network_interface::stop_net_device()
{
	m_poll_timer->reset();
	m_dev->stop();
}

TIMER_CALLBACK_MEMBER(device_network_interface::send_complete)
{
	send_complete_cb(param);
}

void device_network_interface::recv_cb(u8 *buf, int len)
{
	if (m_recv_timer->enabled())
		throw emu_fatalerror("%s(%s): attempted to receive while receive already in progress", device().shortname(), device().tag());

	int result = 0;

	// process the received data
	if (!m_loopback_control)
		result = recv_start_cb(buf, len);

	if (result)
	{
		// stop receiving more data from the network
		if (m_dev)
			stop_net_device();

		// schedule receive complete callback
		m_recv_timer->adjust(attotime::from_ticks(len, m_bandwidth), result);
	}
}

TIMER_CALLBACK_MEMBER(device_network_interface::recv_complete)
{
	recv_complete_cb(param);

	// start receiving data from the network again
	if (m_dev && !m_loopback_control)
		start_net_device();
}

void device_network_interface::set_promisc(bool promisc)
{
	m_promisc = promisc;
	if (m_dev)
		m_dev->set_promisc(promisc);
}

void device_network_interface::set_mac(const u8 *mac)
{
	std::copy_n(mac, std::size(m_mac), std::begin(m_mac));
	if (m_dev)
		m_dev->set_mac(&m_mac[0]);
}

void device_network_interface::set_interface(int id)
{
	if (m_dev)
		stop_net_device();

	m_dev.reset(open_netdev(id, *this));
	if (m_dev)
	{
		if (!m_loopback_control)
			start_net_device();
	}
	else
	{
		device().logerror("Network interface %d not found\n", id);
		id = -1;
	}
	m_intf = id;
}

void device_network_interface::set_loopback(bool loopback)
{
	if (m_loopback_control == loopback)
		return;

	m_loopback_control = loopback;

	if (m_dev)
	{
		if (loopback)
			stop_net_device();
		else if (!m_recv_timer->enabled())
			start_net_device();
	}
}
