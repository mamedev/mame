// license:BSD-3-Clause
// copyright-holders:Carl, Miodrag Milanovic
#ifndef MAME_EMU_DINETWORK_H
#define MAME_EMU_DINETWORK_H

#include "interface/nethandler.h"


class device_network_interface : public device_interface, public osd::network_handler
{
public:
	device_network_interface(const machine_config &mconfig, device_t &device, u32 bandwidth, u32 mtu = 1500);
	virtual ~device_network_interface();

	void interface_post_start() override ATTR_COLD;
	void interface_post_load() override ATTR_COLD;

	void set_interface(int id) ATTR_COLD;
	void set_mac(const u8 *mac);
	void set_loopback(bool loopback);

	int get_interface() const { return m_intf; }

	int send(u8 *buf, int len, int fcs = 0);

	// TODO: de-virtualise this when existing devices implement delayed receive
	virtual void recv_cb(u8 *buf, int len) override;

	// delayed transmit/receive handlers
	virtual void send_complete_cb(int result) {}
	virtual int recv_start_cb(u8 *buf, int len) { return 0; }
	virtual void recv_complete_cb(int result) {}

protected:
	bool has_net_device() const noexcept { return bool(m_dev); }
	void log_bytes(u8 *buf, int len);

	// bandwidth in bytes per second
	u32 m_bandwidth;
	// maximum transmission unit, used for device polling time
	u32 m_mtu;
	int m_intf;
	bool m_loopback_control;

private:
	TIMER_CALLBACK_MEMBER(poll_device);
	TIMER_CALLBACK_MEMBER(send_complete);
	TIMER_CALLBACK_MEMBER(recv_complete);

	void start_net_device();
	void stop_net_device();

	std::unique_ptr<osd::network_device> m_dev;
	emu_timer *m_poll_timer;
	emu_timer *m_send_timer;
	emu_timer *m_recv_timer;
};


// iterator
typedef device_interface_enumerator<device_network_interface> network_interface_enumerator;

#endif // MAME_EMU_DINETWORK_H
